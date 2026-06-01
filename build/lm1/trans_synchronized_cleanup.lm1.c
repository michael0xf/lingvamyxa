#include <stdio.h>

static int depth = 0;

static void lm_synchronized_enter(void *object) {
    (void)object;
    ++depth;
}

static void lm_synchronized_leave(void *object) {
    (void)object;
    --depth;
}
static int leaveByReturn(void) {
    int lock = 0;
    void *lm_sync_0 = (void *)(& lock);
    lm_synchronized_enter(lm_sync_0);
    {
        {
            int lm_return_0 = 10 + depth;
            lm_synchronized_leave(lm_sync_0);
            return lm_return_0;
        }
    }
    lm_synchronized_leave(lm_sync_0);
    return - 1;
}

static int leaveByWrappedReturn(void) {
    int lock = 0;
    void *lm_sync_1 = (void *)(& lock);
    lm_synchronized_enter(lm_sync_1);
    {
        {
            int lm_return_0 = 10 + depth;
            lm_synchronized_leave(lm_sync_1);
            return lm_return_0;
        }
    }
    lm_synchronized_leave(lm_sync_1);
    return - 1;
}

static int leaveByBreak(void) {
    int lock = 0;
    int i = 0;
    while (i < 3) {
        void *lm_sync_2 = (void *)(& lock);
        lm_synchronized_enter(lm_sync_2);
        {
            lm_synchronized_leave(lm_sync_2);
            break;
        }
        lm_synchronized_leave(lm_sync_2);
        i = i + 1;
    }
    return depth;
}

static int leaveByContinue(void) {
    int lock = 0;
    int i = 0;
    while (i < 1) {
        i = i + 1;
        void *lm_sync_3 = (void *)(& lock);
        lm_synchronized_enter(lm_sync_3);
        {
            lm_synchronized_leave(lm_sync_3);
            continue;
        }
        lm_synchronized_leave(lm_sync_3);
    }
    return depth;
}

int main(void) {
    int result = 0;
    result = leaveByReturn() + leaveByWrappedReturn() + leaveByBreak() + leaveByContinue();
    printf("%d\n", result);
    return result != 22;
}

