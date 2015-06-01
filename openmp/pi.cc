#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <omp.h>

using namespace std;

int thread_count;

long double pi_approximation(int iter) {
  long double accum = 0, factor;
# pragma omp parallel for reduction(+: accum) private(factor) \
    num_threads(thread_count)
  for (int i = 0; i < iter; ++i) {
    factor = (i & 1) ? -1 : 1;
    accum += factor / (2 * i + 1);
  }
  return accum * 4;
}

int main(int argc, const char *argv[]) {
  thread_count = strtol(argv[1], NULL, 10);
  int n;
  cin >> n;

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();
  long double pi = pi_approximation(n);
  end = chrono::system_clock::now();
  double ms = chrono::duration_cast<chrono::microseconds>
      (end-start).count() / 1000.;

  cout << fixed << setprecision(20);
  cout << "Pi: " << pi << endl;
  cout << "Time: " << ms << " ms" << endl;
  return 0;
}
