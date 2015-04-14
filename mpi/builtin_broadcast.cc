#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <ctime>

using namespace std;

const size_t SIZE = 2000000;

int main(int argc, char *argv[]) {
  int rank, size;
  int data[SIZE];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  srand(time(NULL));

  if (rank == 0) {
    for (size_t i = 0; i < SIZE; ++i)
      data[i] = random();
  }
  MPI_Bcast(&data, SIZE, MPI_INT, 0, MPI_COMM_WORLD);

  printf("First value '%d' in %d\n", data[0], rank);

  MPI_Finalize();
  return 0;
}

