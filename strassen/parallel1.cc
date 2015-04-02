#include <iostream>

#include "matrix_parallel1.h"

using namespace std;

int main() {
  ios::sync_with_stdio(0);
  size_t n;
  cin >> n;
  MatrixParallel1<int> *p_a = new RealMatrixParallel1<int>(n), *p_b = new RealMatrixParallel1<int>(n);
  MatrixParallel1<int> &a = *p_a, &b = *p_b;
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < n; ++j)
      cin >> a(i, j);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < n; ++j)
      cin >> b(i, j);
  size_t n2 = n >> 1;
  MatrixParallel1<int> *p_c = a.mul(&b); //a.extract(n2, 0, n2);
  MatrixParallel1<int> &c = *p_c;
  for (size_t i = 0; i < c.size(); ++i) {
    for (size_t j = 0; j < c.size(); ++j)
      cout << c(i, j) << ' ';
    cout << endl;
  }
  delete p_a, p_b, p_c;
  return 0;
}
