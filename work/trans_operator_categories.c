static int take(int value);
static int add(int left, int right);
int main(void);

static int take(int value) {
    return value;
}

static int add(int left, int right) {
    return left + right;
}

int main(void) {
    int i = 3;
    int mask = 6;
    int result = 0;
    result = take(i --);
    if (result != 3 || i != 2) {
        return 1;
    }
    result = take(-- i);
    if (result != 1 || i != 1) {
        return 2;
    }
    result = take(++ i);
    if (result != 2 || i != 2) {
        return 3;
    }
    result = take(- i) + take(+ i) + take(~ 0);
    if (result != -1) {
        return 4;
    }
    result = add(1 + (-1), -1);
    if (result != -1) {
        return 5;
    }
    result = add(1 + 2, 4 - 1);
    if (result != 6) {
        return 6;
    }
    result = take(1 + -- i);
    if (result != 2 || i != 1) {
        return 7;
    }
    result = take(1 + -1);
    if (result != 0) {
        return 8;
    }
    result = take(mask & 3) + take(mask | 1) + take(mask ^ 3);
    if (result != 14) {
        return 9;
    }
    return 0;
}
