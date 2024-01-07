#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>


int main() {
    printf("high: %lu  low: %lu  result: %d\n",0, 7, syscall(457, 0, 7));
    printf("high: %lu  low: %lu  result: %d\n",99, 99, syscall(457, 99, 99));
    printf("high: %lu  low: %lu  result: %d\n",23909, 2373487817, syscall(457, 23909, 2373487817));
    printf("high: %lu  low: %lu  result: %d\n",23909, 2373487818, syscall(457, 23909, 2373487818));
    return 0;
}