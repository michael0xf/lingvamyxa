#if defined(_WIN32)
static int lm_os_import_value(void) {
    return 1;
}

static int lm_os_import_plus_one(void) {
    return lm_os_import_value() + 1;
}
#else
static int lm_os_import_value(void) {
    return 2;
}

static int lm_os_import_plus_one(void) {
    return lm_os_import_value() + 1;
}
#endif

int main(void) {
    return lm_os_import_plus_one() != 2 && lm_os_import_plus_one() != 3;
}
