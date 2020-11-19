/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/

/**
 * Initialize the tiny garbage collector
 * @param argc The first parameter of main()
 */
void GC_init(int argc);

/**
 * Allocate a block of memory
 * @param size size of the block
 * @return address of memory block
 */
void *GC_malloc(size_t size);