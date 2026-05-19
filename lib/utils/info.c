/* $Id: info.c 4165 2013-05-10 13:49:52Z mark_ellis $ */
#define _BSD_SOURCE 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "synce.h"
#include "synce_log.h"
#include "config/config.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#if ENABLE_ODCCM_SUPPORT || ENABLE_UDEV_SUPPORT
#if USE_GDBUS
#include <glib-object.h>
#include <gio/gio.h>
#else
#define DBUS_API_SUBJECT_TO_CHANGE 1
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#endif
#endif

struct _SynceInfo
{
  pid_t dccm_pid;
  char* device_ip;
  char* local_iface_ip;
  char* password;
  int key;
  unsigned int os_major;
  unsigned int os_minor;
  unsigned int build_number;
  unsigned int processor_type;
  unsigned int partner_id_1;
  unsigned int partner_id_2;
  char* guid;
  char* name;
  char* os_name;
  char* model;
  char* transport;
  char* object_path;
};

#if ENABLE_ODCCM_SUPPORT || ENABLE_UDEV_SUPPORT
static const char* const DBUS_SERVICE       = "org.freedesktop.DBus";
static const char* const DBUS_IFACE         = "org.freedesktop.DBus";
static const char* const DBUS_PATH          = "/org/freedesktop/DBus";
#endif

#if ENABLE_ODCCM_SUPPORT
static const char* const ODCCM_SERVICE      = "org.synce.odccm";
static const char* const ODCCM_MGR_PATH     = "/org/synce/odccm/DeviceManager";
static const char* const ODCCM_MGR_IFACE    = "org.synce.odccm.DeviceManager";
static const char* const ODCCM_DEV_IFACE    = "org.synce.odccm.Device";
#endif

#if ENABLE_UDEV_SUPPORT
static const char* const DCCM_SERVICE      = "org.synce.dccm";
static const char* const DCCM_MGR_PATH     = "/org/synce/dccm/DeviceManager";
static const char* const DCCM_MGR_IFACE    = "org.synce.dccm.DeviceManager";
static const char* const DCCM_DEV_IFACE    = "org.synce.dccm.Device";
#endif

#define FREE(x)     if(x) free(x)

#if ENABLE_DCCM_FILE_SUPPORT
static char *STRDUP(const char* str)
{
  return str ? strdup(str) : NULL;
}

static SynceInfo* synce_info_from_file(SynceInfoIdField field, const char* data)
{
  SynceInfo* result = calloc(1, sizeof(SynceInfo));
  bool success = false;
  char* connection_filename;
  struct configFile* config = NULL;

  if (data) {
    if (field == INFO_NAME) {
      char *synce_dir;
      if (!synce_get_directory(&synce_dir)) {
        synce_error("unable to determine synce directory");
        goto exit;
      }
      int path_len = strlen(synce_dir) + strlen(data) + 2;
      connection_filename = (char *) malloc(sizeof(char) * path_len);

      if (snprintf(connection_filename, path_len, "%s/%s", synce_dir, data) >= path_len) {
        FREE(synce_dir);
        synce_error("error determining synce connection filename");
        goto exit;
      }
    }
    if (field == INFO_OBJECT_PATH) {
      connection_filename = strdup(data);
    }
  }
  else
    synce_get_connection_filename(&connection_filename);

  config = readConfigFile(connection_filename);
  if (!config)
    {
      synce_error("unable to open file: %s", connection_filename);
      goto exit;
    }

  result->dccm_pid        = getConfigInt(config, "dccm",   "pid");

  result->key             = getConfigInt(config, "device", "key");
  result->os_major        = getConfigInt(config, "device", "os_version");
  result->os_minor        = getConfigInt(config, "device", "os_minor");
  result->build_number    = getConfigInt(config, "device", "build_number");
  result->processor_type  = getConfigInt(config, "device", "processor_type");
  result->partner_id_1    = getConfigInt(config, "device", "partner_id_1");
  result->partner_id_2    = getConfigInt(config, "device", "partner_id_2");

  result->device_ip       = STRDUP(getConfigString(config, "device", "ip"));
  result->local_iface_ip  = NULL;
  result->password  = STRDUP(getConfigString(config, "device", "password"));
  result->name      = STRDUP(getConfigString(config, "device", "name"));
  result->object_path     = STRDUP(connection_filename);

  if (!(result->os_name = STRDUP(getConfigString(config, "device", "os_name"))))
          result->os_name = STRDUP(getConfigString(config, "device", "class"));

  if (!(result->model = STRDUP(getConfigString(config, "device", "model"))))
          result->model = STRDUP(getConfigString(config, "device", "hardware"));

  result->transport = STRDUP(getConfigString(config, "connection", "transport"));

  success = true;

exit:
  FREE(connection_filename);

  if (config)
    unloadConfigFile(config);

  if (success)
    return result;
  else
  {
    synce_info_destroy(result);
    return NULL;
  }
}

