typedef struct LmTestText LmTestText;

struct LmTestText {
    unsigned length;
};

static LmTestText c_value;

static unsigned direct(const LmTestText *text) {
    return text -> length;
}

int main(void) {
    c_value.length = 9;
    return direct(& c_value) != 9 || c_value.length != 9;
}
