#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
int lm_test_os_value(void) {
    return 1;
}

#elif defined(__APPLE__)
int lm_test_os_value(void) {
    return 2;
}

#elif defined(__unix__)
int lm_test_os_value(void) {
    return 3;
}

#else
int lm_test_os_value(void) {
    return 4;
}

#endif
int main(void) {
    return lm_test_os_value() == 0;
}
