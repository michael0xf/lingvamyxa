static int pick(int values[], int index, int fallback) {
    return values[index] + fallback;
}

static int mix(int a, int b, int c) {
    return a * 100 + b * 10 + c;
}

int main(void) {
    int values[3] = {10, 20, 30};
    if (pick(values, 1, 5) != 25) {
        return 1;
    }
    if (mix(1, 2, 3) != 123) {
        return 2;
    }
    return 0;
}
