#ifndef MIXA_SELECTION_H
#define MIXA_SELECTION_H

#include <stddef.h>

/* Selection model: explicit paths + exceptions + reverse index.
 * See mixa_manager/SELECTION_MODEL.txt. Growable arrays; seed 8, double.
 */

#define MIXA_SEL_OK        0
#define MIXA_SEL_CONFLICT  1
#define MIXA_SEL_NOTHING   2
#define MIXA_SEL_ERR       3

typedef struct MixaSelRev {
    char *ancestor;
    char **paths;
    size_t count;
    size_t capacity;
} MixaSelRev;

typedef struct MixaSelection {
    char **selected;
    size_t selected_count;
    size_t selected_cap;

    char **exc_paths;
    char **exc_from;
    size_t exc_count;
    size_t exc_cap;

    MixaSelRev *revs;
    size_t rev_count;
    size_t rev_cap;

    int is_open;
} MixaSelection;

int mixa_selection_init(MixaSelection *sel);
void mixa_selection_release(MixaSelection *sel);
int mixa_selection_select(MixaSelection *sel, const char *path);
int mixa_selection_deselect(MixaSelection *sel, const char *path);
int mixa_selection_query(MixaSelection *sel, const char *path);

#endif
