#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;

const size_t SIZE = 2000000;

int My_Bcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm) {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Status status;
  if (rank == root) {
    for (int i = 0; i < size; ++i)
      if (i != root)
        MPI_Send(buffer, count, datatype, i, 0, comm);
  } else {
    MPI_Recv(buffer, count, datatype, root, MPI_ANY_TAG, comm,
        &status);
  }
  return 0;
}

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
  My_Bcast(&data, SIZE, MPI_INT, 0, MPI_COMM_WORLD);

  printf("First value '%d' in %d\n", data[0], rank);

  MPI_Finalize();
  return 0;
}

