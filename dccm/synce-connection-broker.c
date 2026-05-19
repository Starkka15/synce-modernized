#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "synce-connection-broker.h"

#if USE_GDBUS
#include "synce-device-dbus.h"
#else
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#endif

#include "utils.h"

G_DEFINE_TYPE (SynceConnectionBroker, synce_connection_broker, G_TYPE_OBJECT)

/* properties */
enum
{
  PROP_ID = 1,
  PROP_CONTEXT,

  LAST_PROPERTY
};

/* signals */
enum
{
  DONE,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

/* private stuff */
typedef struct _SynceConnectionBrokerPrivate SynceConnectionBrokerPrivate;

struct _SynceConnectionBrokerPrivate
{
  gboolean dispose_has_run;

  guint id;
#if USE_GDBUS
  GDBusMethodInvocation *ctx;
#else
  DBusGMethodInvocation *ctx;
#endif
  GSocketConnection *conn;
  gchar *filename;
  GSocketService *server;
};

#define SYNCE_CONNECTION_BROKER_GET_PRIVATE(o) \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), SYNCE_TYPE_CONNECTION_BROKER, SynceConnectionBrokerPrivate))

static void
synce_connection_broker_init (SynceConnectionBroker *self)
{
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE(self);

  priv->ctx = NULL;
  priv->conn = NULL;
  priv->filename = NULL;
  priv->server = NULL;
}

static void
synce_connection_broker_get_property (GObject    *obj,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  SynceConnectionBroker *self = SYNCE_CONNECTION_BROKER (obj);
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);

  switch (property_id) {
    case PROP_ID:
      g_value_set_uint (value, priv->id);
      break;
    case PROP_CONTEXT:
      g_value_set_pointer (value, priv->ctx);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
      break;
  }
}

static void
synce_connection_broker_set_property (GObject      *obj,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  SynceConnectionBroker *self = SYNCE_CONNECTION_BROKER (obj);
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);

  switch (property_id) {
    case PROP_ID:
      priv->id = g_value_get_uint (value);
      break;
    case PROP_CONTEXT:
      priv->ctx = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
      break;
  }
}

static void
synce_connection_broker_dispose (GObject *obj)
{
  SynceConnectionBroker *self = SYNCE_CONNECTION_BROKER (obj);
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  if (priv->conn != NULL)
    g_object_unref (priv->conn);

  if (priv->server != NULL) {
    g_socket_service_stop(priv->server);
    g_object_unref(priv->server);
  }

  if (G_OBJECT_CLASS (synce_connection_broker_parent_class)->dispose)
    G_OBJECT_CLASS (synce_connection_broker_parent_class)->dispose (obj);
}

static void
synce_connection_broker_finalize (GObject *obj)
{
  SynceConnectionBroker *self = SYNCE_CONNECTION_BROKER (obj);
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);

  g_free (priv->filename);

  G_OBJECT_CLASS (synce_connection_broker_parent_class)->finalize (obj);
}

