#ifndef SYNCE_DEVICE_H
#define SYNCE_DEVICE_H

#include <glib-object.h>
#include <gio/gio.h>
#if USE_GDBUS
#include "synce-device-dbus.h"
#else
#include <dbus/dbus-glib.h>
#endif

G_BEGIN_DECLS

#define DEVICE_BASE_OBJECT_PATH "/org/synce/dccm/Device"

typedef enum _SynceDeviceSignals SynceDeviceSignals;
enum _SynceDeviceSignals
{
  SYNCE_DEVICE_SIGNAL_PASSWORD_FLAGS_CHANGED,
  SYNCE_DEVICE_SIGNAL_DISCONNECTED,
  SYNCE_DEVICE_LAST_SIGNAL
};

/* class definition */

typedef struct _SynceDevice SynceDevice;
struct _SynceDevice
{
  GObject parent;
};

typedef struct _SynceDeviceClass SynceDeviceClass;
struct _SynceDeviceClass {
  GObjectClass parent;

  guint signals[SYNCE_DEVICE_LAST_SIGNAL];

  void (*synce_device_conn_event_cb) (GObject *istream, GAsyncResult *res, gpointer user_data);
#if USE_GDBUS
  gboolean (*synce_device_request_connection) (SynceDbusDevice *interface, GDBusMethodInvocation *invocation, gpointer userdata);
  gboolean (*synce_device_provide_password) (SynceDbusDevice *interface, GDBusMethodInvocation *invocation, const gchar *password, gpointer userdata);
#else
  void (*synce_device_request_connection) (SynceDevice *self, DBusGMethodInvocation *ctx);
  void (*synce_device_provide_password) (SynceDevice *self, const gchar *password, DBusGMethodInvocation *ctx);
#endif
};

GType synce_device_get_type (void);

#define SYNCE_TYPE_DEVICE (synce_device_get_type())
#define SYNCE_DEVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SYNCE_TYPE_DEVICE, SynceDevice))
#define SYNCE_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SYNCE_TYPE_DEVICE, SynceDeviceClass))
#define SYNCE_IS_DEVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SYNCE_TYPE_DEVICE))
#define SYNCE_IS_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SYNCE_TYPE_DEVICE))
#define SYNCE_DEVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SYNCE_TYPE_DEVICE, SynceDeviceClass))

#if !USE_GDBUS
gboolean synce_device_get_name (SynceDevice *self, gchar **name, GError **error);
gboolean synce_device_get_platform_name (SynceDevice *self, gchar **platform_name, GError **error);
gboolean synce_device_get_model_name (SynceDevice *self, gchar **model_name, GError **error);
gboolean synce_device_get_os_version (SynceDevice *self, guint *os_major, guint *os_minor, GError **error);
gboolean synce_device_get_version (SynceDevice *self, guint *version, GError **error);
gboolean synce_device_get_cpu_type (SynceDevice *self, guint *cpu_type, GError **error);
gboolean synce_device_get_ip_address (SynceDevice *self, gchar **ip_address, GError **error);
gboolean synce_device_get_iface_address (SynceDevice *self, gchar **iface_address, GError **error);
gboolean synce_device_get_guid (SynceDevice *self, gchar **guid, GError **error);
gboolean synce_device_get_current_partner_id (SynceDevice *self, guint *cur_partner_id, GError **error);
gboolean synce_device_get_password_flags (SynceDevice *self, gchar **pw_flag, GError **error);
#endif

G_END_DECLS

#endif /* SYNCE_DEVICE_H */
