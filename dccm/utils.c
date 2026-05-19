#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <synce.h>
#if !USE_GDBUS
#include <dbus/dbus-glib.h>
#endif
#include <glib.h>
#include <gio/gio.h>
#include "utils.h"

#if USE_GDBUS
static GDBusProxy *
synce_get_dbus_g_bus_proxy()
{
  static GDBusProxy *bus_proxy = NULL;

  if (bus_proxy == NULL) {
    GError *error = NULL;

    bus_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
					      G_DBUS_PROXY_FLAGS_NONE,
					      NULL,
					      "org.freedesktop.DBus",
					      "/org/freedesktop/DBus",
					      "org.freedesktop.DBus",
					      NULL,
					      &error);
    if (bus_proxy == NULL) {
      g_critical("%s: Failed to get proxy to system bus: %s", G_STRFUNC, error->message);
      g_error_free(error);
      return NULL;
    }
  }
  return bus_proxy;
}
#else
static DBusGProxy *
synce_get_dbus_g_bus_proxy()
{
  static DBusGProxy *bus_proxy = NULL;

  if (bus_proxy == NULL) {
    GError *error = NULL;
    DBusGConnection *bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    if (bus == NULL) {
      g_critical("%s: Failed to connect to system bus: %s", G_STRFUNC, error->message);
      g_error_free(error);
      return NULL;
    }

    bus_proxy = dbus_g_proxy_new_for_name(bus,
					  "org.freedesktop.DBus",
					  "/org/freedesktop/DBus",
					  "org.freedesktop.DBus");
    if (bus_proxy == NULL)
      g_critical("%s: Failed to get proxy to DBus", G_STRFUNC);

    dbus_g_connection_unref(bus);
  }
  return bus_proxy;
}
#endif

void
synce_get_dbus_sender_uid(const gchar *sender, guint *uid)
{
  GError *error = NULL;
#if USE_GDBUS
  GDBusProxy *dbus_proxy = synce_get_dbus_g_bus_proxy();
#else
  DBusGProxy *dbus_proxy = synce_get_dbus_g_bus_proxy();
#endif
  if (!dbus_proxy) {
    *uid = 0;
    return;
  }

#if USE_GDBUS

  GVariant *result =  g_dbus_proxy_call_sync(dbus_proxy,
					     "GetConnectionUnixUser",
					     g_variant_new ("(s)", sender),
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);
  if (!result) {
    g_critical("Failed to get dbus sender uid: %s", error->message);
    *uid = 0;
  } else {
    g_variant_get(result, "(u)", uid);
    g_variant_unref(result);
  }

#else
  if (!dbus_g_proxy_call(dbus_proxy,
			 "GetConnectionUnixUser",
			 &error,
			 G_TYPE_STRING, sender,
			 G_TYPE_INVALID,
			 G_TYPE_UINT, uid,
			 G_TYPE_INVALID))
    {
      g_critical ("Failed to get dbus sender uid: %s", error->message);
      *uid = 0;
    }
#endif
  return;
}

void
synce_print_hexdump (const void *buf, gssize len)
{
  GString *s = g_string_sized_new (12);
  int i;

  for (i = 0; i < len; i++)
    {
      g_string_append_printf (s, "%s%02x",
			      (i > 0) ? " " : "", ((guchar *) buf)[i]);
    }

  g_debug ("%s", s->str);

  g_string_free (s, TRUE);
}

