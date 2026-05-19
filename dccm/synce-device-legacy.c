#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <synce.h>
#include <arpa/inet.h>
#include <string.h>
#include <glib-object.h>
#include <gio/gio.h>

#if !USE_GDBUS
#include <dbus/dbus-glib.h>
#endif

#include "synce-device.h"
#include "synce-device-internal.h"
#include "synce-device-legacy.h"
#include "synce-connection-broker.h"
#include "synce-errors.h"
#include "utils.h"

G_DEFINE_TYPE(SynceDeviceLegacy, synce_device_legacy, SYNCE_TYPE_DEVICE)

typedef struct _SynceDeviceLegacyPrivate SynceDeviceLegacyPrivate;
struct _SynceDeviceLegacyPrivate {
  gint ping_count;
  gchar *password;
  GSocketClient *rapi_sock_client;
  GInetSocketAddress *rapi_address;
};

#define SYNCE_DEVICE_LEGACY_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SYNCE_TYPE_DEVICE_LEGACY, SynceDeviceLegacyPrivate))

typedef struct _PendingClientInfo PendingClientInfo;
struct _PendingClientInfo {
  SynceConnectionBroker *broker;
  GSocketConnection *conn;
  gchar *buf;
};

#define DCCM_PORT               5679
#define DCCM_PING               0x12345678
#define DCCM_PING_INTERVAL      5       /* seconds */
#define DCCM_MAX_PING_COUNT     3       /* max number of pings without reply */
#define DCCM_MAX_PACKET_SIZE    512
#define DCCM_MIN_PACKET_SIZE    0x24
#define RAPI_PORT               990


/* methods */

static gboolean
synce_device_legacy_send_ping(gpointer data)
{
  if (!(SYNCE_IS_DEVICE_LEGACY(data)))
    return FALSE;

  SynceDeviceLegacy *self = SYNCE_DEVICE_LEGACY (data);
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));
  SynceDeviceLegacyPrivate *priv_legacy = SYNCE_DEVICE_LEGACY_GET_PRIVATE (self);

  guint32 ping = GUINT32_TO_LE(DCCM_PING);

  GError *error = NULL;
  gsize written;
  GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(priv->conn));
  if (!(g_output_stream_write_all(ostream, (gchar *) &ping, sizeof(ping), &written, NULL, &error))) {
    g_critical("%s: failed to send ping: %s", G_STRFUNC, error->message);
    g_error_free(error);
  }

  /* do a read here ? */
  priv->iobuf = g_malloc(4);
  GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(priv->conn));
  g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);

  if (++priv_legacy->ping_count == DCCM_MAX_PING_COUNT) {
    gchar *name = NULL;

    g_object_get(self, "name", &name, NULL);

    g_warning("%s: Device %s not responded to %d pings, assume disconnected", G_STRFUNC, name, DCCM_MAX_PING_COUNT);
    g_signal_emit (self, SYNCE_DEVICE_GET_CLASS(SYNCE_DEVICE(self))->signals[SYNCE_DEVICE_SIGNAL_DISCONNECTED], 0);
    g_free(name);
    return FALSE;
  }

  return TRUE;
}

