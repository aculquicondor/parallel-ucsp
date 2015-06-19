#include <cstdio>
#include <cstdlib>
#include <cuda.h>
#include <chrono>

using namespace std;

__global__ void matrix_product_kernel(float *P, float *M, float *N,
                                      size_t width) {
  int i = threadIdx.x, j = threadIdx.y;
  float acc = 0;
  for (int k = 0; k < width; ++k)
    acc += M[i*width+k] * N[k*width+j];
  P[i*width+j] = acc;
}

void matrix_product_serial(float *P, float *M, float *N, size_t width) {
#pragma omp parallel for
  for (size_t i = 0; i < width; ++i)
    for (size_t j = 0; j < width; ++j) {
      float acc = 0;
      for (size_t k = 0; k < width; ++k)
        acc += M[i*width+k] * N[k*width+j];
      P[i*width+j] = acc;
    }
}

void matrix_product(float *P, float *M, float *N, size_t width) {
  float *Pd, *Md, *Nd;
  size_t dsize = width * width * sizeof(float);
  cudaMalloc((void **)&Md, dsize);
  cudaMemcpy(Md, M, dsize, cudaMemcpyHostToDevice);
  cudaMalloc((void **)&Nd, dsize);
  cudaMemcpy(Nd, N, dsize, cudaMemcpyHostToDevice);
  cudaMalloc((void **)&Pd, dsize);

  dim3 dim_block(width, width);
  dim3 dim_grid(1, 1);
  matrix_product_kernel<<<dim_grid, dim_block>>>(Pd, Md, Nd, width);

  cudaMemcpy(P, Pd, dsize, cudaMemcpyDeviceToHost);

  cudaFree(Pd);
  cudaFree(Md);
  cudaFree(Nd);
}

void print(float *M, size_t width) {
  for (size_t i = 0; i < width; ++i) {
    for (size_t j = 0; j < width; ++j)
      printf("%.2f ", M[i*width+j]);
    puts("");
  }
}

int main(int argc, char *argv[]) {
  size_t width;

#ifdef NOIO
  width = 512;
#else
  scanf("%ld", &width);
#endif

  float *M = new float[width*width],
        *N = new float[width*width],
        *P = new float[width*width];
  for (int i = 0; i < width * width; ++i) {
    M[i] = ((float)rand() / RAND_MAX - 0.5) * 2;
    N[i] = ((float)rand() / RAND_MAX - 0.5) * 2;
  }

  chrono::time_point<chrono::system_clock> start, end;
  int ms;

  start = chrono::system_clock::now();
  matrix_product(P, M, N, width);
  end = chrono::system_clock::now();
  ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
  printf("GPU: %d\n", ms);

#ifndef NOIO
  print(M, width);
  puts("-------");
  print(N, width);
  puts("-------");
  print(P, width);
#endif

  start = chrono::system_clock::now();
  matrix_product_serial(P, M, N, width);
  end = chrono::system_clock::now();
  ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
  printf("CPU: %d\n", ms);

#ifndef NOIO
  print(M, width);
  puts("-------");
  print(N, width);
  puts("-------");
  print(P, width);
#endif

  delete[] M;
  delete[] N;
  delete[] P;
  return 0;
}

