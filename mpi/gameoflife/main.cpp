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
  memset(data[0], 0, size[1]);
  memset(data[local_height+1], 0, size[1]);
  srand(time(NULL) * w_rank);
  for (size_t i = 1; i <= local_height; ++i)
    for (size_t j = 0; j < size[1]; ++j)
      data[i][j] = (rand() & 1) ? 0 : 255;
}

void fill_img(CImage &image, byte **data, size_t local_height, int w_rank) {
  size_t first_pos = w_rank * local_height * image.width();
  memcpy(image.data() + first_pos, data[0], local_height * image.width());
}

void send_recv(int w_rank, int w_size, size_t local_height, size_t size[],
               byte **data) {
  int partner;
  if (w_rank & 1) {
    partner = w_rank ? w_rank - 1 : MPI_PROC_NULL;
    MPI_Sendrecv(data[1], size[1], MPI_BYTE, partner, 1,
                 data[0], size[1], MPI_BYTE, partner, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partner = w_rank < w_size - 1 ? w_rank + 1 : MPI_PROC_NULL;
    MPI_Sendrecv(data[local_height], size[1], MPI_BYTE, partner, 1,
                 data[local_height+1], size[1], MPI_BYTE, partner, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  } else {
    partner = w_rank < w_size - 1 ? w_rank + 1 : MPI_PROC_NULL;
    MPI_Sendrecv(data[local_height], size[1], MPI_BYTE, partner, 1,
                 data[local_height+1], size[1], MPI_BYTE, partner, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partner = w_rank ? w_rank - 1 : MPI_PROC_NULL;
    MPI_Sendrecv(data[1], size[1], MPI_BYTE, partner, 1,
                 data[0], size[1], MPI_BYTE, partner, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1},
    dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

void recalculate(size_t size[], size_t local_height, byte **data) {
  byte **tmp = new byte*[local_height];
  tmp[0] = new byte[local_height * size[1]];
  for (int i = 1; i < local_height; ++i)
    tmp[i] = tmp[i-1] + size[1];
  int q, ii, jj;
  for (int i = 0; i < local_height; ++i)
    for (int j = 0; j < size[1]; ++j) {
      q = 0;
      for (int k = 0; k < 8; ++k) {
        ii = i + dx[k];
        jj = j + dy[k];
        if (jj >= 0 and jj < size[1])
          q += data[ii+1][jj] != 0;
      }
      tmp[i][j] = (data[i+1][j] and (q == 2 or q == 3)) or
                      (not data[i+1][j] and (q == 3)) ? 255 : 0;
    }
  memcpy(data[1], tmp[0], local_height * size[1]);
  delete tmp[0];
  delete tmp;
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
    image = new CImage(size[1], size[0], 1, 1);
    main_disp = new CImgDisplay(size[1] * 2, size[0] * 2, "Game of Life");
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
      usleep(200000);
    } else {
      MPI_Send(data[1], local_height * size[1], MPI_BYTE, 0, 0,
               MPI_COMM_WORLD);
    }
    send_recv(w_rank, w_size, local_height, size, data);
    recalculate(size, local_height, data);
    if (w_rank == 0)
      window_closed = main_disp->is_closed();
    MPI_Bcast(&window_closed, 1, MPI_BYTE, 0, MPI_COMM_WORLD);
  } while (!window_closed);

  delete data[0];
  delete data;

  if (w_rank == 0) {
    delete msg[0];
    delete msg;
    delete image;
    delete main_disp;
  }

  MPI_Finalize();
  return 0;
}