static void
synce_device_legacy_info_received (SynceDeviceLegacy *self, const guchar *buf, gssize length)
{
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));

  gchar *guid = NULL, *name = NULL, *platform_name = NULL, *model_name = NULL;
  guint os_major = 0, os_minor = 0, version = 0, cpu_type = 0, cur_partner_id = 0, id = 0;
  const guchar *p = buf, *end_ptr = buf + length;

  priv->state = CTRL_STATE_GOT_INFO;

  g_debug("%s: info buffer length = %zu", G_STRFUNC, length);
  synce_print_hexdump (buf, length);

  /*
   * Parse and set device properties.
   */

  if (p + 24 > end_ptr)
    {
      g_warning ("%s: short read for initial information", G_STRFUNC);
      goto ERROR;
    }

  /* skip first 4 bytes */
  g_debug("%s: offset 0: unknown: guint32 ?: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  p += sizeof(guint32);

  os_major = *((guint8 *) p);
  g_debug("%s: offset 4: os_major: guint8: %d", G_STRFUNC, os_major);
  p += sizeof (guint8);

  os_minor = *((guint8 *) p);
  g_debug("%s: offset 5: os_minor: guint8: %d", G_STRFUNC, os_minor);
  p += sizeof (guint8);

  /* skip build number ? */
  g_debug("%s: offset 6: unknown, build number ?: guint16 ?: %d", G_STRFUNC, GUINT16_FROM_LE (*((guint16 *) p)));
  p += sizeof (guint16);

  cpu_type = GUINT16_FROM_LE (*((guint16 *) p));
  g_debug("%s: offset 8: cpu_type: guint16: %d", G_STRFUNC, cpu_type);
  p += sizeof (guint16);

  /* skip 2 bytes */
  g_debug("%s: offset 10: unknown: guint16 ?: %d", G_STRFUNC, GUINT16_FROM_LE (*((guint16 *) p)));
  p += sizeof (guint16);
  /* skip 4 bytes */
  g_debug("%s: offset 12: unknown: guint32 ?: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  p += sizeof(guint32);

  cur_partner_id = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset 16: cur_partner_id: guint32: %d", G_STRFUNC, cur_partner_id);
  p += sizeof (guint32);

  /* second partner id */
  g_debug("%s: offset 20: second partner id ?: guint32 ?: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  p += sizeof (guint32);

  /* string information */

  g_debug("%s: offset 24: offset to device name: guint32: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  name = synce_rapi_unicode_string_to_string_at_offset (buf, p, end_ptr);
  if (name == NULL)
    {
      g_warning ("%s: DeviceName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }
  p += sizeof (guint32);

  g_debug("%s: offset 28: offset to platform name: guint32: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  platform_name = synce_rapi_unicode_string_to_string_at_offset (buf, p, end_ptr);
  if (platform_name == NULL)
    {
      g_warning ("%s: PlatformName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }
  p += sizeof (guint32);

  g_debug("%s: offset 32: offset to model name: guint32: %d", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)));
  model_name = synce_rapi_unicode_string_to_string_at_offset (buf, p, end_ptr);
  if (model_name == NULL)
    {
      g_warning ("%s: ModelName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }

  /* what are version, id ? */

  /* we dont have guid on pre WM5 devices, so we'll just pass
   * in the whole info buffer to generate one */
  guid = synce_guid_to_string (buf);

  g_object_set (self,
		"guid", guid,
		"os-major", os_major,
		"os-minor", os_minor,
		"name", name,
		"version", version,
		"cpu-type", cpu_type,
		"current-partner-id", cur_partner_id,
		"id", id,
		"platform-name", platform_name,
		"model-name", model_name,
		NULL);

  synce_device_dbus_init(SYNCE_DEVICE(self));

  if (priv->pw_key != 0)
    {
      priv->state = CTRL_STATE_AUTH;
      synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE);
    }
  else
    {
      synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_UNSET);
      g_debug("%s: setting CTRL_STATE_CONNECTED", G_STRFUNC);
      priv->state = CTRL_STATE_CONNECTED;
      g_timeout_add((DCCM_PING_INTERVAL * 1000), synce_device_legacy_send_ping, self);
    }

  goto OUT;

 ERROR:
  /* TODO: do something sensible here */

 OUT:
  g_free (guid);
  g_free (name);
  g_free (platform_name);
  g_free (model_name);
}

static void
synce_device_legacy_conn_event_cb_impl(GObject *source_object,
				       GAsyncResult *res,
				       gpointer user_data)
{
  GInputStream *istream = G_INPUT_STREAM(source_object);
  SynceDeviceLegacy *self = SYNCE_DEVICE_LEGACY (user_data);
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));
  SynceDeviceLegacyPrivate *priv_legacy = SYNCE_DEVICE_LEGACY_GET_PRIVATE (self);

  GError *error = NULL;
  gssize num_read = g_input_stream_read_finish(istream, res, &error);

  if (priv->state < CTRL_STATE_GETTING_INFO)
    {
      /* CTRL_STATE_HANDSHAKE */
      guint32 header;

      if (num_read != 4)
	{
	  g_warning ("%s: unexpected length", G_STRFUNC);
	  g_free(priv->iobuf);
	  return;
	}

      header = GUINT32_FROM_LE (*((guint32 *) priv->iobuf));
      g_free(priv->iobuf);

      if (header == 0) {
	/* empty packet header */
	priv->iobuf = g_malloc(4);
	g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
	return;
      } else if (header < DCCM_MAX_PACKET_SIZE) {
	if (header < DCCM_MIN_PACKET_SIZE) {
	  g_warning ("%s: undersize packet", G_STRFUNC);
	  return;
	}
	/* info message */
	priv->state = CTRL_STATE_GETTING_INFO;
	priv->info_buf_size = header;
	priv->iobuf = g_malloc(priv->info_buf_size);
	g_input_stream_read_async(istream, priv->iobuf, priv->info_buf_size, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
      } else {
	/* password challenge */
	priv->pw_key = header & 0xff;
	/*
	  synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE);
	*/
	priv->iobuf = g_malloc(4);
	g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
      }
    }
  else if (priv->state < CTRL_STATE_AUTH)
    {
      /* CTRL_STATE_INFO */
      if (num_read != priv->info_buf_size)
	{
	  g_warning("%s: length read=%zd != info_buf_size=%d",
		    G_STRFUNC, num_read, priv->info_buf_size);
	  return;
	}

      gchar *info_buf = g_malloc(num_read);
      memcpy(info_buf, priv->iobuf, num_read);
      g_free(priv->iobuf);
      synce_device_legacy_info_received(self, (guchar *) info_buf,
					num_read);
      g_free(info_buf);
    }
  else if (priv->state == CTRL_STATE_AUTH)
    {
      /* CTRL_STATE_AUTH */
      guint16 result;

      if (num_read != sizeof (guint16))
	{
	  g_warning ("%s: read length != 2", G_STRFUNC);
	  return;
	}

      result = GUINT16_FROM_LE (*((guint16 *) priv->iobuf));
      g_free(priv->iobuf);

      if (result != 0)
	{
	  priv->state = CTRL_STATE_CONNECTED;
	  synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_UNLOCKED);
	  g_timeout_add((DCCM_PING_INTERVAL * 1000), synce_device_legacy_send_ping, self);
	}
      else
	{
	  g_free(priv_legacy->password);
	  synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE);
	}

#if USE_GDBUS
      g_dbus_method_invocation_return_value (priv->pw_ctx, g_variant_new ("(b)", result != 0));
#else
      dbus_g_method_return (priv->pw_ctx, result != 0);
#endif

      priv->pw_ctx = NULL;
    }
  else if (priv->state == CTRL_STATE_CONNECTED)
    {
      /* CTRL_STATE_CONNECTED */
      guint32 req;

      if (num_read != 4)
	{
	  g_warning ("%s: unexpected length", G_STRFUNC);
	  return;
	}
      req = GUINT32_FROM_LE (*((guint32 *) priv->iobuf));
      g_free(priv->iobuf);

      if (req == 0) {
	/* empty packet header */
	priv->iobuf = g_malloc(4);
	g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
	return;
      }
      if (req == DCCM_PING) {
	/* received a ping */

	priv_legacy->ping_count = 0;
      } else {
	g_warning("%s: Received header, not a ping reply: %d", G_STRFUNC, req);
      }
    }
}

static void
synce_device_legacy_client_event_cb(GObject *source_object,
				    GAsyncResult *res,
				    gpointer user_data)
{
  GSocketClient *sock_client = G_SOCKET_CLIENT(source_object);
  GError *error = NULL;
  GSocketConnection *conn = g_socket_client_connect_finish(sock_client, res, &error);
  if (!conn) {
    g_warning("%s: failed to obtain client RAPI connection: %s", G_STRFUNC, error->message);
    g_error_free(error);
    return;
  }

  if (SYNCE_IS_CONNECTION_BROKER(user_data)) {
    _synce_connection_broker_take_connection (SYNCE_CONNECTION_BROKER(user_data), conn);
  } else {
    g_warning("%s: not passed a connection broker", G_STRFUNC);
  }
  return;
}


static void
synce_device_legacy_client_event_password_cb(GObject *source_object,
					     GAsyncResult *res,
					     gpointer user_data)
{
  GInputStream *istream = G_INPUT_STREAM(source_object);

  PendingClientInfo *info = (PendingClientInfo*)user_data;

  GSocketConnection *conn = NULL;
  SynceConnectionBroker *broker = NULL;
  guint8 result = 0;
#if USE_GDBUS
  GDBusMethodInvocation *ctx;
#else
  DBusGMethodInvocation *ctx;
#endif

  GError *error = NULL;
  gssize num_read = g_input_stream_read_finish(istream, res, &error);

  conn = info->conn;
  broker = SYNCE_CONNECTION_BROKER(info->broker);
  result = *((guint8*)info->buf);
  g_object_get(broker, "context", &ctx, NULL);

  g_free(info->buf);
  g_free(info);

  if (num_read == -1) {
    g_warning("%s: Failed to get password reply from device: %s", G_STRFUNC, error->message);
    g_error_free(error);
    g_object_unref(conn);
    g_object_unref(broker);
    error = g_error_new (SYNCE_ERRORS, NotAvailable,
			 "Failed to authenticate connection");
    goto OUT;
  }

  if (num_read != sizeof (guint8)) {
    g_warning ("%s: Failed to get password reply from device, read length != 1", G_STRFUNC);
    g_object_unref(conn);
    g_object_unref(broker);
    error = g_error_new (SYNCE_ERRORS, NotAvailable,
			 "Failed to authenticate connection");
    goto OUT;
  }


  if (result != 0) {
    _synce_connection_broker_take_connection (broker, conn);

    return;
  } else {
    g_warning("%s: Password rejected", G_STRFUNC);
    g_object_unref(conn);
    g_object_unref(broker);
    error = g_error_new (SYNCE_ERRORS, NotAvailable,
                         "Failed to authenticate connection");
  }
 OUT:
  if (error != NULL)
#if USE_GDBUS
    {
      g_dbus_method_invocation_return_gerror(ctx, error);
      g_error_free(error);
    }
  return;
}
#else
    dbus_g_method_return_error (ctx, error);
  return;
}
#endif

#if USE_GDBUS
gboolean
synce_device_legacy_request_connection_impl (SynceDbusDevice *interface, GDBusMethodInvocation *ctx, gpointer userdata)
{
  SynceDeviceLegacy *self = SYNCE_DEVICE_LEGACY (userdata);
#else
void
synce_device_legacy_request_connection_impl (SynceDevice *self, DBusGMethodInvocation *ctx)
{
#endif
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (self);
  GError *error = NULL;
  SynceConnectionBroker *broker;
  GSocketConnection *rapi_conn = NULL;
  guchar *buf = NULL;
  gsize buf_size = 0;

  if (priv->state != CTRL_STATE_CONNECTED)
    {
      error = g_error_new(SYNCE_ERRORS, NotAvailable,
			  (priv->pw_flags & SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE) ?
			  "Not authenticated, you need to call ProvidePassword with the "
			  "correct password." : "Not yet connected.");
      goto OUT;
    }

  SynceDeviceLegacyPrivate *legacy_priv = SYNCE_DEVICE_LEGACY_GET_PRIVATE (SYNCE_DEVICE_LEGACY(self));

  if (!(legacy_priv->rapi_sock_client)) {
    legacy_priv->rapi_sock_client = g_socket_client_new();
    GInetAddress *address = g_inet_address_new_from_string(priv->ip_address);
    legacy_priv->rapi_address = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new(address, RAPI_PORT));
    g_object_unref(address);
  }

  /* 
   * Create a local copy of the global req_id variable to avoid
   * the chances of race conditions
   */
  guint *req_id_local = (guint *) g_malloc (sizeof (guint));
  *req_id_local = ++(priv->req_id) ;

  broker = g_object_new (SYNCE_TYPE_CONNECTION_BROKER,
                         "id", *req_id_local,
                         "context", ctx,
                         NULL);

  g_hash_table_insert (priv->requests, req_id_local, broker);

  g_signal_connect (broker, "done", (GCallback) synce_device_conn_broker_done_cb, self);

  if (legacy_priv->password && strlen(legacy_priv->password))
    {
      synce_password_encode(legacy_priv->password, priv->pw_key, &buf, &buf_size);

      buf_size = GUINT16_TO_LE (buf_size);

      rapi_conn = g_socket_client_connect(legacy_priv->rapi_sock_client, G_SOCKET_CONNECTABLE(legacy_priv->rapi_address), NULL, &error);
      if (!rapi_conn) {
	g_warning("%s: failed to obtain client RAPI connection: %s", G_STRFUNC, error->message);
	goto OUT;
      }

      gsize written = 0;
      GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(rapi_conn));
      if (!(g_output_stream_write_all(ostream, (gchar *) &buf_size, sizeof(buf_size), &written, NULL, &error))) {
	g_critical("%s: failed to send password header: %s", G_STRFUNC, error->message);
	g_object_unref(rapi_conn);
	goto OUT;
      }
      if (!(g_output_stream_write_all(ostream, (gchar *) buf, buf_size, &written, NULL, &error))) {
	g_critical("%s: failed to send password: %s", G_STRFUNC, error->message);
	g_object_unref(rapi_conn);
	goto OUT;
      }

      PendingClientInfo *info = g_new0(PendingClientInfo, 1);
      info->broker = broker;
      info->buf = g_malloc(sizeof(guint8));
      info->conn = rapi_conn;
      GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(rapi_conn));
      g_input_stream_read_async(istream, info->buf, sizeof(guint8), G_PRIORITY_DEFAULT, NULL, synce_device_legacy_client_event_password_cb, info);

      goto OUT;
    }

  g_socket_client_connect_async(legacy_priv->rapi_sock_client, G_SOCKET_CONNECTABLE(legacy_priv->rapi_address), NULL, synce_device_legacy_client_event_cb, broker);

 OUT:
  if (buf)
    wstr_free_string(buf);
  if (error != NULL) {
    if (broker)
      g_hash_table_remove (priv->requests, req_id_local);

#if USE_GDBUS
    g_dbus_method_invocation_return_gerror(ctx, error);
    g_error_free(error);
  }
  return TRUE;
#else /* USE_GDBUS */
    dbus_g_method_return_error (ctx, error);
  }
  return;
