#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "synce-device-manager.h"
#if USE_GDBUS
#include "synce-device-manager-dbus.h"
#else
#include <dbus/dbus-glib.h>
#include "synce-device-manager-glue.h"
#endif
#include "synce-device-manager-control.h"
#include "synce-device.h"
#include "synce-device-rndis.h"
#include "synce-device-legacy.h"
#include "utils.h"

static void     synce_device_manager_initable_iface_init (GInitableIface  *iface);
static gboolean synce_device_manager_initable_init       (GInitable       *initable,
							  GCancellable    *cancellable,
							  GError          **error);

G_DEFINE_TYPE_WITH_CODE (SynceDeviceManager, synce_device_manager, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, synce_device_manager_initable_iface_init))

/* private stuff */

typedef struct
{
  gchar *device_path;
  gchar *device_ip;
  gchar *local_ip;
  gboolean rndis;
  gboolean iface_pending;
  GSocketService *server;
  SynceDevice *device;
} DeviceEntry;

typedef struct _SynceDeviceManagerPrivate SynceDeviceManagerPrivate;

struct _SynceDeviceManagerPrivate
{
  gboolean inited;
  gboolean dispose_has_run;

  GSList *devices;
  SynceDeviceManagerControl *control_iface;
  gulong control_connect_id;
  gulong control_disconnect_id;
  guint iface_check_id;
#if USE_GDBUS
  SynceDbusDeviceManager *interface;
#endif
};

#define SYNCE_DEVICE_MANAGER_GET_PRIVATE(o) \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), SYNCE_TYPE_DEVICE_MANAGER, SynceDeviceManagerPrivate))


static void
synce_device_manager_device_entry_free(DeviceEntry *entry)
{
  g_free(entry->device_path);
  g_free(entry->device_ip);
  g_free(entry->local_ip);
  if (entry->server) {
    if (g_socket_service_is_active(entry->server)) {
      g_socket_service_stop(entry->server);
    }
    g_object_unref(entry->server);
  }
  if (entry->device) g_object_unref(entry->device);
  g_free(entry);

  return;
}


static void
synce_device_manager_device_sends_disconnected_cb(SynceDevice *device,
						  gpointer user_data)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(user_data);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);

  g_debug("%s: received disconnect signal from device object", G_STRFUNC);

  GSList *device_entry_iter = priv->devices;
  while (device_entry_iter) {
    if (((DeviceEntry*)device_entry_iter->data)->device == device)
      break;
    device_entry_iter = g_slist_next(device_entry_iter);
  }

  if (!device_entry_iter) {
    g_critical("%s: disconnect signal received from a non-recognised device", G_STRFUNC);
    return;
  }

  DeviceEntry *deventry = device_entry_iter->data;
  priv->devices = g_slist_delete_link(priv->devices, device_entry_iter);

  gchar *obj_path = NULL;
  g_object_get(device, "object-path", &obj_path, NULL);
  g_debug("%s: emitting disconnect for object path %s", G_STRFUNC, obj_path);
  g_signal_emit (self, SYNCE_DEVICE_MANAGER_GET_CLASS(SYNCE_DEVICE_MANAGER(self))->signals[SYNCE_DEVICE_MANAGER_DEVICE_DISCONNECTED], 0, obj_path);
#if USE_GDBUS
  synce_dbus_device_manager_emit_device_disconnected(priv->interface, obj_path);
#endif
  g_free(obj_path);
  synce_device_manager_device_entry_free(deventry);

  return;
}


static void
synce_device_manager_device_obj_path_changed_cb(GObject    *obj,
						GParamSpec *param,
						gpointer    user_data)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(user_data);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);
  SynceDevice *dev = SYNCE_DEVICE(obj);

  gchar *obj_path = NULL;
  g_object_get (dev, "object-path", &obj_path, NULL);
  if (!obj_path) {
    g_debug("%s: device set object path to NULL", G_STRFUNC);
    return;
  }

  g_debug("%s: sending connected signal for %s", G_STRFUNC, obj_path); 
  g_signal_emit (self, SYNCE_DEVICE_MANAGER_GET_CLASS(SYNCE_DEVICE_MANAGER(self))->signals[SYNCE_DEVICE_MANAGER_DEVICE_CONNECTED], 0, obj_path);
#if USE_GDBUS
  synce_dbus_device_manager_emit_device_connected(priv->interface, obj_path);
