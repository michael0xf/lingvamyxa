typedef struct LmRawBox {
    int value;
} LmRawBox;

static LmRawBox c_box;
int main(void) {
    c_box.value = 9;
    return c_box.value != 9;
}
