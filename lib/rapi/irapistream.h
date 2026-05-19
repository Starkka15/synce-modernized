/* $Id: irapistream.h 3813 2009-07-21 17:56:08Z mark_ellis $ */
#ifndef __irapistream_h__
#define __irapistream_h__

#include <synce.h>

#ifdef __cplusplus
namespace synce
{
extern "C"
{
#endif



struct _IRAPIStream;
typedef struct _IRAPIStream IRAPIStream;

ULONG IRAPIStream_Release(IRAPIStream* stream);

HRESULT IRAPIStream_Read(
		IRAPIStream* stream,
		void *pv,
		ULONG cb,
		ULONG *pcbRead);

HRESULT IRAPIStream_Write(
		IRAPIStream* stream,
		void const *pv,
		ULONG cb,
		ULONG *pcbWritten);

int IRAPIStream_GetRawSocket(IRAPIStream* stream);


#ifdef __cplusplus
}
}
#endif

#endif /* __irapistream_h__ */
