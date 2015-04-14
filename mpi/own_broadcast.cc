#include <mpi.h>
#include <iostream>

using namespace std;

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
  int rank, size, data;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    data = 20;
  }
  My_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);

  cout << data << " from " << rank << endl;

  MPI_Finalize();
  return 0;
}

