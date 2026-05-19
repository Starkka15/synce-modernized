#define _BSD_SOURCE 1
#include "rapi.h"
#include "rapi_context.h"
#include "synce_log.h"
#include <stdlib.h>
#include <string.h>

struct _RapiConnection
{
  RapiContext* context;
  /* if the RapiConnection owns the SynceInfo object, it's here */
  SynceInfo* owned_info;
};

RapiConnection* rapi_connection_from_name(const char* device_name)
{
  SynceInfo* info = synce_info_new(device_name);
  if (info == NULL)
    return NULL;
  RapiConnection* result = rapi_connection_from_info(info);
  if (result)
    result->owned_info = info;
  else
    synce_info_destroy(info);

  return result;
}

RapiConnection* rapi_connection_from_info(SynceInfo* info)
{
  RapiConnection* connection =
    (RapiConnection*)calloc(1, sizeof(RapiConnection));

  if (connection)
  {
    connection->context = rapi_context_new();
    if (!connection->context)
    {
      synce_error("Failed to create RapiContext object");
      free(connection);
      return NULL;
    }

    connection->context->info = info;
  }

  return connection;
}

/** Select what connection is used for RAPI calls */
void rapi_connection_select(RapiConnection* connection)
{
  if (connection)
  {
    rapi_context_set(connection->context);
  }
  else
    rapi_context_set(NULL);
}

/** Destroy RAPI connection - use this after CeRapiUninit() */
void rapi_connection_destroy(RapiConnection* connection)
{
  if (connection)
  {
    synce_info_destroy(connection->owned_info);
    rapi_context_unref(connection->context);
    free(connection);
  }
}


const char *
rapi_connection_get_name(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_name(context->info);

  return NULL;
}

bool
rapi_connection_get_os_version(RapiConnection* connection, unsigned int *os_major, unsigned int *os_minor)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_os_version(context->info, os_major, os_minor);

  return false;
}

unsigned int
rapi_connection_get_build_number(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_build_number(context->info);

  return 0;
}

unsigned int
rapi_connection_get_processor_type(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_processor_type(context->info);

  return 0;
}

const char *
rapi_connection_get_os_name(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_os_name(context->info);

  return NULL;
}

const char *
rapi_connection_get_model(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_model(context->info);

  return NULL;
}

const char *
rapi_connection_get_device_ip(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_device_ip(context->info);

  return NULL;
}

const char *
rapi_connection_get_local_ip(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_info_get_local_ip(context->info);

  return NULL;
}

int
rapi_connection_get_fd(RapiConnection* connection)
{
  RapiContext *context = NULL;
  if (connection)
    context = connection->context;
  else
    context = rapi_context_current();

  if (context)
    return synce_socket_get_descriptor(context->socket);

  return 0;
}

