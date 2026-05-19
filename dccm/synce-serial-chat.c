#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#define CLIENT_SIZE        6
#define CLIENTSERVER_SIZE  (CLIENT_SIZE + 6)

static const char client[] = "CLIENT";
static const char clientserver[] = "CLIENTSERVER";

int main(int argc, char** argv)
{
	char buffer[CLIENT_SIZE+1];
	ssize_t bytes_left = CLIENT_SIZE;
	ssize_t bytes_read;
	ssize_t bytes_read_total = 0;

	if (1 != argc)
	{
		printf("%s: A program that expects the string '%s' and responds with '%s'\n",
				argv[0], client, clientserver);
		return 1;
	}

	openlog("synce-serial-chat", 0, LOG_DAEMON);
	
	memset(buffer, 0, sizeof(buffer));

	bytes_left = CLIENT_SIZE;

	while (bytes_left > 0)
	{
		bytes_read = read(STDIN_FILENO, buffer+bytes_read_total, bytes_left);

		if (bytes_read < 0)
			break;
	
		bytes_left -= bytes_read;	
		bytes_read_total += bytes_read;
	}

	if (bytes_left != 0)
	{
		syslog(LOG_INFO, "Failed to read last %i bytes. Only %i bytes read: '%s'", 
		       (int)bytes_left, (int)bytes_read_total, buffer);
		return 1;
	}

	if (memcmp(buffer, client, CLIENT_SIZE) != 0)
	{
		syslog(LOG_INFO, "Received string not 'CLIENT' but '%s'", buffer);
		return 1;
	}

	if (write(STDOUT_FILENO, clientserver, CLIENTSERVER_SIZE) != CLIENTSERVER_SIZE)
	{
		syslog(LOG_INFO, "Unable to write '%s'", clientserver);
		return 1;
	}
	
	return 0;
}
