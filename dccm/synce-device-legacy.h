#ifndef SYNCE_DEVICE_LEGACY_H
#define SYNCE_DEVICE_LEGACY_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _SynceDeviceLegacy SynceDeviceLegacy;
struct _SynceDeviceLegacy {
  SynceDevice parent;
};

typedef struct _SynceDeviceLegacyClass SynceDeviceLegacyClass;
struct _SynceDeviceLegacyClass {
  SynceDeviceClass parent_class;
};

GType synce_device_legacy_get_type (void);

#define SYNCE_TYPE_DEVICE_LEGACY (synce_device_legacy_get_type())
#define SYNCE_DEVICE_LEGACY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYNCE_TYPE_DEVICE_LEGACY, SynceDeviceLegacy))
#define SYNCE_DEVICE_LEGACY_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), SYNCE_TYPE_DEVICE_LEGACY, SynceDeviceLegacyClass))
#define SYNCE_IS_DEVICE_LEGACY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYNCE_TYPE_DEVICE_LEGACY))
#define SYNCE_IS_DEVICE_LEGACY_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), SYNCE_TYPE_DEVICE_LEGACY))
#define SYNCE_DEVICE_LEGACY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SYNCE_TYPE_DEVICE_LEGACY, SynceDeviceLegacyClass))

G_END_DECLS

#endif /* SYNCE_DEVICE_LEGACY_H */
