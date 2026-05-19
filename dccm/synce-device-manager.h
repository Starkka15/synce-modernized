#ifndef SYNCE_DEVICE_MANAGER_H
#define SYNCE_DEVICE_MANAGER_H

#include <glib-object.h>


G_BEGIN_DECLS

#define DEVICE_MANAGER_OBJECT_PATH "/org/synce/dccm/DeviceManager"

/* signals */
enum
{
  SYNCE_DEVICE_MANAGER_DEVICE_CONNECTED,
  SYNCE_DEVICE_MANAGER_DEVICE_DISCONNECTED,
  SYNCE_DEVICE_MANAGER_LAST_SIGNAL
};

/* class definition */

typedef struct _SynceDeviceManager SynceDeviceManager;
struct _SynceDeviceManager
{
  GObject parent;
};

typedef struct _SynceDeviceManagerClass SynceDeviceManagerClass;
struct _SynceDeviceManagerClass {
  GObjectClass parent;

  guint signals[SYNCE_DEVICE_MANAGER_LAST_SIGNAL];
};

GType synce_device_manager_get_type (void);

#define SYNCE_TYPE_DEVICE_MANAGER (synce_device_manager_get_type())
#define SYNCE_DEVICE_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SYNCE_TYPE_DEVICE_MANAGER, SynceDeviceManager))
#define SYNCE_DEVICE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SYNCE_TYPE_DEVICE_MANAGER, SynceDeviceManagerClass))
#define SYNCE_IS_DEVICE_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SYNCE_TYPE_DEVICE_MANAGER))
#define SYNCE_IS_DEVICE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SYNCE_TYPE_DEVICE_MANAGER))
#define SYNCE_DEVICE_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SYNCE_TYPE_DEVICE_MANAGER, SynceDeviceManagerClass))

#if !USE_GDBUS
gboolean synce_device_manager_get_connected_devices (SynceDeviceManager *self, GPtrArray **ret, GError **error);
#endif

G_END_DECLS

#endif /* SYNCE_DEVICE_MANAGER_H */
