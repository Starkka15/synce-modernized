#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glib-object.h>
#include <gio/gio.h>
#if !USE_GDBUS
#include <dbus/dbus-glib.h>
#endif
#include <synce.h>

#include "synce-device.h"
#include "synce-device-internal.h"
#include "synce-device-rndis.h"
#include "synce-connection-broker.h"
#include "synce-errors.h"
#include "utils.h"

G_DEFINE_TYPE(SynceDeviceRndis, synce_device_rndis, SYNCE_TYPE_DEVICE)

typedef struct _SynceDeviceRndisPrivate SynceDeviceRndisPrivate;
struct _SynceDeviceRndisPrivate {
  GHashTable *pending_client_conns;
};

#define SYNCE_DEVICE_RNDIS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SYNCE_TYPE_DEVICE_RNDIS, SynceDeviceRndisPrivate))

/* methods */


static void
synce_device_rndis_info_received(SynceDeviceRndis *self, const guchar *buf, gssize length)
{
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));
  gchar *guid = NULL, *name = NULL, *platform_name = NULL, *model_name = NULL;
  guint os_major, os_minor, version, cpu_type, cur_partner_id, id, comp_count;
  const guchar *p = buf, *end_ptr = buf + length;
  gsize consumed;
  gsize debug_offset;

  priv->state = CTRL_STATE_GOT_INFO;

  g_debug("%s: info buffer length = %zu", G_STRFUNC, length);
  synce_print_hexdump (buf, length);

  /*
   * Parse and set device properties.
   */
  if (p + 24 > end_ptr)
    {
      g_warning ("%s: short read trying to read GUID/OsMajor/OsMinor", G_STRFUNC);
      goto ERROR;
    }

  guid = synce_guid_to_string (p);
  g_debug("%s: offset 0: guid: 16 bytes : %s", G_STRFUNC, guid);
  p += 16;

  os_major = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset 16: os_major: guint32: %d", G_STRFUNC, os_major);
  p += sizeof (guint32);

  os_minor = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset 20: os_minor: guint32: %d", G_STRFUNC, os_minor);
  p += sizeof (guint32);

  name = synce_rapi_unicode_string_to_string (p, end_ptr, 31, &consumed);
  if (name == NULL)
    {
      g_warning ("%s: DeviceName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }
  g_debug("%s: offset 24: device name: guint32 length and string: %d, %s", G_STRFUNC, GUINT32_FROM_LE (*((guint32 *) p)), name);
  p += consumed + sizeof (WCHAR);
  debug_offset = 24 + consumed + sizeof(WCHAR);

  if (p + 20 > end_ptr)
    {
      g_warning ("%s: short read trying to read Version/CpuType/"
		 "Flags/CurPartnerId/Id", G_STRFUNC);
      goto ERROR;
    }

  version = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: version: guint32: %d", G_STRFUNC, debug_offset, version);
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  cpu_type = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: cpu_type: guint32: %d", G_STRFUNC, debug_offset, cpu_type);
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  /* counter or flags? */
  g_debug("%s: offset %zu: unknown, counter or flags ?: guint32: %d", G_STRFUNC, debug_offset, GUINT32_FROM_LE (*((guint32 *) p)));
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  cur_partner_id = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: cur_partner_id: guint32: %d", G_STRFUNC, debug_offset, cur_partner_id);
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  id = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: id: guint32: %d", G_STRFUNC, debug_offset, id);
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  /* TODO: PlatformName is actually a list of strings,
   *       terminated with an extra NUL byte */
  platform_name = synce_rapi_ascii_string_to_string (p, end_ptr, 255,
						      &consumed);
  if (platform_name == NULL)
    {
      g_warning ("%s: PlatformName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }
  g_debug("%s: offset %zu: platform_name: guint32 length and string: %d, %s", G_STRFUNC, debug_offset, GUINT32_FROM_LE (*((guint32 *) p)), platform_name);
  p += consumed;
  debug_offset += consumed;

  model_name = synce_rapi_ascii_string_to_string (p, end_ptr, 255, &consumed);
  if (model_name == NULL)
    {
      g_warning ("%s: ModelName is out of bounds or too long", G_STRFUNC);
      goto ERROR;
    }
  g_debug("%s: offset %zu: model_name: guint32 length and string: %d, %s", G_STRFUNC, debug_offset, GUINT32_FROM_LE (*((guint32 *) p)), model_name);
  p += consumed + 1;
  debug_offset += consumed + 1;

  /* TODO: parse the platform component versions,
   *       for now we just ignore them */
  if (p + 4 > end_ptr)
    {
      g_warning ("%s: short read trying to read ComponentCount", G_STRFUNC);
      goto ERROR;
    }
  comp_count = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: component count: guint32: %d", G_STRFUNC, debug_offset, comp_count);
  if (comp_count > 6)
    {
      g_warning ("%s: ComponentCount %d is out of range", G_STRFUNC, comp_count);
      goto ERROR;
    }
  p += sizeof (guint32) + (comp_count * 8);
  debug_offset += sizeof (guint32) + (comp_count * 8);

  if (p + 4 > end_ptr)
    {
      g_warning ("%s: short read trying to read Components/PasswordKey",
		 G_STRFUNC);
      goto ERROR;
    }
  priv->pw_key = GUINT32_FROM_LE (*((guint32 *) p));
  g_debug("%s: offset %zu: password key: guint32: %d", G_STRFUNC, debug_offset, priv->pw_key);
  p += sizeof (guint32);
  debug_offset += sizeof (guint32);

  if (p < buf + length)
    {
      guint n;

      if (p + 4 > end_ptr)
        {
          g_warning ("%s: short read trying to read ExtraData size", G_STRFUNC);
          goto ERROR;
        }
      n = GUINT32_FROM_LE (*((guint32 *) p));
      g_debug("%s: offset %zu: extra data size: guint32: %d", G_STRFUNC, debug_offset, n);
      if (p + n > end_ptr)
        {
          g_warning ("%s: short read trying to read ExtraData data", G_STRFUNC);
          goto ERROR;
        }

      g_debug ("extradata:");
      synce_print_hexdump (p, n);

      p += n;
    }
  else
    g_debug("%s: offset %zu: no ExtraData", G_STRFUNC, debug_offset);

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
      if (priv->pw_key != 0xffffffff)
        {
	  /* WM5 */
          priv->state = CTRL_STATE_AUTH;
          synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE);
        }
      else
        {
          /* WM6 */
          priv->state = CTRL_STATE_AUTH ;

          guint32 requestShowUnlockScreen ;

          requestShowUnlockScreen = 8 ;

          requestShowUnlockScreen = GUINT32_TO_LE(requestShowUnlockScreen) ;

	  gsize written = 0;
	  GError *error = NULL;
	  GOutputStream *out_stream = g_io_stream_get_output_stream(G_IO_STREAM(priv->conn));
	  if (!(g_output_stream_write_all(out_stream, (gchar *) &requestShowUnlockScreen, sizeof (requestShowUnlockScreen), &written, NULL, &error))) {
	    g_critical("%s: failed to write control message to display unlock screen: %s", G_STRFUNC, error->message);
	    g_error_free(error);
	  }

	  GInputStream *in_stream = g_io_stream_get_input_stream(G_IO_STREAM(priv->conn));
	  priv->iobuf = g_malloc(sizeof(guint32));
	  g_input_stream_read_async(in_stream, priv->iobuf, sizeof(guint32), G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);

          /*
           * The flag should be that the password is set AND that the device
           * is waiting to be unlocked, on the device itself
           */
          synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE_ON_DEVICE);
        }
    }
  else
    {
      synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_UNSET);
      priv->state = CTRL_STATE_CONNECTED;
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
synce_device_rndis_conn_event_cb_impl(GObject *source_object,
				      GAsyncResult *res,
				      gpointer user_data)
{
  GInputStream *istream = G_INPUT_STREAM(source_object);
  SynceDeviceRndis *self = SYNCE_DEVICE_RNDIS (user_data);
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));

  GError *error = NULL;
  gssize num_read = g_input_stream_read_finish(istream, res, &error);

  if (priv->state == CTRL_STATE_HANDSHAKE)
    {
      guint32 req, v;
      GArray *resp;
      guint i;
      gsize len;
      gchar *buf;

      if (num_read != 4)
	{
	  g_warning ("%s: unexpected length", G_STRFUNC);
	  g_free(priv->iobuf);
	  return;
	}

      req = GUINT32_FROM_LE (*((guint32 *) priv->iobuf));
      g_free(priv->iobuf);
      resp = g_array_sized_new (FALSE, FALSE, sizeof (guint32), 1);

      switch (req)
	{
	case 0:
	  v = 3; g_array_append_val (resp, v);
	  break;
	case 4:
	  priv->state = CTRL_STATE_GETTING_INFO;
	  priv->iobuf = g_malloc(4);
	  g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
	  break;
	case 6:
	  v = 7; g_array_append_val (resp, v);
	  v = 8; g_array_append_val (resp, v);
	  v = 4; g_array_append_val (resp, v);
	  v = 5; g_array_append_val (resp, v);
	  break;
	default:
	  g_warning ("%s: unknown request 0x%08x", G_STRFUNC, req);
	  break;
	}

      if (resp->len > 0)
	{
	  len = resp->len * sizeof (guint32);
	  buf = g_new (gchar, len);

	  for (i = 0; i < resp->len; i++)
	    {
	      v = g_array_index (resp, guint32, i);
	      *((guint32 *)(buf + (i * sizeof (guint32)))) = GUINT32_TO_LE (v);
	    }

	  gsize written = 0;
	  GOutputStream *out_stream = g_io_stream_get_output_stream(G_IO_STREAM(priv->conn));
	  if (!(g_output_stream_write_all(out_stream, buf, len, &written, NULL, &error))) {
	    g_critical("%s: failed to write control message to device: %s", G_STRFUNC, error->message);
	    g_error_free(error);
	  }
	  priv->iobuf = g_malloc(4);
	  g_input_stream_read_async(istream, priv->iobuf, 4, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);

	  g_free (buf);
	}

      g_array_free (resp, TRUE);
    }
  else if (priv->state < CTRL_STATE_AUTH)
    {
      if (priv->info_buf_size == -1)
	{
	  if (num_read != 4)
	    {
	      g_warning ("%s: event->length != 4", G_STRFUNC);
	      g_free(priv->iobuf);
	      return;
	    }

	  priv->info_buf_size = GUINT32_FROM_LE (*((guint32 *) priv->iobuf));
	  g_free(priv->iobuf);

	  priv->iobuf = g_malloc(priv->info_buf_size);
	  g_input_stream_read_async(istream, priv->iobuf, priv->info_buf_size, G_PRIORITY_DEFAULT, NULL, synce_device_conn_event_cb, self);
	}
      else
	{
	  if (num_read != priv->info_buf_size)
	    {
	      g_warning ("%s: length read=%zd != info_buf_size=%d",
			 G_STRFUNC, num_read, priv->info_buf_size);
	      g_free(priv->iobuf);
	      return;
	    }

	  gchar *info_buf = g_malloc0(num_read);
	  memcpy(info_buf, priv->iobuf, num_read);
	  g_free(priv->iobuf);
	  synce_device_rndis_info_received(self, (guchar *) info_buf,
					   num_read);
	  g_free(info_buf);
	}
    }
  else if (priv->state == CTRL_STATE_AUTH)
    {
      if (priv->pw_key != 0xffffffff) {
	/* wm5 - password sent to device */

	guint16 result;

	if (num_read != sizeof (guint16))
	  {
	    g_warning ("%s: length read != 2", G_STRFUNC);
	    g_free(priv->iobuf);
	    return;
	  }

	result = GUINT16_FROM_LE (*((guint16 *) priv->iobuf));
	g_free(priv->iobuf);

	if (result != 0)
	  {
	    priv->state = CTRL_STATE_CONNECTED;
	    synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_UNLOCKED);
	  }
	else
	  {
	    synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE);
	  }
