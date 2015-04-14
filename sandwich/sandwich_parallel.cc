#include <mpi.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void decompose_domain(size_t domain_size, int world_rank,
    int world_size, size_t &subdomain_start, size_t &subdomain_size) {
  if (world_size > domain_size) {
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  subdomain_start = domain_size / world_size * world_rank;
  subdomain_size = domain_size / world_size;
  if (world_rank == world_size - 1) {
    subdomain_size += domain_size % world_size;
  }
}

size_t read_domain_size(ifstream &if_data) {
  if_data.seekg(0);
  size_t size;
  if_data.read(reinterpret_cast<char *>(&size), sizeof(size_t));
  return size;
}

void read_elements(ifstream &if_data, vector<int> &data,
    size_t subdomain_start, size_t subdomain_size) {
  data.resize(subdomain_size);
  if_data.seekg(subdomain_start * sizeof(int) + sizeof(size_t));
  if_data.read(reinterpret_cast<char *>(data.data()),
      subdomain_size * sizeof(int));
}

int main(int argc, char *argv[]) {
  int world_rank, world_size;
  ifstream if_data("sandwich.bin", ios_base::binary);
  vector<int> data;
  size_t domain_size = read_domain_size(if_data),
         subdomain_start, subdomain_size;
  if_data.close();

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if_data.open("sandwich.bin", ios_base::binary);
  decompose_domain(domain_size, world_rank, world_size,
      subdomain_start, subdomain_size);
  read_elements(if_data, data, subdomain_start, subdomain_size);
  if_data.close();

  printf("Elements in %d (%lu):", world_rank, subdomain_size);
  for (int i = 0; i < data.size(); ++i)
    printf(" %d", data[i]);
  puts("");

  if (world_rank == 0) {
    // merge results
  }

  MPI_Finalize();
  return 0;
}

