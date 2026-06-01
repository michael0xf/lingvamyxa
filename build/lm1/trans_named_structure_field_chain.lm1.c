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
    return 0;
}

