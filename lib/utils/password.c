/* $Id: password.c 4132 2012-11-20 20:22:43Z mark_ellis $ */
#include "synce.h"
#include "synce_log.h"
#include "synce_socket.h"
#include <string.h>

/**
 * Free an encoded password returned from synce_password_encode()
 */
static void synce_password_free(unsigned char *encodedPassword)
{
	wstr_free_string(encodedPassword);
}

/**
 * Encode a password with a key
 */
bool synce_password_encode(
		const char *asciiPassword,
		unsigned char key,
		unsigned char **encodedPassword,
		size_t *encodedPasswordSize)
{	
	size_t length;
	unsigned i;
	
	*encodedPassword = NULL;
	
	if (!asciiPassword)
	{
		synce_error("password parameter is NULL");
		goto error;
	}

	length = strlen(asciiPassword);

	*encodedPassword      = (unsigned char*)wstr_from_utf8(asciiPassword);
	*encodedPasswordSize  = sizeof(WCHAR) * (length + 1);

	for (i = 0; i < *encodedPasswordSize; i++)
	{
		(*encodedPassword)[i] ^= key;
	}

	return true;

error:
	synce_password_free(*encodedPassword);
	*encodedPassword = NULL;
	return false;
}

/**
 * Encode and send password on a socket
 */
bool synce_password_send(
		SynceSocket* socket,
		const char* asciiPassword,
		unsigned char key)
{
	bool success = false;
	unsigned char* encoded_password = NULL;
	size_t size = 0;
	uint16_t size_le = 0;
	
	if (!synce_password_encode(asciiPassword, key, &encoded_password, &size))
	{
		synce_error("failed to encode password");
	}

	size_le = htole16((uint16_t)size);

	if ( !synce_socket_write(socket, &size_le, sizeof(uint16_t)) )
	{
		synce_error("failed to write buffer size to socket");
		goto exit;
	}

	if ( !synce_socket_write(socket, encoded_password, size) )
	{
		synce_error("failed to write encoded password to socket");
		goto exit;
	}

	success = true;

exit:
	synce_password_free(encoded_password);
	return success;
}

bool synce_password_recv_reply(
		SynceSocket* socket,
		size_t size,
		bool* passwordCorrect)
{
	bool success = false;
	union
	{
		uint8_t   byte;
		uint16_t  word;
	} reply;

	if (size < 1 || size > 2)
	{
		synce_error("invalid size");
		goto exit;	
	}

	if (!synce_socket_read(socket, &reply, size))
	{
		synce_error("failed to read password reply");
		goto exit;	
	}

	if (size == 1)
	{
		/*synce_trace("password reply = 0x%02x (%i)", reply.byte, reply.byte);*/
		*passwordCorrect = reply.byte;
	}
	else /* size == 2 */
	{
		reply.word = letoh16(reply.word);
		/*synce_trace("password reply = 0x%04x (%i)", reply.word, reply.word);*/
		*passwordCorrect = reply.word;
	}

	/*synce_trace("Password was %s", *passwordCorrect ? "correct!" : "incorrect :-(");*/
	success = true;

exit:
	return success;
}

