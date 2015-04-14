#include <fstream>
#include <cstdio>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

const size_t BF_SIZE = 262144;
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
  stringstream input(argv[1]);
  input >> size;
  ofstream gen_file("sandwich.bin", ios::binary);
  gen_file.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
  int buffer[BF_SIZE];
  size_t i, j;
  for (i = 0; i < size; i += j) {
    for (j = 0; j < BF_SIZE and i + j < size; ++j) {
      buffer[j] = get_val();
    }
    gen_file.write(reinterpret_cast<const char *>(&buffer), j * sizeof(int));
  }
  printf("%lu bytes written\n", i * sizeof(int) + sizeof(size_t));
  gen_file.close();
  return 0;
}
