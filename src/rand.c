
#if RAND_FUNC == 0

#include <stdlib.h>

unsigned int seedp;

void rand_init(unsigned int s) {
    seedp = s;
}

float rand_norm() {
    return ((float) rand_r(&seedp)) / RAND_MAX;
}

unsigned int rand_imax(unsigned int m) {
    return rand_r(&seedp) % m;
}

#elif RAND_FUNC == 1

double genrand();
void sgenrand(unsigned long seed);

void rand_init(unsigned int s) {
    sgenrand(s);
}

float rand_norm() {
    return genrand();
}

unsigned int rand_imax(unsigned int m) {
    return (unsigned int)(genrand() * m);
}

#endif
