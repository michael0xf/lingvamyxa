typedef struct Leaf Leaf;

struct Leaf {
    int value;
};

typedef struct Branch Branch;

struct Branch {
    Leaf *leaf;
};

static int read(const Branch *branch) {
    return branch -> leaf -> value;
}

int main(void) {
    Leaf leaf;
    Branch branch;
    leaf.value = 7;
    branch.leaf = & leaf;
    return read(& branch) != 7;
}
