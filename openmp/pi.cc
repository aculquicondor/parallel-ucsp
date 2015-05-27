#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <omp.h>

using namespace std;

int main(int argc, const char *argv[]) {
  int thread_count = strtol(argv[1], NULL, 10);
  int n;
  cin >> n;
  long double pi = 0, factor;
# pragma omp parallel for reduction(+: pi) private(factor)
  for (int i = 0; i < n; ++i) {
    factor = (i & 1) ? -1 : 1;
    pi += factor / (2 * i + 1);
  }
  pi *= 4;
  cout << fixed << setprecision(20) << pi << endl;
  return 0;
}
