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
  size_t prefix_idx, suffix_idx, inner_start, inner_end;
  long prefix_sum, suffix_sum, inner_sum, total_sum;
  Result(size_t start=0, size_t size=0) {
    inner_sum = total_sum = suffix_sum = prefix_sum = 0;
    inner_start = inner_end = prefix_idx = start;
    suffix_idx = start + size;
  }
  Result &operator+=(const Result &b) {
    Result &r = *this;
    if (r.inner_sum < b.inner_sum) {
      r.inner_sum = b.inner_sum;
      r.inner_start = b.inner_start;
      r.inner_end = b.inner_end;
    }
    if (r.inner_sum < r.suffix_sum + b.prefix_sum) {
      r.inner_sum = r.suffix_sum + b.prefix_sum;
      r.inner_start = r.suffix_idx;
      r.inner_end = b.prefix_idx;
    }
    if (r.prefix_sum < r.total_sum + b.prefix_sum) {
      r.prefix_sum = r.total_sum + b.prefix_sum;
      r.prefix_idx = b.prefix_idx;
    }
    if (r.suffix_sum + b.total_sum > b.suffix_sum) {
      r.suffix_sum += b.total_sum;
    } else {
      r.suffix_sum = b.suffix_sum;
      r.suffix_idx = b.suffix_idx;
    }
    r.total_sum += b.total_sum;
    return r;
  }
};

Result solve_elements(const vector<int> &data, size_t start) {
  Result r(start, data.size());
  long max_ending_here = 0, acc = 0;
  size_t curr_start = start;
  for (size_t i = 0; i < data.size(); ++i) {
    acc += data[i];
    max_ending_here += data[i];
    if (max_ending_here < 0) {
      max_ending_here = 0;
      curr_start = start + i + 1;
    } else if (max_ending_here > r.inner_sum) {
      r.inner_sum = max_ending_here;
      r.inner_start = curr_start;
      r.inner_end = start + i + 1;
    }
    if (acc > r.prefix_sum) {
      r.prefix_sum = acc;
      r.prefix_idx = start + i + 1;
    }
  }
  r.total_sum = acc;
  acc = 0;
  for (long i = data.size()-1; i >= 0; --i) {
    acc += data[i];
    if (acc > r.suffix_sum) {
      r.suffix_sum = acc;
      r.suffix_idx = start + i;
    }
  }
  return r;
}

const size_t MAX_BF_SIZE = 1 << 15;

int main(int argc, char *argv[]) {
  int world_rank, world_size;
  FILE *f_data = fopen("sandwich.bin", "r");
  vector<int> data;
  size_t domain_size = read_domain_size(f_data),
         subdomain_start, subdomain_size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  decompose_domain(domain_size, world_rank, world_size,
      subdomain_start, subdomain_size);

  Result result(subdomain_start);
  for (size_t i = 0, size; i < subdomain_size; i += size) {
    size = min(subdomain_size - i, MAX_BF_SIZE);
    read_elements(f_data, data, subdomain_start + i, size);
    result += solve_elements(data, subdomain_start + i);
  }
  fclose(f_data);

  if (world_rank == 0) {
    vector<Result> results(world_size);
    results[0] = result;
    puts("Partial results");
    printf("[%lu, %lu[ -> %ld\n", result.inner_start, result.inner_end, result.inner_sum);
    for (size_t i = 1; i < world_size; ++i) {
      MPI_Status status;
      MPI_Recv(reinterpret_cast<void *>(&result), sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, 0,
          MPI_COMM_WORLD, &status);
      results[status.MPI_SOURCE] = result;
      printf("[%lu, %lu[ -> %ld\n", result.inner_start, result.inner_end, result.inner_sum);
    }
    result = Result();
    for (size_t i = 0; i < results.size(); ++i)
      result += results[i];
    printf("\nBEST\n[%lu, %lu[ -> %ld\n", result.inner_start, result.inner_end, result.inner_sum);
  } else {
    MPI_Send(reinterpret_cast<void *>(&result), sizeof(Result), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}

