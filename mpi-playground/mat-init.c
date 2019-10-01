#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main() {
  srand(time(NULL));
  int row = 1000, col = 100;
  int i, j;
  char *filename = "1000x100.matB";
  FILE* result = fopen(filename, "w+");
  fprintf(result, "%d %d\n", row, col);
  for (i = 0; i < row; i++) {
      for (j = 0; j < col; j++) {
          fprintf(result, "%d ", rand() % 100);
      }
      fprintf(result, "\n");
  }
  fclose(result);
  return 0;
}