gchar *
synce_guid_to_string (const guchar *p)
{
  guint32 v1;
  guint16 v2, v3;

  v1 = GUINT32_FROM_LE (*((guint32 *) p));
  v2 = GUINT16_FROM_LE (*((guint32 *) (p + 4)));
  v3 = GUINT16_FROM_LE (*((guint32 *) (p + 6)));

  return g_strdup_printf ("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			  v1, v2, v3, p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

gchar *
synce_rapi_unicode_string_to_string (const guchar *buf, const guchar *buf_max, guint max_len, gsize *bytes_consumed)
{
  gchar *ret;
  guint32 len;

  if (buf + 4 > buf_max)
    return NULL;

  len = GUINT32_FROM_LE (*((guint32 *) buf));
  if (buf + 4 + (len * sizeof (WCHAR)) > buf_max || len > max_len)
    return NULL;

  if (len > 0)
    {
      ret = wstr_to_utf8 ((LPCWSTR) (buf + 4));
    }
  else
    {
      ret = g_strdup ("");
    }

  if (bytes_consumed != NULL)
    *bytes_consumed = sizeof (len) + (len * sizeof (WCHAR));

  return ret;
}

gchar *
synce_rapi_ascii_string_to_string (const guchar *buf, const guchar *buf_max, guint max_len, gsize *bytes_consumed)
{
  gchar *ret;
  guint32 len;

  if (buf + 4 > buf_max)
    return NULL;

  len = GUINT32_FROM_LE (*((guint32 *) buf));
  if (buf + 4 + len > buf_max || len > max_len)
    return NULL;

  if (len > 0)
    {
      ret = g_strdup ((const gchar *) (buf + 4));
    }
  else
    {
      ret = g_strdup ("");
    }

  if (bytes_consumed != NULL)
    *bytes_consumed = sizeof (len) + len;

  return ret;
}

gchar *
synce_rapi_unicode_string_to_string_at_offset (const guchar *buf, const guchar *offset, const guchar *buf_max)
{
  gchar *ret;
  guint32 string_offset;

  if (offset + 4 > buf_max)
    return NULL;

  string_offset = GUINT32_FROM_LE (*((guint32 *) offset));
  if ((buf + string_offset) > buf_max)
    return NULL;

  ret = wstr_to_utf8 ((LPCWSTR) (buf + string_offset));

  return ret;
}

gboolean
synce_trigger_connection (const gchar *device_ip)
{
  gboolean result = FALSE;
  gint fd = -1;
  struct sockaddr_in sa;
  guchar b = 0x7f;

  if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
    g_warning ("%s: failed to create socket: %s", G_STRFUNC, strerror (errno));
    goto exit;
  }

  sa.sin_family = AF_INET;
  sa.sin_port = htons (5679);
  inet_aton (device_ip, &sa.sin_addr);

  if (sendto (fd, &b, sizeof (b), 0, (const struct sockaddr *) &sa,
              sizeof (sa)) != 1)
    {
      g_warning ("%s: failed to send on socket: %s", G_STRFUNC, strerror (errno));
      goto exit;
    }

  result = TRUE;

exit:
  if (fd != -1)
    close (fd);

  return result;
}

GSocket *
synce_create_socket(const gchar *address, guint16 port)
{
  GError *error = NULL;
  GSocket *socket = NULL;
  GInetAddress *inet_address = NULL;
  GSocketAddress *sock_address = NULL;

  if (!(inet_address = g_inet_address_new_from_string(address))) {
    g_critical("%s: failed to parse ip address: %s", G_STRFUNC, address);
    goto error_exit;
  }

  socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &error);
  if (!socket) {
    g_critical("%s: failed to create socket: %s", G_STRFUNC, error->message);
    goto error_exit;
  }
  if (!(g_initable_init(G_INITABLE(socket), NULL, &error))) {
    g_critical("%s: failed to initialise socket: %s", G_STRFUNC, error->message);
    g_object_unref(socket);
    goto error_exit;
  }
  sock_address = g_inet_socket_address_new(inet_address, port);
  if (!(g_socket_bind(socket, sock_address, TRUE, &error))) {
    g_critical("%s: failed to bind to port %u: %s", G_STRFUNC, port, error->message);
    g_object_unref(socket);
    goto error_exit;
  }
  if (!(g_socket_listen(socket, &error))) {
    g_critical("%s: failed to listen to port %u: %s", G_STRFUNC, port, error->message);
    g_object_unref(socket);
    goto error_exit;
  }

  g_object_unref(sock_address);
  g_object_unref(inet_address);
  return socket;

 error_exit:
  if (error) g_error_free(error);
  if (inet_address) g_object_unref(inet_address);
  if (sock_address) g_object_unref(sock_address);

  return NULL;
}
