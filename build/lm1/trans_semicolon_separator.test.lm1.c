#if defined(_WIN32)
#include <direct.h>
static int platform_value(void) {
    return 1;
}
#else
#include <unistd.h>
static int platform_value(void) {
    return 2;
}
#endif

int main(void) {
    return platform_value() != 1 && platform_value() != 2;
}
