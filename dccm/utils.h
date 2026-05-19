#include <glib.h>
#include <gio/gio.h>

void
synce_get_dbus_sender_uid (const gchar *sender, guint *uid);

void
synce_print_hexdump (const void *buf, gssize len);

gchar *
synce_guid_to_string (const guchar *p);

gchar *
synce_rapi_unicode_string_to_string (const guchar *buf, const guchar *buf_max, guint max_len, gsize *bytes_consumed);

gchar *
synce_rapi_ascii_string_to_string (const guchar *buf, const guchar *buf_max, guint max_len, gsize *bytes_consumed);

gchar *
synce_rapi_unicode_string_to_string_at_offset (const guchar *buf, const guchar *offset, const guchar *buf_max);

gboolean
synce_trigger_connection (const gchar *device_ip);

GSocket *
synce_create_socket(const gchar *address, guint16 port);
