#include <iostream>
#include <chrono>

#include "matrix_sequential.h"
#include "matrix_parallel1.h"

using namespace std;

template<typename M>
void fill(M &a, typename M::Type *data) {
  size_t q = 0;
  for (size_t i = 0; i < a.size(); ++i)
    for (size_t j = 0; j < a.size(); ++j)
      a(i, j) = data[q++];
}

int main() {
  size_t n, sz;
  cin >> n;
  sz = n * n;
  int *data0 = new int[sz], *data1 = new int[sz];
  srand((unsigned int) time(NULL));
  for (size_t i = 0; i < sz; ++i)
    data0[i] = rand() % 100;
  for (size_t i = 0; i < sz; ++i)
    data1[i] = rand() % 100;

  Matrix<int> *res;
  chrono::time_point<chrono::system_clock> start, end;

  RealMatrixSequential<int> sa(n), sb(n);
  fill(sa, data0);
  fill(sb, data1);
  start = chrono::system_clock::now();
  res = sa.mul(&sb);
  end = chrono::system_clock::now();
  cout << "Sequential " << res->size() << ": " <<
      chrono::duration_cast<chrono::microseconds>(end-start).count() << endl;

  delete res;

  RealMatrixParallel1<int> p1a(n), p1b(n);
  fill(p1a, data0);
  fill(p1b, data1);
  start = chrono::system_clock::now();
  res = p1a.mul(&p1b);
  end = chrono::system_clock::now();
  cout << "Parallel1 " << res->size() << ": " <<
      chrono::duration_cast<chrono::microseconds>(end-start).count() << endl;

  delete res;

  return 0;
}