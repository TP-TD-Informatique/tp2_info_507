/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/
#include <stdio.h>

int varA = 2;
char varB;

int main() {
    printf("@varA = %18p\n", (void *) &varA);
    printf("@varB = %18p\n", (void *) &varB);

    return 1;
}


/*
 * 1. Les adresses des variables sont :
 * @varA =     0x5592ba996010
 * @varB =     0x5592ba996015
 * On remarque qu'elles sont dans les adresses basses et qu'elles se suivent.
 *
 * 2. On faisant $ size ./question2, on obtient
 *    text    data     bss     dec     hex filename
 *    1615     604       4    2223     8af ./question2
 * On remarque que le bss est situé juste après le data et qu'il ne fait que 4 octets de taille (sizeof(char)), ce qui explique que les adresses de nos variables globales se suivent.
 */