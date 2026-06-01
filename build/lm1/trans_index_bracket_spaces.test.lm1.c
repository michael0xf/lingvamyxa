int main(void) {
    int values[4] = {10, 20, 30, 40};
    int index = 2;
    values[index - 1] = 99;
    return values[index - 1] != 99;
}
