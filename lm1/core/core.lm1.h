#ifndef LM_CORE_H
#define LM_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pure C API for the Lingvamyxa core.
 *
 * Keep this layer independent from:
 *   - Qt
 *   - Android
 *   - Windows API
 *   - POSIX-specific APIs
 *   - GUI/event-loop assumptions
 *
 * Platform shells can call this API from C, C++, JNI, Qt, etc.
 */

int lm_add(int a, int b);

const char *lm_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LM_CORE_H */
