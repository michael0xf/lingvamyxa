typedef struct LmRawBox {
    int value;
} LmRawBox;

static LmRawBox boxes[2];
int main(void) {
    boxes[1].value = 7;
    return boxes[1].value != 7;
}