#endif
}

#if USE_GDBUS
static gboolean
synce_device_legacy_provide_password_impl(SynceDbusDevice *interface,
					  GDBusMethodInvocation *ctx,
					  const gchar *password,
					  gpointer userdata)
{
  SynceDevice *self = SYNCE_DEVICE (userdata);
#else /* USE_GDBUS */
static void
synce_device_legacy_provide_password_impl(SynceDevice *self,
					  const gchar *password,
					  DBusGMethodInvocation *ctx)
{
#endif
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (self);
  GError *error = NULL;

  if (priv->state != CTRL_STATE_AUTH ||
      (priv->pw_flags & SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE) == 0)
    {
      error = g_error_new (SYNCE_ERRORS, NotAvailable,
			   "No password expected in the current state.");
      goto OUT;
    }
  else if (priv->pw_ctx != NULL)
    {
      error = g_error_new (SYNCE_ERRORS, NotAvailable,
			   "An authentication attempt is still in progress.");
      goto OUT;
    }

  SynceDeviceLegacyPrivate *priv_legacy = SYNCE_DEVICE_LEGACY_GET_PRIVATE (SYNCE_DEVICE_LEGACY(self));
  priv_legacy->password = g_strdup(password);

#if USE_GDBUS
  (SYNCE_DEVICE_CLASS(synce_device_legacy_parent_class)->synce_device_provide_password) (interface, ctx, password, SYNCE_DEVICE(self));
#else
  (SYNCE_DEVICE_CLASS(synce_device_legacy_parent_class)->synce_device_provide_password) (SYNCE_DEVICE(self), password, ctx);
#endif

 OUT:
  if (error != NULL)
#if USE_GDBUS
    {
      g_dbus_method_invocation_return_gerror(ctx, error);
      g_error_free(error);
    }
  return TRUE;
#else
    dbus_g_method_return_error (ctx, error);
  return;
#endif
}

/* class & instance functions */

static void
synce_device_legacy_init(SynceDeviceLegacy *self)
{
  SynceDeviceLegacyPrivate *priv = SYNCE_DEVICE_LEGACY_GET_PRIVATE (self);

  priv->ping_count = 0;
  priv->password = NULL;
  priv->rapi_sock_client = NULL;
  priv->rapi_address = NULL;
}

static void
synce_device_legacy_dispose (GObject *obj)
{
  SynceDeviceLegacy *self = SYNCE_DEVICE_LEGACY(obj);
  SynceDevicePrivate *parent_priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));
  SynceDeviceLegacyPrivate *priv_legacy = SYNCE_DEVICE_LEGACY_GET_PRIVATE (SYNCE_DEVICE_LEGACY(self));

  if (parent_priv->dispose_has_run) {
    return;
  }

  /* unref other objects */

  g_object_unref(priv_legacy->rapi_sock_client);
  g_object_unref(priv_legacy->rapi_address);

  if (G_OBJECT_CLASS (synce_device_legacy_parent_class)->dispose)
    G_OBJECT_CLASS (synce_device_legacy_parent_class)->dispose (obj);
}

static void
synce_device_legacy_finalize (GObject *obj)
{
  SynceDeviceLegacy *self = SYNCE_DEVICE_LEGACY(obj);
  SynceDeviceLegacyPrivate *priv = SYNCE_DEVICE_LEGACY_GET_PRIVATE (self);

  g_free(priv->password);

  if (G_OBJECT_CLASS (synce_device_legacy_parent_class)->finalize)
    G_OBJECT_CLASS (synce_device_legacy_parent_class)->finalize (obj);
}

static void
synce_device_legacy_class_init (SynceDeviceLegacyClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  SynceDeviceClass *synce_device_class = SYNCE_DEVICE_CLASS(klass);

  gobject_class->dispose = synce_device_legacy_dispose;
  gobject_class->finalize = synce_device_legacy_finalize;

  g_type_class_add_private (klass, sizeof (SynceDeviceLegacyPrivate));

  synce_device_class->synce_device_conn_event_cb = synce_device_legacy_conn_event_cb_impl;
  synce_device_class->synce_device_request_connection = synce_device_legacy_request_connection_impl;
  synce_device_class->synce_device_provide_password = synce_device_legacy_provide_password_impl;
}