#endif /* ENABLE_DCCM_FILE_SUPPORT */

#if ENABLE_ODCCM_SUPPORT || ENABLE_UDEV_SUPPORT

#if USE_GDBUS

static gboolean
synce_info_fields_from_dbus(SynceInfo *result, GDBusProxy *proxy)
{
  GError *error = NULL;
  GVariant *res = NULL;

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetName",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device name: %s", G_STRFUNC, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(s)", &(result->name));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetOsVersion",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device OS for %s: %s", G_STRFUNC, result->name, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(uu)", &(result->os_major), &(result->os_minor));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetCpuType",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device cpu type for %s: %s", G_STRFUNC, result->name, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(u)", &(result->processor_type));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetPlatformName",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device platform name for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(s)", &(result->os_name));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetModelName",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device model name for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(s)", &(result->model));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetIpAddress",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get device IP address for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(s)", &(result->device_ip));
  g_variant_unref (res);

  if (!(res = g_dbus_proxy_call_sync(proxy, "GetGuid",
				g_variant_new ("()"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error)))
    {
      g_warning("%s: Failed to get GUID for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  g_variant_get (res, "(s)", &(result->guid));
  g_variant_unref (res);

  return TRUE;

ERROR:
  if (error != NULL)
    g_error_free(error);
  return FALSE;
}

static SynceInfo *synce_info_from_udev_or_odccm(SynceInfoIdField field, const char* data, gboolean udev)
{
  SynceInfo *result = NULL;
  GError *error = NULL;
  GDBusProxy *dbus_proxy = NULL;
  GDBusProxy *mgr_proxy = NULL;
  guint i;
  gboolean dccm_running = FALSE;
  GVariant *ret = NULL;
  gchar **devices = NULL;
  const gchar *service = NULL;
  const gchar *mgr_path = NULL;
  const gchar *mgr_iface = NULL;
  const gchar *dev_iface = NULL;
  const gchar *dccm_name = NULL;
  if (udev) {
#if ENABLE_UDEV_SUPPORT
    service = DCCM_SERVICE;
    mgr_path = DCCM_MGR_PATH;
    mgr_iface = DCCM_MGR_IFACE;
    dev_iface = DCCM_DEV_IFACE;
    dccm_name = "udev";
#endif
  } else {
#if ENABLE_ODCCM_SUPPORT
    service = ODCCM_SERVICE;
    mgr_path = ODCCM_MGR_PATH;
    mgr_iface = ODCCM_MGR_IFACE;
    dev_iface = ODCCM_DEV_IFACE;
    dccm_name = "odccm";
#endif
  }

  g_type_init();

  dbus_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
					      G_DBUS_PROXY_FLAGS_NONE,
					      NULL, /* GDBusInterfaceInfo */
					      DBUS_SERVICE,
					      DBUS_PATH,
					      DBUS_IFACE,
					      NULL, /* GCancellable */
					      &error);
  if (dbus_proxy == NULL) {
    g_warning("%s: Failed to get dbus proxy object: %s", G_STRFUNC, error->message);
    goto ERROR;
  }

  ret = g_dbus_proxy_call_sync (dbus_proxy,
				"NameHasOwner",
				g_variant_new ("(s)", service),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				&error);
  if (!ret) {
    g_critical("%s: Error checking owner of service %s: %s", G_STRFUNC, service, error->message);
    g_object_unref(dbus_proxy);
    goto ERROR;
  }
  g_variant_get (ret, "(b)", &dccm_running);
  g_variant_unref (ret);

  g_object_unref(dbus_proxy);
  if (!dccm_running) {
    g_message("%s is not running, ignoring", dccm_name);
    goto ERROR;
  }

  mgr_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
					     G_DBUS_PROXY_FLAGS_NONE,
					     NULL, /* GDBusInterfaceInfo */
					     service,
					     mgr_path,
					     mgr_iface,
					     NULL, /* GCancellable */
					     &error);
  if (mgr_proxy == NULL) {
    g_warning("%s: Failed to get DeviceManager proxy object: %s", G_STRFUNC, error->message);
    goto ERROR;
  }

  ret = g_dbus_proxy_call_sync (mgr_proxy,
				"GetConnectedDevices",
				g_variant_new ("()"),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				&error);
  if (!ret) {
    g_warning("%s: Failed to get devices: %s", G_STRFUNC, error->message);
    goto ERROR;
  }
  g_variant_get (ret, "(^ao)", &devices);
  g_variant_unref (ret);

  if (g_strv_length(devices) == 0) {
    g_message("No devices connected to %s", dccm_name);
    goto ERROR;
  }

  for (i = 0; i < g_strv_length(devices); i++) {
    gchar *obj_path = devices[i];
    gchar *match_data = NULL;

    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
					     G_DBUS_PROXY_FLAGS_NONE,
					     NULL, /* GDBusInterfaceInfo */
					     service,
					     obj_path,
					     dev_iface,
					     NULL, /* GCancellable */
					     &error);
    if (proxy == NULL) {
      g_warning("%s: Failed to get proxy for device '%s': %s", G_STRFUNC, obj_path, error->message);
      goto ERROR;
    }

    if (data != NULL)
    {
      switch (field)
        {
        case INFO_NAME:
	  ret = g_dbus_proxy_call_sync (proxy,
					"GetName",
					g_variant_new ("()"),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);
	  if (!ret) {
	    g_warning("%s: Failed to get device name: %s", G_STRFUNC, error->message);
	    g_object_unref(proxy);
	    goto ERROR;
	  }
	  g_variant_get (ret, "(s)", &match_data);
	  g_variant_unref (ret);

          break;
        case INFO_OBJECT_PATH:
          match_data = g_strdup(obj_path);
          break;
        }

      if (strcasecmp(data, match_data) != 0) {
        g_free(match_data);
	g_object_unref(proxy);
        continue;
      }
    }

    g_free(match_data);

    if (!(result = calloc(1, sizeof(SynceInfo)))) {
      g_critical("%s: Failed to allocate SynceInfo", G_STRFUNC);
      g_object_unref(proxy);
      goto ERROR;
    }

    result->object_path = g_strdup(obj_path);

    if (!synce_info_fields_from_dbus(result, proxy)) {
      g_object_unref(proxy);
      goto ERROR;
    }

    if (udev) {
      ret = g_dbus_proxy_call_sync (proxy,
				    "GetIfaceAddress",
				    g_variant_new ("()"),
				    G_DBUS_CALL_FLAGS_NONE,
				    -1,
				    NULL,
				    &error);
      if (!ret) {
	g_warning("%s: Failed to get local interface IP address for %s: %s", G_STRFUNC, result->name, error->message);
	g_object_unref(proxy);
	goto ERROR;
      }
      g_variant_get (ret, "(s)", &(result->local_iface_ip));
      g_variant_unref (ret);
    } else {
      result->local_iface_ip = NULL;
    }

    g_object_unref(proxy);

    result->transport = g_strdup(dccm_name);

    break;
  }

  goto OUT;

