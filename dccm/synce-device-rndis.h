#ifndef SYNCE_DEVICE_RNDIS_H
#define SYNCE_DEVICE_RNDIS_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _SynceDeviceRndis SynceDeviceRndis;
struct _SynceDeviceRndis {
  SynceDevice parent;
};

typedef struct _SynceDeviceRndisClass SynceDeviceRndisClass;
struct _SynceDeviceRndisClass {
  SynceDeviceClass parent_class;
};

GType synce_device_rndis_get_type (void);

#define SYNCE_TYPE_DEVICE_RNDIS (synce_device_rndis_get_type())
#define SYNCE_DEVICE_RNDIS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYNCE_TYPE_DEVICE_RNDIS, SynceDeviceRndis))
#define SYNCE_DEVICE_RNDIS_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), SYNCE_TYPE_DEVICE_RNDIS, SynceDeviceRndisClass))
#define SYNCE_IS_DEVICE_RNDIS(obj) (G_TYPE_CHECK_TYPE ((obj), SYNCE_TYPE_DEVICE_RNDIS))
#define SYNCE_IS_DEVICE_RNDIS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), SYNCE_TYPE_DEVICE_RNDIS))
#define SYNCE_DEVICE_RNDIS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SYNCE_TYPE_DEVICE_RNDIS, SynceDeviceRndisClass))

void synce_device_rndis_client_connected (SynceDeviceRndis *self, GSocketConnection *conn);

G_END_DECLS

#endif /* SYNCE_DEVICE_RNDIS_H */
