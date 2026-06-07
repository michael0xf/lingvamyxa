int main(void);

int main(void) {
    int var;
    var = 0;
    int var2 = 1;
    int8_t raw = 0;
    int8_t state = -1;
    int8_t no = 0;
    int8_t yes = 1;
    int8_t status = 1;
    int8_t legacy = -1;
    if (var == 0 && state == -1) {
        return status;
    }
    if (var == 0) {
        return no;
    }
    if (raw == 0 && legacy == -1) {
        return yes;
    }
    return 0;
}
