static int nested(int value, int mix) {
    return value + mix;
}

static int sum4(int a, int b, int c, int d) {
    return a + b + c + d;
}

int main(void) {
    int arr[4] = {1, 2, 3, 4};
    int a = 2;
    int b = 3;
    int c = 5;
    int i = 1;
    int index = 1;
    int x = 6;
    int y = 3;
    int result = 0;
    result = sum4(a + b, c --, nested(++ i, x | y), arr[index + 1]);
    if (result != 22) {
        return 1;
    }
    if (c != 4 || i != 2) {
        return 2;
    }
    result = sum4((a + b) * 2, c + 1, nested(i --, x & y), arr[index]);
    if (result != 21) {
        return 3;
    }
    return i != 1;
}
