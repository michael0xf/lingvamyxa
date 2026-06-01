static int mix(int a, int b, int c) {
    return a * 100 + b * 10 + c;
}

int main(void) {
    return mix(1, 2, 3) != 123;
}

