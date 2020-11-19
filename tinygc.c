/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/

/*
 * Question 10 : résultat
  +-------+------------------+----------+------+
  | index |     address      |   size   | used |
  +-------+------------------+----------+------+
  |  000  |  0x55a577e0e2a0  |       4  |      |
  |  001  |  0x55a577e0e2f0  |       4  |      |
  |  002  |  0x55a577e0e340  |       4  |      |
  |  003  |  0x55a577e0e390  |       1  |      |
  +-------+------------------+----------+------+
  +-------+------------------+----------+------+
  | index |     address      |   size   | used |
  +-------+------------------+----------+------+
  |  000  |  0x55a577e0e2f0  |       4  |  *   |
  |  001  |  0x55a577e0e390  |       1  |  *   |
  +-------+------------------+----------+------+
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define WORD_SIZE 4

/*
 * verbosity level
 */
int VERBOSE = 0;

/*
 * type for "raw" addresses
 * ``void*`` is not appropriate because C fordids pointer arithmetic on
 * ``void*``...
 */
typedef char *address_t;

/*
 * type of linked lists
 *
 * Note that because pointers are aligned, the field ``in_use`` could be put
 * in the least significant bit of ``start``!
 */
typedef struct cell {
    address_t start;
    size_t size;
    int flags; // least significant bit is set to 1 when block is used
    struct cell *next;
} cell_t;

/*
 * prototypes
 */
void print_list(cell_t *list);

void insert_BLOCKS(address_t start, size_t size);

void *GC_malloc(size_t size);

void mark_BLOCK(address_t v);

void mark_region(address_t start, address_t end);

void mark_from_heap();

void mark_from_stack();

void GC_collect();

void DEBUG(int v, char *format, ...);

/*
 * GLOBAL variable containing the list of user allocated blocks
 */
cell_t *BLOCKS = NULL;

/*
 * GLOBAL variable containing the address of the start of the stack (this
 * needs to be initialized in ``main``.
 */
address_t STACK_TOP;

/*
 * display an ASCII representation of a linked list of blocks
 */
void print_list(cell_t *list) {
    fprintf(stderr, "  +-------+------------------+----------+------+\n");
    fprintf(stderr, "  | index |     address      |   size   | used |\n");
    fprintf(stderr, "  +-------+------------------+----------+------+\n");

    cell_t *p;
    int index = 0;
    for (p = list; p != NULL; p = p->next) {
        fprintf(stderr, "  |  %03i  |  %10p  |  %6zu  |  %c   |\n",
                index++,
                p->start,
                p->size,
                p->flags % 2 == 1 ? '*' : ' ');
    }

    fprintf(stderr, "  +-------+------------------+----------+------+\n");
}

/*
 * special malloc that calls system's malloc and records the corresponding
 * blocks inside global variable ``BLOCKS``
 */
void *GC_malloc(size_t size) {
    void *t = malloc(size);
    insert_BLOCKS(t, size);
    return t;
}

/*
 * mark blocks from a memory region
 */
void mark_region(address_t start, address_t end) {
    address_t p; // pointer to memory
    address_t v; // value at pointer p
    for (p = start; p < end; p += WORD_SIZE) {
        v = (address_t) *((uint64_t *) p);
        mark_BLOCK(v);
    }
}

/*
 * mark the block containing address ``v`` as used
 */
void mark_BLOCK(address_t v) {
    for (cell_t *p = BLOCKS; p != NULL; p = p->next) {
        if (p->start <= v && p->start + p->size > v) {
            p->flags = 1;
        }
    }
}

/*
 * mark blocks referenced from the stack,
 */
void mark_from_stack() {
    int tmp;
    mark_region((address_t) &tmp, STACK_TOP);
}

/*
 * look at the stack and heap for references to user allocated blocks, and
 * free all the blocks that were not referenced
 */
void GC_collect() {
    // unmark all blocks
    cell_t *p;
    for (p = BLOCKS; p != NULL; p = p->next) {
        // set "used" bit to 0
        p->flags &= ~1;
    }

    // mark blocks referenced from stack
    mark_from_stack();

    // mark blocks referenced from heap
    // TODO

    // free all unused blocks
    cell_t *tmp;
    p = BLOCKS;
    cell_t *prev = NULL;
    while (p != NULL) {
        if (!(p->flags & 1)) {
            free(p->start);
            if (prev == NULL) { // first element
                BLOCKS = p->next;
                // NOTE: do not set prev, it should remain NULL
            } else {
                prev->next = p->next;
            }
            tmp = p->next;
            free(p);
            p = tmp;
        } else {
            prev = p;
            p = p->next;
        }
    }
}

/***************************************************************************
 **** tests ****************************************************************/
void test() {
    int *a = GC_malloc(sizeof(int));
    a = NULL;
    int *b = GC_malloc(sizeof(int));
    *b = 2;
    int *c = GC_malloc(sizeof(int));
    c = NULL;
    char *d = GC_malloc(sizeof(char));
    *d = 'd';
    printf("%p", (void *) a);
    printf("%p\n\n", (void *) c);

    print_list(BLOCKS);

    GC_collect();

    print_list(BLOCKS);
}

/***************************************************************************
 ***   ``main`` function   *************************************************
 ***************************************************************************/
int main(int argc, char **argv) {
    int tmp;
    STACK_TOP = (address_t) &tmp;

    // get first command line argument for verbosity
    if (argc > 1) {
        VERBOSE = atoi(argv[1]);
        printf("VERBOSE = %i\n", VERBOSE);
    }
    // tests
    test();

    return 0;
}

/***************************************************************************
 ***************************************************************************
 ***   DO NOT MODIFY FOLLOWING FUNCTIONS   *********************************
 ***************************************************************************
 ***************************************************************************/

/*
 * display debug messages:
 * ``DEBUG`` takes a first argument giving the verbosity level at which the
 * message is displayed
 * the remaining arguments are interpreted as for the standard ``printf``
 * function...
 *
 * Note: the global variable ``VERBOSE`` is initialized in ``main`` as the
 * value of the first argument ``argv[1]``.
 */
void DEBUG(int v, char *format, ...) {
    va_list args;
    if (v <= VERBOSE) {
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}

/*
 * insert a new cell into the global variable ``BLOCKS``
 */
void insert_BLOCKS(address_t start, size_t size) {
    cell_t *c = malloc(sizeof(cell_t));
    c->start = start;
    c->size = size;
    c->next = NULL;
    c->flags = 0;

    if (BLOCKS == NULL) {
        // initialize BLOCKS list if necessary
        BLOCKS = c;
    } else if (c->start < BLOCKS->start) {
        // insert in first position
        c->next = BLOCKS;
        BLOCKS = c;
    } else {
        // otherwise, look for position just before the new cell
        cell_t *p;
        for (p = BLOCKS; p->next != NULL; p = p->next) {
            if (p->next->start > c->start) {
                break;
            }
        }
        c->next = p->next;
        p->next = c;
    }
}
