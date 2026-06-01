static int get(int value) {
    return value;
}

static int mix(int a, int b, int c) {
    return a * 100 + b * 10 + c;
}

int main(void) {
    return mix(1, get(2), 3) != 123;
}
