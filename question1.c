/****
 * info507: système d'exploitation
 * TP2
 * Kevin Traini
 ***/

#include <stdio.h>

void c() {
    int l = 0;
    printf("@l = %18p\n", (void *) &l);
}

void b() {
    int k = 0;
    printf("@k = %18p\n", (void *) &k);
    c();
}

void a() {
    int j = 0;
    printf("@j = %18p\n", (void *) &j);
    b();
}

int main() {
    int i = 0;
    printf("@i = %18p\n", (void *) &i);
    a();
    return 0;
}

/*
 * 1. Les adresses trouvées sont :
 * @i =     0x7ffcd9a20a54
 * @j =     0x7ffcd9a20a34
 * @k =     0x7ffcd9a20a14
 * @l =     0x7ffcd9a209f4
 *                     ^^^
 * Et donc on remarque que les 3 derniers bits changent en décroissant, ce qui démontre que les allocations de la pile se font vers les adresses basses.
 * On remarque en même temps que les adresses descendent de 20 bits.
 *
 * 2. Il s'agit d'adresse virtuelles, on ne manipule jamais nous-même les adresses physiques, c'est l'os qui s'en charge.
 */