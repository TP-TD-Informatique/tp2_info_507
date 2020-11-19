/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/

/*
 * Question 10 :
 * Résultat ->
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
 *
 * Question 11 :
 * Avec size qui vaut 3, on obtient ->
  +-------+------------------+----------+------+
  | index |     address      |   size   | used |
  +-------+------------------+----------+------+
  |  000  |  0x55e22b4552a0  |      24  |      |
  |  001  |  0x55e22b4552f0  |      12  |      |
  |  002  |  0x55e22b455340  |      12  |      |
  |  003  |  0x55e22b455390  |      12  |      |
  +-------+------------------+----------+------+
  +-------+------------------+----------+------+
  | index |     address      |   size   | used |
  +-------+------------------+----------+------+
  |  000  |  0x55e22b4552a0  |      24  |  *   |
  +-------+------------------+----------+------+
 * Ainsi, on remarque que sur la première liste les 3 dernières lignes correspondent aux 3 malloc pour chaque lignes
 * du tableau (12 = 3 * 4). La première ligne correspond donc naturellement au pointeur sur le début du tableau T
 * (on en déduit que sizeof(int*) = 8 car 24 / 3 = 8).
 * Le GC_collect marque donc le pointeur du tableau, mais pas les lignes ce qui a pour effet des libérer.
 *
 * Question 12 :
 * 1. Les zones DATA et BSS, même si BSS peut être dangereux si certaines variables globales sont initialisées plus loin.
 *
 * 2. Appeler GC_collect dans GC_malloc
 *
 * 3.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "tinygc.h"

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
 * Global variable containing the address of the start of the heap (need to be initialized in 'main')
 */
address_t HEAP_BOTTOM;

/*
 * Initialize the garbage collector
 */
void GC_init(int argc) {
    STACK_TOP = (address_t) &argc;
    int *t = malloc(1);
    HEAP_BOTTOM = (address_t) t;
    free(t);
}

/*
 * display an ASCII representation of a linked list of blocks
 */
void print_list(cell_t *list) {
    DEBUG(2, "  +-------+------------------+----------+------+\n");
    DEBUG(2, "  | index |     address      |   size   | used |\n");
    DEBUG(2, "  +-------+------------------+----------+------+\n");

    cell_t *p;
    int index = 0;
    for (p = list; p != NULL; p = p->next) {
        DEBUG(2, "  |  %03i  |  %10p  |  %6zu  |  %c   |\n",
                index++,
                p->start,
                p->size,
                p->flags % 2 == 1 ? '*' : ' ');
    }

    DEBUG(2, "  +-------+------------------+----------+------+\n");
}

/*
 * special malloc that calls system's malloc and records the corresponding
 * blocks inside global variable ``BLOCKS``
 */
void *GC_malloc(size_t size) {
    void *t = malloc(size);
    insert_BLOCKS(t, size);
    GC_collect();
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
 * mark blocks referenced from the stack
 */
void mark_from_heap() {
    int *t = malloc(1);
    DEBUG(1, "tas : %p -> %p\n", HEAP_BOTTOM, (void *) t);
    mark_region(HEAP_BOTTOM, (address_t) t);
    free(t);
}

/*
 * mark blocks referenced from the stack,
 */
void mark_from_stack() {
    int tmp;
    DEBUG(1, "pile : %p -> %p\n", (void *) &tmp, STACK_TOP);
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
    mark_from_heap();

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
    int size = 3;
    int **T = GC_malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        T[i] = GC_malloc(size * sizeof(int));
    }

    print_list(BLOCKS);
}

/***************************************************************************
 ***   ``main`` function   *************************************************
 ***************************************************************************/
int main(int argc, char **argv) {
    GC_init(argc);

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
