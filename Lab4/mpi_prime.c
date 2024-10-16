#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int N, *v;

int is_prime(int n) {
  if (n < 2) return 0;
  else return prime(n, n-1);
}

int prime(int x, int y) {
  if (y == 1) return 1;
  else if (x % y == 0) return 0;
  else return prime(x, y - 1);
}

void load_vector() {
  int i;

  N = 32000;
  v = (int *) malloc(N * sizeof(int));
  for (i = 0; i < N; i++)
    v[i] = i;
  return;
}

main(int argc, char **argv) {
  ...
}