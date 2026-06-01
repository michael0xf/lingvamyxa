typedef struct Box Box;

struct Box {
    int value;
};

typedef struct Holder Holder;

struct Holder {
    Box box;
};

static int read(const Holder *holder) {
    return holder -> box 