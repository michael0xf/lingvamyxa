static int variable = 5;

typedef struct LmBranch {
    int value;
} LmBranch;

static LmBranch branch = { 23 };
static LmBranch *node = &branch;

static int add(int left, int right) {
    return left + right;
}

static int wrap(int value) {
    return value;
}
int main(void) {
    return add(variable, wrap(add(2, node->value))) != 30;
}
