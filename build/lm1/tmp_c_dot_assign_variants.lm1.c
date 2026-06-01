typedef struct LmTestText LmTestText;

struct LmTestText {
    unsigned length;
};

static LmTestText c_value;

int main(void) {
    c_value.length = 9;
    return c_value . length != 9;
}

