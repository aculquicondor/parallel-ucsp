#include <mpi.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  int rank, size, data;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    data = 20;
  }
  MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);

  cout << data << " from " << rank << endl;

  MPI_Finalize();
  return 0;
}

