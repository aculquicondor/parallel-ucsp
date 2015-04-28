#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mpi.h>
#include <CImg.h>

using namespace std;
using namespace cimg_library;

typedef unsigned char byte;
typedef CImg<byte> CImage;

void initialize(int w_rank, int w_size, size_t size[],
                size_t &local_height, byte **&data) {
  local_height = size[0] / w_size;
  data = new byte*[local_height + 2];
  data[0] = new byte[(local_height + 2) * size[1]];
  for (size_t i = 1; i < local_height + 2; ++i)
    data[i] = data[i-1] + size[1];
  srand(time(NULL));
  for (size_t i = 1; i <= local_height; ++i)
    for (size_t j = 0; j < size[1]; ++j)
      data[i][j] = (rand() & 1) ? 0 : 255;
}

void fill_img(CImage &image, byte **data, size_t local_height, int w_rank) {
  size_t first_row = w_rank * local_height;
  for (int y = 0; y < local_height; ++y) {
    cimg_forX(image, x) image(x, y + first_row) = data[y][x];
  }
}

void recalculate(size_t size[], byte **data) {

}

int main(int argc, char *argv[]) {
  int w_rank, w_size;
  CImage *image;
  CImgDisplay *main_disp;
  size_t size[2], local_height;
  byte **data, **msg;
  bool window_closed;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &w_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &w_size);

  if (w_rank == 0) {
    scanf("%zd %zd", &size[0], &size[1]);
    size[0] -= size[0] % w_size;
    image = new CImage(size[0], size[1], 1, 1);
    main_disp = new CImgDisplay(*image, "Game of Life");
    main_disp->resize(size[1] * 2, size[0] * 2);
    main_disp->show();
  }

  MPI_Bcast(size, 2, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
  initialize(w_rank, w_size, size, local_height, data);

  if (w_rank == 0) {
    msg = new byte*[local_height];
    msg[0] = new byte[local_height * size[1]];
    for (size_t i = 1; i < local_height; ++i)
      msg[i] = msg[i-1] + size[1];
  }

  do {
    if (w_rank == 0) {
      fill_img(*image, &data[1], local_height, 0);
      for (int i = 1; i < w_size; ++i) {
        MPI_Recv(msg[0], local_height * size[1], MPI_BYTE, MPI_ANY_SOURCE, 0,
                 MPI_COMM_WORLD, &status);
        fill_img(*image, msg, local_height, status.MPI_SOURCE);
      }
      image->display(*main_disp);
      sleep(5);
    } else {
      MPI_Send(data[1], local_height * size[1], MPI_BYTE, 0, 0,
               MPI_COMM_WORLD);
    }

    recalculate(size, data);
    if (w_rank == 0)
      window_closed = main_disp->is_closed();
    MPI_Bcast(&window_closed, 1, MPI_BYTE, 0, MPI_COMM_WORLD);
  } while (!window_closed);

  if (w_rank == 0) {
    delete image;
    delete main_disp;
  }

  MPI_Finalize();
  return 0;
}