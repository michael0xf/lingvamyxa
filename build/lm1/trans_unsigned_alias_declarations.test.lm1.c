#include <stdint.h>
int main(void) {
    uint8_t a = 1;
    uint16_t b = 2;
    uint32_t c = 3;
    uint64_t d = 4;
    int8_t e = 1;
    int16_t f = 2;
    int32_t g = 3;
    int64_t h = 4;
    return a != 1 || b != 2 || c != 3 || d != 4 || e != 1 || f != 2 || g != 3 || h != 4;
}
