int main(void) {
    int x = 2;
    int y = 3;
    int z = 4;
    char array[x][y][z];
    array[1][2][3] = 'a';
    char fallback[x][y][z];
    fallback[1][2][3] = 'b';
    return array[1][2][3] != 'a' || fallback[1][2][3] != 'b';
}
