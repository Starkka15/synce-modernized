#ifndef SYNCE_CONNECTION_BROKER_H
#define SYNCE_CONNECTION_BROKER_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _SynceConnectionBroker SynceConnectionBroker;
typedef struct _SynceConnectionBrokerClass SynceConnectionBrokerClass;

struct _SynceConnectionBroker
{
  GObject parent;
};

struct _SynceConnectionBrokerClass
{
  GObjectClass parent_class;
};

GType synce_connection_broker_get_type (void);

#define SYNCE_TYPE_CONNECTION_BROKER \
    (synce_connection_broker_get_type())
#define SYNCE_CONNECTION_BROKER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), SYNCE_TYPE_CONNECTION_BROKER, SynceConnectionBroker))
#define SYNCE_CONNECTION_BROKER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), SYNCE_TYPE_CONNECTION_BROKER, SynceConnectionBrokerClass))
#define SYNCE_IS_CONNECTION_BROKER(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), SYNCE_TYPE_CONNECTION_BROKER))
#define SYNCE_IS_CONNECTION_BROKER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), SYNCE_TYPE_CONNECTION_BROKER))
#define SYNCE_CONNECTION_BROKER_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), SYNCE_TYPE_CONNECTION_BROKER, SynceConnectionBrokerClass))

void _synce_connection_broker_take_connection (SynceConnectionBroker *self, GSocketConnection *conn);

G_END_DECLS

#endif /* SYNCE_CONNECTION_BROKER_H */
