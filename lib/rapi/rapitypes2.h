/* $Id: rapitypes2.h 3791 2009-07-12 17:27:04Z mark_ellis $ */
#ifndef __rapitypes2_h__
#define __rapitypes2_h__

#include <synce.h>

#ifdef __cplusplus
namespace synce
{
extern "C"
{
#endif


typedef struct _GUID {
  DWORD Data1;
  WORD Data2;
  WORD Data3;
  BYTE Data4[8];
} GUID, UUID;

typedef GUID RAPIDEVICEID;

typedef enum {
        RAPI_DEVICE_DISCONNECTED = 0,
        RAPI_DEVICE_CONNECTED = 1
} RAPI_DEVICESTATUS;

typedef enum {
        RAPI_GETDEVICE_NONBLOCKING,
        RAPI_GETDEVICE_BLOCKING
} RAPI_GETDEVICEOPCODE;

typedef enum {
        RAPI_CONNECTION_USB = 0,
        RAPI_CONNECTION_IR = 1,
        RAPI_CONNECTION_SERIAL = 2,
        RAPI_CONNECTION_NETWORK = 3
} RAPI_CONNECTIONTYPE;

/*
 * The MS definition has bstrName and bstrPlatform as type BSTR,
 * which is not a sensible option for us, so we have them as normal
 * strings.
 */
typedef struct {
        RAPIDEVICEID DeviceId;
        DWORD dwOsVersionMajor;
        DWORD dwOsVersionMinor;
        char *bstrName;
        char *bstrPlatform;
} RAPI_DEVICEINFO;


/*
 * MS has ipaddr and hostIpaddr as sockaddr, which makes no sense for
 * us since applications should not be using the naked socket connection.
 * We'll therefore just have the textual IP addresses.
 */
typedef struct {
        char *ipaddr;
        char *hostIpaddr;
        RAPI_CONNECTIONTYPE connectionType;
} RAPI_CONNECTIONINFO;


#ifdef __cplusplus
}
}
#endif

#endif /* __rapitypes2_h__ */

