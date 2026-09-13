/* mixa_file_win32_l2_win.h - minimal system-header adapter for the clean
 * L2 port of mixa_file_win32 (ticket 20260913-114500).
 *
 * The real mixa_file_win32.h combines "#define WIN32_LEAN_AND_MEAN" with
 * "#include <windows.h>" before declaring the concrete MixaFile struct.
 * L1's own include: line syntax only ever emits bare #include directives
 * (no interleaved #define), so this tiny wrapper reproduces the same
 * WIN32_LEAN_AND_MEAN + <windows.h> pairing as its own standalone system
 * header -- it declares nothing of mixa_manager's own, only the same
 * standard Win32 API surface every other manager module already links
 * against (CreateFileW/ReadFile/WriteFile/CloseHandle/GetFileSize/
 * SetFilePointer/GetLastError/SetLastError/MultiByteToWideChar, all via
 * -lkernel32, matching mixa_file_win32.lm1's own real link line).
 */
#ifndef MIXA_FILE_WIN32_L2_WIN_H
#define MIXA_FILE_WIN32_L2_WIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif
