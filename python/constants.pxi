cdef extern from "rapitypes.h":
    cdef enum: # dwDesiredAccess
        GENERIC_WRITE                = 0x40000000
        GENERIC_READ                 = 0x80000000

    cdef enum: # dwCreationDisposition 
        CREATE_NEW                   = 1
        CREATE_ALWAYS                = 2
        OPEN_EXISTING                = 3
        OPEN_ALWAYS                  = 4
        TRUNCATE_EXISTING            = 5
        OPEN_FOR_LOADER              = 6
