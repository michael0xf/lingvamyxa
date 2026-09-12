/* mixa_file_win32.h - concrete Win32 file handle; portable API in mixa_file.h.
 *
 * Opaque to callers of mixa_file.h. Owner holds MixaFile* and closes it.
 */
#ifndef MIXA_FILE_WIN32_H
#define MIXA_FILE_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mixa_manager/mixa_file.h"

struct MixaFile {
    HANDLE handle; /* INVALID_HANDLE_VALUE when closed */
    int mode;
    int is_open;
};

#endif
