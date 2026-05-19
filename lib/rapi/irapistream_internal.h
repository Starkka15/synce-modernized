#include "rapi_context.h"

struct _IRAPIStream
{
  RapiContext* context;
};

IRAPIStream* rapi_stream_new();
void rapi_stream_destroy(IRAPIStream* stream);

