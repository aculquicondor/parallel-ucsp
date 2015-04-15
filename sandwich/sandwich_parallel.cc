#include <mpi.h>
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

size_t read_domain_size(FILE *f_data) {
  fseek(f_data, 0, SEEK_SET);
  size_t size;
  fread(reinterpret_cast<void *>(&size), sizeof(size_t), 1, f_data);
  return size;
}

void read_elements(FILE *f_data, vector<int> &data,
    size_t start, size_t size) {
  data.resize(size);
  fseek(f_data, start * sizeof(int) + sizeof(size_t), SEEK_SET);
  fread(reinterpret_cast<void *>(data.data()), sizeof(int), size,
      f_data);
}

struct Result {
  long prefix_idx, suffix_idx, inner_start, inner_end;
  long prefix_sum, suffix_sum, inner_sum, total_sum;
};

const size_t BF_SIZE = 1048576;

int main(int argc, char *argv[]) {
  int world_rank, world_size;
  FILE *f_data = fopen("sandwich.bin", "r");
  vector<int> data;
  size_t domain_size = read_domain_size(f_data),
         subdomain_start, subdomain_size;
  Result result;
  size_t curr_start;
  long max_ending_here, acc;
  long i, j;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  decompose_domain(domain_size, world_rank, world_size,
      subdomain_start, subdomain_size);

  result.prefix_sum = 0;
  result.prefix_idx = subdomain_start;
  acc = 0;

  result.inner_sum = 0;
  result.inner_start = result.inner_end = subdomain_start;
  max_ending_here = 0;
  curr_start = 0;
  for (i = 0; i < subdomain_size; i += j) {
    read_elements(f_data, data, subdomain_start + i,
        min(subdomain_size - i, BF_SIZE));

    for (j = 0; j < data.size(); ++j) {
      acc += data[j];
      max_ending_here += data[j];
      if (max_ending_here < 0) {
        max_ending_here = 0;
        curr_start = i + j + 1;
      } else if (max_ending_here > result.inner_sum) {
        result.inner_sum = max_ending_here;
        result.inner_start = subdomain_start + curr_start;
        result.inner_end = subdomain_start + i + j + 1;
      }
      if (acc > result.prefix_sum) {
        result.prefix_sum = acc;
        result.prefix_idx = subdomain_start + i + j + 1;
      }
    }
  }
  result.total_sum = acc;

  acc = 0;
  result.suffix_sum = 0;
  result.suffix_idx = subdomain_start + subdomain_size;
  long end_position = result.suffix_idx;
  for (i = 0; i < subdomain_size; i += data.size()) {
    size_t size = min(subdomain_size - i, BF_SIZE);
    read_elements(f_data, data, end_position - size, size);
    for (j = data.size() - 1; j >= 0; --j) {
      acc += data[j];
      --end_position;
      if (acc > result.suffix_sum) {
        result.suffix_sum = acc;
        result.suffix_idx = end_position;
      }
    }
  }
  fclose(f_data);

  if (world_rank == 0) {
    vector<Result> results(world_size);
    results[0] = result;
    puts("Partial results");
    printf("[%lu, %lu[ -> %ld\n", result.inner_start, result.inner_end, result.inner_sum);
    for (i = 1; i < world_size; ++i) {
      MPI_Status status;
      MPI_Recv(reinterpret_cast<void *>(&result), sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, 0,
          MPI_COMM_WORLD, &status);
      results[status.MPI_SOURCE] = result;
      printf("[%lu, %lu[ -> %ld\n", result.inner_start, result.inner_end, result.inner_sum);
    }
    size_t best_start, best_end;
    long best_sum = -1;
    for (i = 0; i < results.size(); ++i) {
      if (best_sum < results[i].inner_sum) {
        best_sum = results[i].inner_sum;
        best_start = results[i].inner_start;
        best_end = results[i].inner_end;
      }
    }
    for (i = 0; i < results.size(); ++i) {
      long curr_sum = results[i].suffix_sum;
      for (j = i + 1; j < results.size(); ++j) {
        if (curr_sum + results[j].prefix_sum > best_sum) {
          best_sum = curr_sum + results[j].prefix_sum;
          best_start = results[i].suffix_idx;
          best_end = results[j].prefix_idx;
        }
        curr_sum += results[j].total_sum;
      }
    }
    printf("\nBEST\n[%lu, %lu[ -> %ld\n", best_start, best_end, best_sum);
  } else {
    MPI_Send(reinterpret_cast<void *>(&result), sizeof(Result), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}

