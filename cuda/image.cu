#include <cstdio>
#include <cstdlib>
#include <chrono>
#define cimg_display 0
#include <cuda.h>
#include <CImg.h>

using namespace std;
using namespace cimg_library;

const int block_row = 32;

__global__ void img_filter_kernel(unsigned char *input, unsigned char *output,
                                  size_t r, size_t c) {
  int x = blockIdx.x * block_row + threadIdx.x,
      y = blockIdx.y * block_row + threadIdx.y,
      z = blockIdx.z;
  int base = r * c * z;
  int sum = 0, count = 0;
  for (int i = y - 1; i <= y + 1; ++i)
    if (i >= 0 && i < r)
      for (int j = x - 1; j <= x + 1; ++j)
        if (j >= 0 && j < c) {
          ++count;
          sum += input[base + c * i + j];
        }
  if (count)
    output[base + c * y + x] = sum / count;
}

typedef CImg<unsigned char> Image;

void img_filter(Image &img) {
  unsigned char *input_d, *output_d;
  size_t data_size = img.size() * sizeof(unsigned char);
  cudaMalloc((void **)&input_d, data_size);
  cudaMalloc((void **)&output_d, data_size);
  cudaMemcpy(input_d, img.data(), data_size, cudaMemcpyHostToDevice);
  size_t r = img.height(), c = img.width();
  dim3 grid_dim((c+block_row-1)/block_row, (r+block_row-1)/block_row,
                img.spectrum()), block_dim(block_row, block_row);
  img_filter_kernel<<<grid_dim, block_dim>>>(input_d, output_d, r, c);
  cudaMemcpy(img.data(), output_d, data_size, cudaMemcpyDeviceToHost);
  cudaFree(input_d);
  cudaFree(output_d);
}

int main(int argc, char *argv[]) {
  Image img("lena.jpg");

  chrono::time_point<chrono::system_clock> start, end;
  int ms;
  start = chrono::system_clock::now();

  img_filter(img);

  end = chrono::system_clock::now();
  ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
  printf("GPU: %d\n", ms);

  img.save("lena_filter.jpg");

  return 0;
}

