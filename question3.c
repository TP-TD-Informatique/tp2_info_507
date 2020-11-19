/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/
#include <stdio.h>
#include <stdlib.h>

typedef void *address_t;

int main() {
    address_t *x;

    x = malloc(sizeof(address_t));
    *x = (void *) 3735928559;

    printf(" x = %p\n", (address_t) x);
    printf("*x = %p\n", (address_t) *x);
    printf("&x = %p\n", (address_t) &x);
}

/*
 * La première ligne : x = 0x5574fd0732a0 donne l'adresse renvoyée par le malloc, c'est l'adresse ou se trouve le nombre 3735928559.
 * La seconde ligne : *x = 0xdeadbeef c'est notre nombre (3735928559), mais en hexadécimal.
 * La troisième ligne : &x = 0x7ffffd8c5b50 c'est l'adresse à laquelle se trouve notre pointeur.
 *
 * On remarque que le malloc alloue de la mémoire dans le tas, et que le pointeur est stocké dans la pile. C'est pour cela qu'un malloc est persistant (il alloue dans le tas)
 */