#include <cstdio>
#include <vector>

using namespace std;

const size_t BF_SIZE = 1048576;

int main(int argc, char *argv[]) {
  FILE *f_data = fopen("sandwich.bin", "r");
  size_t size;
  fread(reinterpret_cast<void *>(&size), sizeof(size_t), 1, f_data);
  int buffer[BF_SIZE];
  size_t i, j;
  long max_so_far = 0, max_ending_here = 0;
  size_t curr_start = 0, max_start = 0, max_end = 0;
  for (i = 0; i < size; i += j) {
    fread(reinterpret_cast<void *>(buffer), sizeof(int), BF_SIZE, f_data);
    for (j = 0; j < BF_SIZE and i + j < size; ++j) {
      max_ending_here += buffer[j];
      if (max_ending_here < 0) {
        max_ending_here = 0;
        curr_start = j + 1;
      } else if (max_ending_here > max_so_far) {
        max_so_far = max_ending_here;
        max_start = curr_start;
        max_end = j + 1;
      }
    }
  }
  fclose(f_data);
  printf("[%lu, %lu[ -> %ld\n", max_start, max_end, max_so_far);
  return 0;
}