ERROR:
  if (error != NULL)
    g_error_free(error);
  if (result) synce_info_destroy(result);
  result = NULL;

OUT:
  g_strfreev(devices);

  if (mgr_proxy != NULL)
    g_object_unref (mgr_proxy);

  return result;
}

#else /* USE_GDBUS */

#define DCCM_TYPE_OBJECT_PATH_ARRAY \
  (dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH))

static gboolean
synce_info_fields_from_dbus(SynceInfo *result, DBusGProxy *proxy)
{
  GError *error = NULL;
  gchar *name;
  guint os_major;
  guint os_minor;
  guint cpu_type;
  gchar* os_name;
  gchar* model;
  gchar* ip;

  if (!dbus_g_proxy_call(proxy, "GetName", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &name,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device name: %s", G_STRFUNC, error->message);
      goto ERROR;
    }

  result->name = name;

  if (!dbus_g_proxy_call(proxy, "GetOsVersion", &error,
			 G_TYPE_INVALID,
			 G_TYPE_UINT, &os_major,
			 G_TYPE_UINT, &os_minor,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device OS for %s: %s", G_STRFUNC, result->name, error->message);
      goto ERROR;
    }
  result->os_major = os_major;
  result->os_minor = os_minor;

  if (!dbus_g_proxy_call(proxy, "GetCpuType", &error,
			 G_TYPE_INVALID,
			 G_TYPE_UINT, &cpu_type,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device cpu type for %s: %s", G_STRFUNC, result->name, error->message);
      goto ERROR;
    }
  result->processor_type = cpu_type;

  if (!dbus_g_proxy_call(proxy, "GetPlatformName", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &os_name,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device platform name for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  result->os_name = os_name;

  if (!dbus_g_proxy_call(proxy, "GetModelName", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &model,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device model name for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  result->model = model;

  if (!dbus_g_proxy_call(proxy, "GetIpAddress", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &ip,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get device IP address for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  result->device_ip = ip;

  gchar* guid;
  if (!dbus_g_proxy_call(proxy, "GetGuid", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &guid,
			 G_TYPE_INVALID))
    {
      g_warning("%s: Failed to get GUID for %s: %s", result->name, G_STRFUNC, error->message);
      goto ERROR;
    }
  result->guid = guid;

  return TRUE;

ERROR:
  if (error != NULL)
    g_error_free(error);
  return FALSE;
}

static SynceInfo *synce_info_from_udev_or_odccm(SynceInfoIdField field, const char* data, gboolean udev)
{
  SynceInfo *result = NULL;
  GError *error = NULL;
  DBusGConnection *bus = NULL;
  DBusGProxy *dbus_proxy = NULL;
  DBusGProxy *mgr_proxy = NULL;
  GPtrArray *devices = NULL;
  guint i;
  gboolean dccm_running = FALSE;
  if (udev) {
#if ENABLE_UDEV_SUPPORT
    service = DCCM_SERVICE;
    mgr_path = DCCM_MGR_PATH;
    mgr_iface = DCCM_MGR_IFACE;
    dev_iface = DCCM_DEV_IFACE;
    dccm_name = "udev";
#endif
  } else {
#if ENABLE_ODCCM_SUPPORT
    service = ODCCM_SERVICE;
    mgr_path = ODCCM_MGR_PATH;
    mgr_iface = ODCCM_MGR_IFACE;
    dev_iface = ODCCM_DEV_IFACE;
    dccm_name = "odccm";
#endif
  }

  g_type_init();

  bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
  if (bus == NULL) {
    g_warning("%s: Failed to connect to system bus: %s", G_STRFUNC, error->message);
    goto ERROR;
  }

  dbus_proxy = dbus_g_proxy_new_for_name (bus,
                                          DBUS_SERVICE,
                                          DBUS_PATH,
                                          DBUS_IFACE);
  if (dbus_proxy == NULL) {
    g_warning("%s: Failed to get dbus proxy object", G_STRFUNC);
    goto ERROR;
  }

  if (!(dbus_g_proxy_call(dbus_proxy, "NameHasOwner",
                          &error,
                          G_TYPE_STRING, service,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &dccm_running,
                          G_TYPE_INVALID))) {
          g_critical("%s: Error checking owner of service %s: %s", G_STRFUNC, service, error->message);
          g_object_unref(dbus_proxy);
          goto ERROR;
  }

  g_object_unref(dbus_proxy);
  if (!dccm_running) {
          g_message("%s is not running, ignoring", dccm_name);
          goto ERROR;
  }

  mgr_proxy = dbus_g_proxy_new_for_name(bus, service,
                                        mgr_path,
                                        mgr_iface);
  if (mgr_proxy == NULL) {
    g_warning("%s: Failed to get DeviceManager proxy object", G_STRFUNC);
    goto ERROR;
  }

  if (!dbus_g_proxy_call(mgr_proxy, "GetConnectedDevices", &error,
                         G_TYPE_INVALID,
                         DCCM_TYPE_OBJECT_PATH_ARRAY, &devices,
                         G_TYPE_INVALID))
  {
    g_warning("%s: Failed to get devices: %s", G_STRFUNC, error->message);
    goto ERROR;
  }

  if (devices->len == 0) {
    g_message("No devices connected to %s", dccm_name);
    goto ERROR;
  }

  for (i = 0; i < devices->len; i++) {
    gchar *obj_path = g_ptr_array_index(devices, i);
    gchar *match_data = NULL;
    DBusGProxy *proxy = dbus_g_proxy_new_for_name(bus, service,
                                                  obj_path,
                                                  dev_iface);
    if (proxy == NULL) {
      g_warning("%s: Failed to get proxy for device '%s'", G_STRFUNC, obj_path);
      goto ERROR;
    }

    if (data != NULL)
    {
      switch (field)
        {
        case INFO_NAME:
          if (!dbus_g_proxy_call(proxy, "GetName", &error,
                                 G_TYPE_INVALID,
                                 G_TYPE_STRING, &match_data,
                                 G_TYPE_INVALID))
            {
              g_warning("%s: Failed to get device name: %s", G_STRFUNC, error->message);
              g_object_unref(proxy);
              goto ERROR;
            }
          break;
        case INFO_OBJECT_PATH:
          match_data = g_strdup(obj_path);
          break;
        }

      if (strcasecmp(data, match_data) != 0) {
        g_free(match_data);
        g_object_unref(proxy);
        continue;
      }
    }

    g_free(match_data);

    if (!(result = calloc(1, sizeof(SynceInfo)))) {
      g_critical("%s: Failed to allocate SynceInfo", G_STRFUNC);
      g_object_unref(proxy);
      goto ERROR;
    }

    result->object_path = g_strdup(obj_path);

    if (!synce_info_fields_from_dbus(result, proxy)) {
      g_object_unref(proxy);
      goto ERROR;
    }

    if (udev) {
      gchar* iface_ip;
      if (!dbus_g_proxy_call(proxy, "GetIfaceAddress", &error,
			     G_TYPE_INVALID,
			     G_TYPE_STRING, &iface_ip,
			     G_TYPE_INVALID))
	{
	  g_warning("%s: Failed to get local interface IP address for %s: %s", result->name, G_STRFUNC, error->message);
	  g_object_unref(proxy);
	  goto ERROR;
	}
      result->local_iface_ip = iface_ip;
    } else {
      result->local_iface_ip = NULL;
    }

    g_object_unref(proxy);

    result->transport = g_strdup(dccm_name);

    break;
  }

  goto OUT;

ERROR:
  if (error != NULL)
    g_error_free(error);
  if (result) synce_info_destroy(result);
  result = NULL;

OUT:
  if (devices != NULL) {
    for (i = 0; i < devices->len; i++)
      g_free(g_ptr_array_index(devices, i));

    g_ptr_array_free(devices, TRUE);
  }

  if (mgr_proxy != NULL)
    g_object_unref (mgr_proxy);
  if (bus != NULL)
    dbus_g_connection_unref (bus);

  return result;
}

#endif /* USE_GDBUS */
#endif /* ENABLE_ODCCM_SUPPORT || ENABLE_UDEV_SUPPORT */


SynceInfo* synce_info_new(const char* device_name)
{
  return synce_info_new_by_field(INFO_NAME, device_name);
}

SynceInfo* synce_info_new_by_field(SynceInfoIdField field, const char* data)
{
  SynceInfo* result = NULL;

#if ENABLE_UDEV_SUPPORT
  result = synce_info_from_udev_or_odccm(field, data, TRUE);
#endif

#if ENABLE_ODCCM_SUPPORT
  if (!result)
    result = synce_info_from_udev_or_odccm(field, data, FALSE);

#if ENABLE_MIDASYNC
  if (!result)
    result = synce_info_from_midasyncd(field, data);
#endif
#endif

#if ENABLE_DCCM_FILE_SUPPORT
  if (!result)
    result = synce_info_from_file(field, data);
#endif

  return result;
}


void synce_info_destroy(SynceInfo* info)
{
  if (info)
  {
    FREE(info->device_ip);
    FREE(info->local_iface_ip);
    FREE(info->password);
    FREE(info->name);
    FREE(info->os_name);
    FREE(info->model);
    FREE(info->transport);
    FREE(info->object_path);
    free(info);
  }
}

const char *
synce_info_get_name(SynceInfo *info)
{
  if (!info) return NULL;
  return info->name;
}

bool
synce_info_get_os_version(SynceInfo *info, unsigned int *os_major, unsigned int *os_minor)
{
  if (!info) return NULL;
  if ((!os_major) || (!os_minor))
    return false;

  *os_major = info->os_major;
  *os_minor = info->os_minor;
  return true;
}

unsigned int
synce_info_get_build_number(SynceInfo *info)
{
  if (!info) return 0;
  return info->build_number;
}

unsigned int
synce_info_get_processor_type(SynceInfo *info)
{
  if (!info) return 0;
  return info->processor_type;
}

const char *
synce_info_get_os_name(SynceInfo *info)
{
  if (!info) return NULL;
  return info->os_name;
}

const char *
synce_info_get_model(SynceInfo *info)
{
  if (!info) return NULL;
  return info->model;
}

const char *
synce_info_get_device_ip(SynceInfo *info)
{
  if (!info) return NULL;
  return info->device_ip;
}

const char *
synce_info_get_local_ip(SynceInfo *info)
{
  if (!info) return NULL;
  return info->local_iface_ip;
}

const char *
synce_info_get_guid(SynceInfo *info)
{
  if (!info) return NULL;
  return info->guid;
}

unsigned int
synce_info_get_partner_id_1(SynceInfo *info)
{
  if (!info) return 0;
  return info->partner_id_1;
}

unsigned int
synce_info_get_partner_id_2(SynceInfo *info)
{
  if (!info) return 0;
  return info->partner_id_2;
}

const char *
synce_info_get_object_path(SynceInfo *info)
{
  if (!info) return NULL;
  return info->object_path;
}

pid_t
synce_info_get_dccm_pid(SynceInfo *info)
{
  if (!info) return 0;
  return info->dccm_pid;
}

const char *
synce_info_get_transport(SynceInfo *info)
{
  if (!info) return NULL;
  return info->transport;
}

const char *
synce_info_get_password(SynceInfo *info)
{
  if (!info) return NULL;
  return info->password;
}

int
synce_info_get_key(SynceInfo *info)
{
  if (!info) return 0;
  return info->key;
}