static void
synce_connection_broker_class_init (SynceConnectionBrokerClass *conn_broker_class)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (conn_broker_class);
  GParamSpec *param_spec;

  g_type_class_add_private (conn_broker_class,
                            sizeof (SynceConnectionBrokerPrivate));

  obj_class->get_property = synce_connection_broker_get_property;
  obj_class->set_property = synce_connection_broker_set_property;

  obj_class->dispose = synce_connection_broker_dispose;
  obj_class->finalize = synce_connection_broker_finalize;

  param_spec = g_param_spec_uint ("id", "Unique id",
                                  "Unique id.",
                                  0, G_MAXUINT32, 0,
                                  G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_READWRITE |
                                  G_PARAM_STATIC_NICK |
                                  G_PARAM_STATIC_BLURB);
  g_object_class_install_property (obj_class, PROP_ID, param_spec);

  param_spec = g_param_spec_pointer ("context", "D-Bus context",
                                     "D-Bus method invocation context.",
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_READWRITE |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB);
  g_object_class_install_property (obj_class, PROP_CONTEXT, param_spec);

  signals[DONE] =
    g_signal_new ("done",
                  G_OBJECT_CLASS_TYPE (conn_broker_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static gboolean
server_socket_readable_cb(GSocketService *source,
			  GSocketConnection *client_connection,
			  GObject *source_object,
			  gpointer user_data)
{
  SynceConnectionBroker *self = user_data;
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);
  gint device_fd, client_fd;
  ssize_t ret;
  struct msghdr msg = { 0, };
  struct cmsghdr *cmsg;
  gchar cmsg_buf[CMSG_SPACE (sizeof (device_fd))];
  struct iovec iov;
  guchar dummy_byte = 0x7f;

  GSocket *client_socket = g_socket_connection_get_socket(client_connection);
  if (client_socket == NULL) {
    g_warning ("%s: client disconnected ?", G_STRFUNC);
    return TRUE;
  }
  GSocket *device_socket = g_socket_connection_get_socket(priv->conn);
  if (device_socket == NULL) {
    g_warning ("%s: device disconnected ?", G_STRFUNC);
    return TRUE;
  }

  device_fd = g_socket_get_fd(device_socket);
  client_fd = g_socket_get_fd(client_socket);

  msg.msg_control = cmsg_buf;
  msg.msg_controllen = sizeof (cmsg_buf);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN (sizeof (device_fd));
  *((gint *) CMSG_DATA (cmsg)) = device_fd;

  iov.iov_base = &dummy_byte;
  iov.iov_len = sizeof (dummy_byte);

  ret = sendmsg (client_fd, &msg, MSG_NOSIGNAL);
  if (ret != 1)
    {
      g_warning ("%s: sendmsg returned %zd", G_STRFUNC, ret);
    }

  g_signal_emit (self, signals[DONE], 0);

  return TRUE;
}

void
_synce_connection_broker_take_connection (SynceConnectionBroker *self,
                                          GSocketConnection *conn)
{
  SynceConnectionBrokerPrivate *priv = SYNCE_CONNECTION_BROKER_GET_PRIVATE (self);
  GRand *rnd;
  guint uid = 0;
  GError *error = NULL;
  GSocketAddress *sock_address = NULL;

  g_assert (priv->conn == NULL);

  priv->conn = conn;

  rnd = g_rand_new ();
  priv->filename = g_strdup_printf ("%s/run/synce-%08x%08x%08x%08x.sock", LOCALSTATEDIR,
      g_rand_int (rnd), g_rand_int (rnd), g_rand_int (rnd), g_rand_int (rnd));
  g_rand_free (rnd);
  sock_address = g_unix_socket_address_new(priv->filename);

  priv->server = g_socket_service_new();
  if (!(priv->server)) {
    g_critical("%s: failed to create socket service", G_STRFUNC);
    goto error_exit;
  }

  if (!(g_socket_listener_add_address(G_SOCKET_LISTENER(priv->server), sock_address, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, &error))) {
    g_critical("%s: failed to add address to socket service: %s", G_STRFUNC, error->message);
    goto error_exit;
  }
  g_object_unref(sock_address);

  g_signal_connect(priv->server, "incoming", G_CALLBACK(server_socket_readable_cb), self);

#if USE_GDBUS
  synce_get_dbus_sender_uid(g_dbus_method_invocation_get_sender (priv->ctx), &uid);
#else
  synce_get_dbus_sender_uid(dbus_g_method_get_sender (priv->ctx), &uid);
#endif

  chmod (priv->filename, S_IRUSR | S_IWUSR);
  chown (priv->filename, uid, -1);

  g_socket_service_start(priv->server);

#if USE_GDBUS
  /* we don't need the object for the first argument, it doesn't go anywhere */
  synce_dbus_device_complete_request_connection(NULL, priv->ctx, priv->filename);
#else
  dbus_g_method_return (priv->ctx, priv->filename);
#endif
  priv->ctx = NULL;

  return;

 error_exit:
  if (error) g_error_free(error);
  if (sock_address) g_object_unref(sock_address);

  error = g_error_new (G_FILE_ERROR, G_FILE_ERROR_FAILED,
		       "Failed to create socket to pass connection");
#if USE_GDBUS
  g_dbus_method_invocation_return_gerror(priv->ctx, error);
  g_error_free(error);
#else
  dbus_g_method_return_error (priv->ctx, error);
#endif
  priv->ctx = NULL;
  g_free(priv->filename);
  priv->filename = NULL;

  return;
}
