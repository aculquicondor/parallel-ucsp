#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;

const size_t BF_SIZE = 1048576;
const int MAX_VAL = 100000000;
const int RANGE_SIZE = 2 * MAX_VAL + 1;

int get_val() {
  return random() % RANGE_SIZE - MAX_VAL;
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    exit(1);
  srand(time(NULL));
  size_t size;
  size = atol(argv[1]);
  FILE *gen_file = fopen("sandwich.bin", "w+");
  fwrite(reinterpret_cast<void *>(&size), sizeof(size_t), 1, gen_file);
  int buffer[BF_SIZE];
  size_t i, j;
  for (i = 0; i < size; i += j) {
    for (j = 0; j < BF_SIZE and i + j < size; ++j) {
      buffer[j] = get_val();
    }
    fwrite(reinterpret_cast<void *>(buffer), sizeof(int), j, gen_file);
  }
  printf("%lu bytes written\n", i * sizeof(int) + sizeof(size_t));
  fclose(gen_file);
  return 0;
}