#endif
  g_free (obj_path);
}


static gboolean
synce_device_manager_client_connected_cb(GSocketService *server,
					 GSocketConnection *conn,
					 GObject *source_object,
					 gpointer user_data)
{
  if (conn == NULL) {
    g_critical("%s: a connection error occured", G_STRFUNC);
    return TRUE;
  }

  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(user_data);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);

  GSList *device_entry_iter = priv->devices;
  while (device_entry_iter) {
    if ( (((DeviceEntry*)device_entry_iter->data)->server == server) )
      break;
    device_entry_iter = g_slist_next(device_entry_iter);
  }

  if (!device_entry_iter) {
    g_critical("%s: connection from a non-recognised server", G_STRFUNC);
    return TRUE;
  }

  DeviceEntry *deventry = device_entry_iter->data;
  GError *error = NULL;

  GSocketAddress *local_inet_addr = g_socket_connection_get_local_address(conn, &error);
  if (!local_inet_addr) {
    g_critical("%s: failed to get address from new connection: %s", G_STRFUNC, error->message);
    g_error_free(error);
    return TRUE;
  }
  guint local_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(local_inet_addr));
  g_object_unref(local_inet_addr);

  g_debug("%s: have a connection to port %d", G_STRFUNC, local_port);

  if (local_port == 5679) {
    if (!(deventry->device)) {
      /*
       * should close the socket listener on port 990 here, but can't see how to
       */
      g_debug("%s: creating device object for %s", G_STRFUNC, deventry->device_path);
      deventry->device = g_initable_new (SYNCE_TYPE_DEVICE_LEGACY, NULL, &error, "connection", conn, "device-path", deventry->device_path, NULL);
    } else {
      g_warning("%s: unexpected secondary connection to port %d", G_STRFUNC, local_port);
      return TRUE;
    }
  } else if (local_port == 990) {
    if (!(deventry->device)) {
      /*
       * should close the socket listener on port 5679 here, but can't see how to
       */
      g_debug("%s: creating device object for %s", G_STRFUNC, deventry->device_path);
      deventry->device = g_initable_new (SYNCE_TYPE_DEVICE_RNDIS, NULL, &error, "connection", conn, "device-path", deventry->device_path, NULL);
    } else {
      synce_device_rndis_client_connected (SYNCE_DEVICE_RNDIS(deventry->device), conn);
      return TRUE;
    }
  } else {
    g_warning("%s: unexpected connection to port %d", G_STRFUNC, local_port);
    return TRUE;
  }

  if (!deventry->device) {
    g_critical("%s: failed to create device object for new connection: %s", G_STRFUNC, error->message);
    g_error_free(error);
    return TRUE;
  }

  g_signal_connect(deventry->device, "disconnected", G_CALLBACK(synce_device_manager_device_sends_disconnected_cb), self);
  g_signal_connect(deventry->device, "notify::object-path", G_CALLBACK(synce_device_manager_device_obj_path_changed_cb), self);

  return TRUE;
}


