#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

const size_t BF_SIZE = 262144;

int main(int argc, char *argv[]) {
  ifstream if_data("sandwich.bin", ios_base::binary);
  size_t size;
  if_data.read(reinterpret_cast<char *>(&size), sizeof(size_t));
  int buffer[BF_SIZE];
  size_t i, j;
  long max_so_far = 0, max_ending_here = 0;
  size_t curr_start = 0, max_start = 0, max_end = 0;
  for (i = 0; i < size; i += j) {
    if_data.read(reinterpret_cast<char *>(buffer), sizeof(int) * BF_SIZE);
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
  printf("[%lu, %lu[ -> %ld\n", max_start, max_end, max_so_far);
  return 0;
}

