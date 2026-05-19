#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <glib-object.h>
#include <gio/gio.h>
#if USE_GDBUS
#include "synce-device-manager-control-dbus.h"
#else
#include <dbus/dbus-glib.h>
#include "synce-device-manager-control-glue.h"
#endif

#include "synce-device-manager-control.h"
#include "synce-device-manager-control-signals-marshal.h"


static void     synce_device_manager_control_initable_iface_init (GInitableIface  *iface);
static gboolean synce_device_manager_control_initable_init       (GInitable       *initable,
								  GCancellable    *cancellable,
								  GError         **error);

G_DEFINE_TYPE_WITH_CODE (SynceDeviceManagerControl, synce_device_manager_control, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, synce_device_manager_control_initable_iface_init))

/* private stuff */
typedef struct _SynceDeviceManagerControlPrivate SynceDeviceManagerControlPrivate;

struct _SynceDeviceManagerControlPrivate
{
  gboolean inited;
  gboolean dispose_has_run;
#if USE_GDBUS
  SynceDbusDeviceManagerControl *interface;
#endif
};

#define SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(o) \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), SYNCE_TYPE_DEVICE_MANAGER_CONTROL, SynceDeviceManagerControlPrivate))

#if USE_GDBUS

static gboolean
synce_device_manager_control_device_connected(SynceDbusDeviceManagerControl *interface,
					      GDBusMethodInvocation *invocation,
					      const gchar *device_path, const gchar *device_ip,
					      const gchar *local_ip, gboolean rndis,
					      gpointer userdata)
{
  SynceDeviceManagerControl *self = SYNCE_DEVICE_MANAGER_CONTROL(userdata);
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  g_signal_emit(self,
		SYNCE_DEVICE_MANAGER_CONTROL_GET_CLASS(SYNCE_DEVICE_MANAGER_CONTROL(self))->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_CONNECTED],
		0,
		device_path, device_ip, local_ip, rndis);

  synce_dbus_device_manager_control_complete_device_connected(interface, invocation);

  return TRUE;
}

static gboolean
synce_device_manager_control_device_disconnected(SynceDbusDeviceManagerControl *interface,
                                                 GDBusMethodInvocation *invocation,
                                                 const gchar *device_path,
                                                 gpointer userdata)
{
  SynceDeviceManagerControl *self = SYNCE_DEVICE_MANAGER_CONTROL(userdata);
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(self);
  g_return_val_if_fail(priv->inited && !(priv->dispose_has_run), FALSE);

  g_signal_emit(self,
		SYNCE_DEVICE_MANAGER_CONTROL_GET_CLASS(SYNCE_DEVICE_MANAGER_CONTROL(self))->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_DISCONNECTED],
		0,
		device_path);

  synce_dbus_device_manager_control_complete_device_disconnected(interface, invocation);

  return TRUE;
}

#else
void
synce_device_manager_control_device_connected(SynceDeviceManagerControl *self, gchar *device_path, gchar *device_ip, gchar *local_ip, gboolean rndis, GError **error)
{
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(self);
  g_return_if_fail(priv->inited && !(priv->dispose_has_run));

  g_signal_emit(self,
		SYNCE_DEVICE_MANAGER_CONTROL_GET_CLASS(SYNCE_DEVICE_MANAGER_CONTROL(self))->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_CONNECTED],
		0,
		device_path, device_ip, local_ip, rndis);

  return;
}

void
synce_device_manager_control_device_disconnected(SynceDeviceManagerControl *self, gchar *device_path, GError **error)
{
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(self);
  g_return_if_fail(priv->inited && !(priv->dispose_has_run));

  g_debug("%s: received disconnect for device %s", G_STRFUNC, device_path);
  g_signal_emit(self,
		SYNCE_DEVICE_MANAGER_CONTROL_GET_CLASS(SYNCE_DEVICE_MANAGER_CONTROL(self))->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_DISCONNECTED],
		0,
		device_path);

  return;
}
#endif

static void
synce_device_manager_control_initable_iface_init (GInitableIface *iface)
{
  iface->init = synce_device_manager_control_initable_init;
}

static void
synce_device_manager_control_init (SynceDeviceManagerControl *self)
{
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE(self);

  priv->inited = FALSE;
  priv->dispose_has_run = FALSE;

  return;
}

static gboolean
synce_device_manager_control_initable_init (GInitable *initable, GCancellable *cancellable, GError **error)
{
  g_return_val_if_fail (SYNCE_IS_DEVICE_MANAGER_CONTROL(initable), FALSE);
  SynceDeviceManagerControl *self = SYNCE_DEVICE_MANAGER_CONTROL(initable);
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE (self);

  if (cancellable != NULL) {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
			 "Cancellable initialization not supported");
    return FALSE;
  }


#if USE_GDBUS
  priv->interface = synce_dbus_device_manager_control_skeleton_new();
  g_signal_connect(priv->interface,
		   "handle-device-connected",
		   G_CALLBACK (synce_device_manager_control_device_connected),
		   self);
  g_signal_connect(priv->interface,
		   "handle-device-disconnected",
		   G_CALLBACK (synce_device_manager_control_device_disconnected),
		   self);

  GDBusConnection *system_bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error);
  if (system_bus == NULL) {
    g_critical("%s: Failed to connect to system bus: %s", G_STRFUNC, (*error)->message);
    return FALSE;
  }
  if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(priv->interface),
					system_bus,
                                        DEVICE_MANAGER_CONTROL_OBJECT_PATH,
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
                                       DEVICE_MANAGER_CONTROL_OBJECT_PATH,
				       G_OBJECT(self));
  dbus_g_connection_unref(system_bus);
#endif

  priv->inited = TRUE;
  return TRUE;
}

static void
synce_device_manager_control_dispose (GObject *obj)
{
  SynceDeviceManagerControl *self = SYNCE_DEVICE_MANAGER_CONTROL (obj);
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

#if USE_GDBUS
  if (priv->interface) {
    g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(priv->interface));
    g_object_unref(priv->interface);
  }
#endif

  if (G_OBJECT_CLASS (synce_device_manager_control_parent_class)->dispose)
    G_OBJECT_CLASS (synce_device_manager_control_parent_class)->dispose (obj);
}

static void
synce_device_manager_control_finalize (GObject *obj)
{
  SynceDeviceManagerControl *self = SYNCE_DEVICE_MANAGER_CONTROL (obj);
  SynceDeviceManagerControlPrivate *priv = SYNCE_DEVICE_MANAGER_CONTROL_GET_PRIVATE (self);

  G_OBJECT_CLASS (synce_device_manager_control_parent_class)->finalize (obj);
}

static void
synce_device_manager_control_class_init (SynceDeviceManagerControlClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (SynceDeviceManagerControlPrivate));

  obj_class->dispose = synce_device_manager_control_dispose;
  obj_class->finalize = synce_device_manager_control_finalize;

  klass->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_CONNECTED] =
    g_signal_new ("device-connected",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  synce_device_manager_control_marshal_VOID__STRING_STRING_STRING_BOOLEAN,
                  G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

  klass->signals[SYNCE_DEVICE_MANAGER_CONTROL_DEVICE_DISCONNECTED] =
    g_signal_new ("device-disconnected",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

#if !USE_GDBUS
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                   &dbus_glib_synce_device_manager_control_object_info);
#endif
}

