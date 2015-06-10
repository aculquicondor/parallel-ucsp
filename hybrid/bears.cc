#include <mpi.h>
#include <iostream>

using namespace std;

int m_rank, m_count;
int n, m, q;
bool *data;
int *ans;
int local_n;

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
	ans = new int[local_n];
	if (m_rank == 0) {
		bool *all_data = new bool[n*m];
		int d;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j) {
				cin >> d;
				all_data[n*i+j] = d;
			}
		// completing
		for (int i = n; i < local_n * m_count; ++i)
			for (int j = 0; j < m; ++j)
				all_data[n*i+j] = 0;
		ans = new int[n];
		MPI_Scatter(all_data, local_n * m, MPI_BYTE, 
		            data, local_n * m, MPI_BYTE, 0, MPI_COMM_WORLD);
	} else {
		MPI_Scatter(all_data, local_n * m, MPI_BYTE,
		            data, local_n * m, MPI_BYTE, 0, MPI_COMM_WORLD);
	}
	n = local_n * m_count;
}

void doPre() {
	for (int i = 0; i < local_n; ++i) {

	}
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &m_count);

	doReading();
	doPre();

	MPI_Finalize();
	return 0;
}