static gboolean
synce_device_manager_create_device(SynceDeviceManager *self,
				   const gchar *local_ip,
				   DeviceEntry *deventry)
{
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE(self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  g_debug("%s: found device interface for %s", G_STRFUNC, deventry->device_path);

  deventry->iface_pending = FALSE;

  GError *error = NULL;
  GSocket *socket_990 = NULL;
  GSocket *socket_5679 = NULL;

  if (!(socket_990 = synce_create_socket(local_ip, 990))) {
    g_critical("%s: failed to create listening socket on rndis port (990)", G_STRFUNC);
    goto error_exit;
  }

  if (!(socket_5679 = synce_create_socket(local_ip, 5679))) {
    g_critical("%s: failed to create listening socket on legacy port (5679)", G_STRFUNC);
    goto error_exit;
  }

  deventry->server = g_socket_service_new();
  if (!(deventry->server)) {
    g_critical("%s: failed to create socket service", G_STRFUNC);
    goto error_exit;
  }

  if (!(g_socket_listener_add_socket(G_SOCKET_LISTENER(deventry->server), socket_990, NULL, &error))) {
    g_critical("%s: failed to add 990 socket to socket service: %s", G_STRFUNC, error->message);
    g_error_free(error);
    goto error_exit;
  }

  if (!(g_socket_listener_add_socket(G_SOCKET_LISTENER(deventry->server), socket_5679, NULL, &error))) {
    g_critical("%s: failed to add 5679 socket to socket service: %s", G_STRFUNC, error->message);
    g_error_free(error);
    goto error_exit;
  }

  g_object_unref(socket_990);
  g_object_unref(socket_5679);

  g_signal_connect(deventry->server, "incoming", G_CALLBACK(synce_device_manager_client_connected_cb), self);

  g_socket_service_start(deventry->server);

  g_debug("%s: listening for device %s", G_STRFUNC, deventry->device_path);

  if (deventry->rndis) {
  g_debug("%s: triggering connection", G_STRFUNC);
    synce_trigger_connection(deventry->device_ip);
  } else {
  g_debug("%s: NOT triggering connection", G_STRFUNC);
}

  return TRUE;

 error_exit:
  if (socket_990) g_object_unref(socket_990);
  if (socket_5679) g_object_unref(socket_5679);
  if (deventry->server) g_object_unref(deventry->server);

  return FALSE;
}

static gboolean
synce_device_manager_check_interface_cb (gpointer userdata)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(userdata);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE(self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  GError *error = NULL;
  GSList *device_entry_iter = NULL;

  device_entry_iter = priv->devices;
  while (device_entry_iter) {
    gchar *local_ip = ((DeviceEntry*)device_entry_iter->data)->local_ip;

    GSocket *socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &error);
    if (!socket) {
      g_critical("%s: failed to create a socket: %s", G_STRFUNC, error->message);
      g_error_free(error);
      return TRUE;
    }

    GInetAddress *local_addr = g_inet_address_new_from_string(local_ip);
    GInetSocketAddress *sockaddr = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new(local_addr, 990));

    if (g_socket_bind(socket, G_SOCKET_ADDRESS(sockaddr), TRUE, &error)) {
      g_debug("%s: address ready", G_STRFUNC);
      g_object_unref(socket);
      synce_device_manager_create_device(self, ((DeviceEntry*)device_entry_iter->data)->local_ip, device_entry_iter->data);
    } else {
      g_debug("%s: address not yet ready, failed to bind: %s", G_STRFUNC, error->message);
      g_error_free(error);
      error = NULL;
      g_object_unref(socket);
    }
    g_object_unref(sockaddr);
    g_object_unref(local_addr);

    device_entry_iter = g_slist_next(device_entry_iter);
  }

  /* check if any interfaces are still outstanding */

  device_entry_iter = priv->devices;
  while (device_entry_iter) {
    if (((DeviceEntry*)device_entry_iter->data)->iface_pending)
      return TRUE;

    device_entry_iter = g_slist_next(device_entry_iter);
  }

  priv->iface_check_id = 0;
  return FALSE;
}


static void
synce_device_manager_device_connected_cb(SynceDeviceManagerControl *device_manager_control,
					 gchar *device_path,
					 gchar *device_ip,
					 gchar *local_ip,
					 gboolean rndis,
					 gpointer userdata)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(userdata);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE(self);
  g_return_if_fail(priv->inited && !(priv->dispose_has_run));

  g_debug("%s: receieved device connected signal for %s", G_STRFUNC, device_path);

  DeviceEntry *deventry = g_new0(DeviceEntry, 1);
  if (!deventry) {
    g_critical("%s: failed to allocate DeviceEntry", G_STRFUNC);
    return;
  }

  deventry->device_path = g_strdup(device_path);
  deventry->device_ip = g_strdup(device_ip);
  deventry->local_ip = g_strdup(local_ip);
  deventry->rndis = rndis;
  deventry->iface_pending = TRUE;

  priv->devices = g_slist_append(priv->devices, deventry);

  /* only set up the timeout callback if it doesn't already exist */
  if (priv->iface_check_id > 0)
    return;

  priv->iface_check_id = g_timeout_add (100, synce_device_manager_check_interface_cb, self);
}


static void
synce_device_manager_device_disconnected_cb(SynceDeviceManagerControl *device_manager_control,
					    gchar *device_path,
					    gpointer userdata)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(userdata);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE(self);
  g_return_if_fail(priv->inited && !(priv->dispose_has_run));

  g_debug("%s: receieved device disconnected signal for %s", G_STRFUNC, device_path);

  /*
   * this signal comes through the udev scripts, if we have gudev we ignore this as we
   * are monitoring udev directly
   */

