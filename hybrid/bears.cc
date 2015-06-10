#include <mpi.h>
#include <iostream>

using namespace std;

int m_rank, m_count;
int n, m, q;
bool *data;
int *l_ans, ans;
int local_n;
int n_threads;

void doReading() {
	int buff[3];
	if (m_rank == 0) {
		ios::sync_with_stdio(0);
		cin >> buff[0] >> buff[1] >> buff[2];
	}
	MPI_Bcast(buff, 3, MPI_INT, 0, MPI_COMM_WORLD);
	n = buff[0];
	m = buff[1];
	q = buff[2];
	local_n = (n + m_count - 1) / m_count;
	data = new bool[local_n * m];
	l_ans = new int[local_n];
  bool *all_data = nullptr;
	if (m_rank == 0) {
		all_data = new bool[n*m];
		int d;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j) {
				cin >> d;
				all_data[m*i+j] = d;
			}
		// completing
		for (int i = n; i < local_n * m_count; ++i)
			for (int j = 0; j < m; ++j)
				all_data[n*i+j] = 0;
		l_ans = new int[n];
		MPI_Scatter(all_data, local_n * m, MPI_BYTE, 
		            data, local_n * m, MPI_BYTE, 0, MPI_COMM_WORLD);
    delete []all_data;
	} else {
		MPI_Scatter(all_data, local_n * m, MPI_BYTE,
		            data, local_n * m, MPI_BYTE, 0, MPI_COMM_WORLD);
	}
	n = local_n * m_count;
}

int calcRow(int i) {
  int q = 0;
  l_ans[i] = 0;
  for (int j = 0; j < m; ++j) {
    if (data[i*m+j]) {
      if (++q > l_ans[i])
        l_ans[i] = q;
    } else {
      q = 0;
    }
  }
  return l_ans[i];
}

void doPre() {
  ans = 0;
# pragma omp parallel num_threads(n_threads)
  {
    int lmax = 0;
# pragma omp for
    for (int i = 0; i < local_n; ++i) {
      if (calcRow(i) > lmax)
        lmax = l_ans[i];
    }
    if (lmax > ans) {
# pragma omp critical
      if (lmax > ans)
        ans = lmax;
    }
  }
}

void allMax() {
  ans = 0;
# pragma omp parallel num_threads(n_threads)
  {
    int lmax = 0;
    for (int i = 0; i < local_n; ++i)
      lmax = max(lmax, l_ans[i]);
    if (lmax > ans) {
# pragma omp critical
      if (lmax > ans)
        ans = lmax;
    }
  }
}

void doUpdates() {
  int buff[2];
  int lower = local_n * m_rank;
  while (q--) {
    if (m_rank == 0) {
      cin >> buff[0] >> buff[1];
      --buff[0];
      --buff[1];
    }
    MPI_Bcast(buff, 2, MPI_INT, 0, MPI_COMM_WORLD);
    int i = buff[0] - lower, j = buff[1];
    if (i >= 0 and i < local_n) {
      data[i*m+j] = !data[i*m+j];
      calcRow(i);
      ans = 0;
      allMax();
    }
    int tans;
    MPI_Reduce(&ans, &tans, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    if (m_rank == 0)
      cout << tans << endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    exit(1);
  n_threads = atoi(argv[1]);
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &m_count);

	doReading();
	doPre();
  doUpdates();

	MPI_Finalize();
	return 0;
}