#if USE_GDBUS
	g_dbus_method_invocation_return_value (priv->pw_ctx, g_variant_new ("(b)", result != 0));
#else
	dbus_g_method_return (priv->pw_ctx, result != 0);
#endif
	priv->pw_ctx = NULL;

      } else {
	/* wm6 - password entered on device */

	guint32 result;
	result = GUINT32_FROM_LE (*((guint32 *) priv->iobuf));
	g_free(priv->iobuf);
	if (result == 0)
	  {
	    priv->state = CTRL_STATE_CONNECTED;
	    synce_device_change_password_flags (SYNCE_DEVICE(self), SYNCE_DEVICE_PASSWORD_FLAG_UNLOCKED);

	    guint32 extraDataForPhone ;
	    /*
	     * This response looks like a confirmation that needs to
	     * be sent to the phone
	     */
	    extraDataForPhone = 0 ;

	    extraDataForPhone = GUINT32_TO_LE(extraDataForPhone) ;

	    gsize written = 0;
	    GOutputStream *out_stream = g_io_stream_get_output_stream(G_IO_STREAM(priv->conn));
	    if (!(g_output_stream_write_all(out_stream, (gchar *) &extraDataForPhone, sizeof (extraDataForPhone), &written, NULL, &error))) {
	      g_critical("%s: failed to write first password response to device: %s", G_STRFUNC, error->message);
	      g_error_free(error);
	    }

	    /*
	     * I don't know what this response is for. Wiredumps showed
	     * ActiveSync sending this also. If you don't send this value
	     * you can briefly start a rapi session, and after short
	     * period of time the odccm process starts using 100% CPU.
	     */
	    extraDataForPhone = 0xc ;
	    extraDataForPhone = GUINT32_TO_LE(extraDataForPhone) ;
	    written = 0;
	    if (!(g_output_stream_write_all(out_stream, (gchar *) &extraDataForPhone, sizeof (extraDataForPhone), &written, NULL, &error))) {
	      g_critical("%s: failed to write second password response to device: %s", G_STRFUNC, error->message);
	      g_error_free(error);
	    }
	  }
	else
	  {
	    g_warning("Don't understand the client response after unlocking device!") ;
	  }

      }
    }
}

