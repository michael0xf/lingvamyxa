static int take(int value) {
    return value;
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
    if (result != - 1) {
        return 4;
    }
    result = take(mask & 3) + take(mask | 1) + take(mask ^ 3);
    return result != 14;
}
