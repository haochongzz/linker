/***********************/
/* BIND, TYPE, SECTION */
/***********************/

/* Global, Notype, Undefined */
extern unsigned long long sum(unsigned long long *a, unsigned long long n); 

/* Global, Object, .data */
unsigned long long array[2] = {0x12340000, 0xabcd};
/* Global, Object, .data */
unsigned long long bias = 0xf00000000;

unsigned long long main() {
    unsigned long long val = sum(array, 2);
    return val;
}