#if USE_GDBUS
gboolean
synce_device_rndis_request_connection_impl (SynceDbusDevice *interface, GDBusMethodInvocation *ctx, gpointer userdata)
{
  SynceDeviceRndis *self = SYNCE_DEVICE_RNDIS (userdata);
#else
void
synce_device_rndis_request_connection_impl (SynceDevice *self, DBusGMethodInvocation *ctx)
{
#endif
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE(self);
  GError *error = NULL;
  SynceConnectionBroker *broker;
  guint32 buf[3];

  if (priv->state != CTRL_STATE_CONNECTED)
    {
      error = g_error_new (SYNCE_ERRORS, NotAvailable,
			   (priv->pw_flags & SYNCE_DEVICE_PASSWORD_FLAG_PROVIDE) ?
          "Not authenticated, you need to call ProvidePassword with the "
			   "correct password." : "Not yet connected.");
      goto OUT;
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

  /* FIXME: have SynceConnectionBroker emit a signal when the request has
   *        timed out so that we don't risk zombie requests hanging around. */
  g_hash_table_insert (priv->requests, req_id_local, broker);

  g_signal_connect (broker, "done", (GCallback) synce_device_conn_broker_done_cb, self);

  buf[0] = GUINT32_TO_LE (5);
  buf[1] = GUINT32_TO_LE (4);
  buf[2] = GUINT32_TO_LE (*req_id_local);

  gsize written = 0;
  GOutputStream *out_stream = g_io_stream_get_output_stream(G_IO_STREAM(priv->conn));
  if (!(g_output_stream_write_all(out_stream, (gchar *) buf, sizeof(buf), &written, NULL, &error))) {
    g_critical("%s: failed to write out request for RAPI connection: %s", G_STRFUNC, error->message);
  }

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

static void
synce_device_rndis_client_event_cb(GObject *source_object,
				   GAsyncResult *res,
				   gpointer user_data)
{
  GInputStream *istream = G_INPUT_STREAM(source_object);
  SynceDeviceRndis *self = SYNCE_DEVICE_RNDIS (user_data);
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE (SYNCE_DEVICE(self));
  SynceDeviceRndisPrivate *priv_rndis = SYNCE_DEVICE_RNDIS_GET_PRIVATE (self);
  GSocketConnection *conn = NULL;

  GError *error = NULL;
  gssize num_read = g_input_stream_read_finish(istream, res, &error);
  if (num_read == -1) {
    g_warning("%s: failed to read connection ID from device: %s", G_STRFUNC, error->message);
    g_error_free(error);
    return;
  }

  if (num_read != sizeof (guint32)) {
    g_warning("%s: failed to read full connection ID from device", G_STRFUNC);
    return;
  }


  GList *keys = g_hash_table_get_keys(priv_rndis->pending_client_conns);
  GList *key = keys;
  while (key) {
    GSocketConnection *stored_conn = G_SOCKET_CONNECTION(key->data);
    GInputStream *stored_istream = g_io_stream_get_input_stream(G_IO_STREAM(stored_conn));
    if (stored_istream == istream) {
      conn = stored_conn;
      break;
    }
    key = g_list_next(key);
  }
  g_list_free(keys);

  if (!conn) {
    g_warning("%s: data receieved from unexpected connection", G_STRFUNC);
    return;
  }

  guint32 *tmp_id = (guint32 *)g_hash_table_lookup(priv_rndis->pending_client_conns, conn);
  guint32 id = GUINT32_FROM_LE (*(tmp_id));
  conn = g_object_ref(conn);
  g_hash_table_remove(priv_rndis->pending_client_conns, conn);

  SynceConnectionBroker *broker = g_hash_table_lookup (priv->requests, &id);

  if (broker != NULL)
    {
      _synce_connection_broker_take_connection (broker, conn);

      return;
    }

  g_warning ("%s: unhandled event", G_STRFUNC);
}

void
synce_device_rndis_client_connected (SynceDeviceRndis *self, GSocketConnection *conn)
{
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE(self);
  SynceDeviceRndisPrivate *priv_rndis = SYNCE_DEVICE_RNDIS_GET_PRIVATE (self);

  g_object_ref(conn);
  GInputStream *in_stream = g_io_stream_get_input_stream(G_IO_STREAM(conn));
  guint *tmp_id = g_malloc(sizeof (guint32));
  g_hash_table_insert(priv_rndis->pending_client_conns, conn, tmp_id);
  g_input_stream_read_async(in_stream, tmp_id, sizeof (guint32), G_PRIORITY_DEFAULT, NULL, synce_device_rndis_client_event_cb, self);
}


/* class & instance functions */

static void
synce_device_rndis_init(SynceDeviceRndis *self)
{
  SynceDeviceRndisPrivate *priv_rndis = SYNCE_DEVICE_RNDIS_GET_PRIVATE (self);
  priv_rndis->pending_client_conns = g_hash_table_new_full(g_direct_hash, g_direct_equal, g_object_unref, g_free);
}

static void
synce_device_rndis_dispose (GObject *obj)
{
  SynceDeviceRndis *self = SYNCE_DEVICE_RNDIS(obj);
  SynceDevicePrivate *priv = SYNCE_DEVICE_GET_PRIVATE(self);
  SynceDeviceRndisPrivate *priv_rndis = SYNCE_DEVICE_RNDIS_GET_PRIVATE (self);

  if (priv->dispose_has_run) {
    return;
  }

  g_hash_table_destroy(priv_rndis->pending_client_conns);

  /* unref other objects */

  if (G_OBJECT_CLASS (synce_device_rndis_parent_class)->dispose)
    G_OBJECT_CLASS (synce_device_rndis_parent_class)->dispose (obj);
}

static void
synce_device_rndis_finalize (GObject *obj)
{
  if (G_OBJECT_CLASS (synce_device_rndis_parent_class)->finalize)
    G_OBJECT_CLASS (synce_device_rndis_parent_class)->finalize (obj);
}

static void
synce_device_rndis_class_init (SynceDeviceRndisClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  SynceDeviceClass *synce_device_class = SYNCE_DEVICE_CLASS(klass);

  g_type_class_add_private (klass, sizeof (SynceDeviceRndisPrivate));

  gobject_class->dispose = synce_device_rndis_dispose;
  gobject_class->finalize = synce_device_rndis_finalize;

  synce_device_class->synce_device_conn_event_cb = synce_device_rndis_conn_event_cb_impl;
  synce_device_class->synce_device_request_connection = synce_device_rndis_request_connection_impl;
}