#if HAVE_GUDEV
  g_debug("%s: ignored, listening through libgudev", G_STRFUNC);
#else
  GSList *device_entry_iter = priv->devices;
  while (device_entry_iter != NULL) {

    if (strcmp(device_path, ((DeviceEntry*)device_entry_iter->data)->device_path) == 0) {
      break;
    }

    device_entry_iter = g_slist_next(device_entry_iter);
  }

  if (!device_entry_iter) {
    g_critical("%s: disconnect signal received for a non-recognised device", G_STRFUNC);
    return;
  }

  DeviceEntry *deventry = device_entry_iter->data;
  priv->devices = g_slist_delete_link(priv->devices, device_entry_iter);

  g_debug("%s: emitting disconnect for object path %s", G_STRFUNC, device_path);
  g_signal_emit (self, SYNCE_DEVICE_MANAGER_GET_CLASS(SYNCE_DEVICE_MANAGER(self))->signals[SYNCE_DEVICE_MANAGER_DEVICE_DISCONNECTED], 0, device_path);
  synce_device_manager_device_entry_free(deventry);
#endif

  return;
}

#if USE_GDBUS
gboolean
synce_device_manager_get_connected_devices (SynceDbusDeviceManager *interface,
                                            GDBusMethodInvocation *invocation,
                                            gpointer userdata)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(userdata);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  gchar **device_list = g_malloc0(g_slist_length(priv->devices) + 1);
  guint n = 0;
  GSList *device_entry_iter = priv->devices;
  while (device_entry_iter != NULL) {

    gchar *obj_path;
    SynceDevice *device = ((DeviceEntry*)device_entry_iter->data)->device;
    if (device == NULL) {
      /* interface is not yet ready */
      device_entry_iter = g_slist_next(device_entry_iter);
      continue;
    }

    g_object_get (device, "object-path", &obj_path, NULL);
    if (obj_path == NULL) {
      /* device is not yet ready */
      device_entry_iter = g_slist_next(device_entry_iter);
      continue;
    }

    g_debug("%s: found device %s with object path %s", G_STRFUNC, ((DeviceEntry*)device_entry_iter->data)->device_path, obj_path);

    device_list[n] = obj_path;
    n++;

    device_entry_iter = g_slist_next(device_entry_iter);
  }

  synce_dbus_device_manager_complete_get_connected_devices(interface, invocation, device_list);

  g_strfreev(device_list);

  return TRUE;
}
#else
gboolean
synce_device_manager_get_connected_devices (SynceDeviceManager *self,
                                            GPtrArray **ret,
                                            GError **error)
{
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  *ret = g_ptr_array_new ();

  GSList *device_entry_iter = priv->devices;

  for (; device_entry_iter; device_entry_iter = g_slist_next(device_entry_iter)) {

  DeviceEntry *deventry = device_entry_iter->data;

  if (!deventry) {
    g_critical("%s: DeviceEntry was null", G_STRFUNC);
    continue;
  }

  g_debug("%s: deventry->device_path = %s", G_STRFUNC, deventry->device_path);
  g_debug("%s: deventry->device_ip = %s", G_STRFUNC, deventry->device_ip);
  g_debug("%s: deventry->local_ip = %s", G_STRFUNC, deventry->local_ip);
  g_debug("%s: deventry->rndis = %s", G_STRFUNC, (deventry->rndis ? "true" : "false"));
  g_debug("%s: deventry->iface_pending = %s", G_STRFUNC, (deventry->iface_pending ? "true" : "false"));

    gchar *obj_path;
    SynceDevice *device = ((DeviceEntry*)device_entry_iter->data)->device;
    if (device == NULL) {
      g_debug("%s: device was null", G_STRFUNC);
      /* interface is not yet ready */
      continue;
    }

    g_object_get (device, "object-path", &obj_path, NULL);
    if (obj_path == NULL) {
      g_debug("%s: object path was null", G_STRFUNC);
      /* device is not yet ready */
      continue;
    }

    g_debug("%s: found device %s with object path %s", G_STRFUNC, ((DeviceEntry*)device_entry_iter->data)->device_path, obj_path);

    g_ptr_array_add (*ret, obj_path);

  }

  return TRUE;
}
#endif

/*
 * class / object functions
 */

static void
synce_device_manager_initable_iface_init (GInitableIface *iface)
{
  iface->init = synce_device_manager_initable_init;
}

