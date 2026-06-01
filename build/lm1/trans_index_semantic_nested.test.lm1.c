static int pick(int value) {
    return value;
}

static int offset(int value, int delta) {
    return value + delta;
}

int main(void) {
    int values[5] = {0, 10, 20, 30, 40};
    int indices[3] = {0, 1, 2};
    int base = 1;
    int result = 0;
    result = values[indices[offset(base, 1)] + pick(1)];
    if (result != 30) {
        return 1;
    }
    values[indices[pick(2)] + 2] = 99;
    return values[4] != 99;
}
