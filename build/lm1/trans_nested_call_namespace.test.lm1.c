static int get(int a, int b) {
    return a + b;
}

static int print(int a, int b, int c) {
    return a + b + c;
}

int main(void) {
    return print(1, 2, get(1, 2)) != 6;
}