static void
synce_device_manager_init (SynceDeviceManager *self)
{
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE(self);

  priv->devices = NULL;
  priv->control_iface = NULL;
  priv->control_connect_id = 0;
  priv->control_disconnect_id = 0;
  priv->iface_check_id = 0;

  return;
}

static gboolean
synce_device_manager_initable_init (GInitable *initable, GCancellable *cancellable, GError **error)
{
  g_return_val_if_fail (SYNCE_IS_DEVICE_MANAGER(initable), FALSE);
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER(initable);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);

  if (cancellable != NULL) {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
			 "Cancellable initialization not supported");
    return FALSE;
  }

#if USE_GDBUS
  priv->interface = synce_dbus_device_manager_skeleton_new();
  g_signal_connect(priv->interface,
		   "handle-get-connected-devices",
		   G_CALLBACK (synce_device_manager_get_connected_devices),
		   self);

  GDBusConnection *system_bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error);
  if (system_bus == NULL) {
    g_critical("%s: Failed to connect to system bus: %s", G_STRFUNC, (*error)->message);
    return FALSE;
  }
  if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(priv->interface),
					system_bus,
                                        DEVICE_MANAGER_OBJECT_PATH,
					error)) {
    g_critical("%s: Failed to export interface on system bus: %s", G_STRFUNC, (*error)->message);
    g_object_unref(system_bus);
    return FALSE;
  }
  g_object_unref(system_bus);
#else
  DBusGConnection *system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, error);
  if (system_bus == NULL) {
    g_critical("%s: Failed to connect to system bus: %s", G_STRFUNC, (*error)->message);
    return FALSE;
  }
  dbus_g_connection_register_g_object (system_bus,
                                       DEVICE_MANAGER_OBJECT_PATH,
				       G_OBJECT(self));
  dbus_g_connection_unref(system_bus);
#endif

  priv->control_iface = g_initable_new(SYNCE_TYPE_DEVICE_MANAGER_CONTROL, NULL, error, NULL);
  if (!(priv->control_iface)) {
    g_critical("%s: failed to create manager control interface: %s", G_STRFUNC, (*error)->message);
    return FALSE;
  }

  priv->control_connect_id = g_signal_connect(priv->control_iface, "device-connected", G_CALLBACK(synce_device_manager_device_connected_cb), self);
  priv->control_disconnect_id = g_signal_connect(priv->control_iface, "device-disconnected", G_CALLBACK(synce_device_manager_device_disconnected_cb), self);

  priv->inited = TRUE;
  return TRUE;
}

static void
synce_device_manager_dispose (GObject *obj)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER (obj);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

#if USE_GDBUS
  if (priv->interface) {
    g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(priv->interface));
    g_object_unref(priv->interface);
  }
#endif

  GSList *list_entry = priv->devices;
  while (list_entry) {
    synce_device_manager_device_entry_free(list_entry->data);
    list_entry = g_slist_next(list_entry);
  }
  g_slist_free(priv->devices);

  g_signal_handler_disconnect(priv->control_iface, priv->control_connect_id);
  g_signal_handler_disconnect(priv->control_iface, priv->control_disconnect_id);
  g_object_unref(priv->control_iface);

  if (G_OBJECT_CLASS (synce_device_manager_parent_class)->dispose)
    G_OBJECT_CLASS (synce_device_manager_parent_class)->dispose (obj);
}

static void
synce_device_manager_finalize (GObject *obj)
{
  SynceDeviceManager *self = SYNCE_DEVICE_MANAGER (obj);
  SynceDeviceManagerPrivate *priv = SYNCE_DEVICE_MANAGER_GET_PRIVATE (self);

  G_OBJECT_CLASS (synce_device_manager_parent_class)->finalize (obj);
}

static void
synce_device_manager_class_init (SynceDeviceManagerClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (SynceDeviceManagerPrivate));

  obj_class->dispose = synce_device_manager_dispose;
  obj_class->finalize = synce_device_manager_finalize;

  klass->signals[SYNCE_DEVICE_MANAGER_DEVICE_CONNECTED] =
    g_signal_new ("device-connected",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  klass->signals[SYNCE_DEVICE_MANAGER_DEVICE_DISCONNECTED] =
    g_signal_new ("device-disconnected",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

#if !USE_GDBUS
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                   &dbus_glib_synce_device_manager_object_info);
#endif
}


