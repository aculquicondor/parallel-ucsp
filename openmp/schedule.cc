#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace std;

double f(int i) {
  int j, start = i * (i+1) / 2, finish = start + i;
  double result = 0;
  for (j = start; j <= finish; ++j)
    result += sin(j);
  return result;
}

int thread_count;

double parallel_default(int n) {
  double sum = 0;
  int i;
# pragma omp parallel for num_threads(thread_count) \
    reduction(+: sum)
  for (i = 0; i <= n; ++i)
    sum += f(i);
  return sum;
}

double parallel_cyclic(int n) {
  double sum = 0;
  int i;
# pragma omp parallel for num_threads(thread_count) \
    reduction(+: sum) schedule(static, 1)
  for (i = 0; i <= n; ++i)
    sum += f(i);
  return sum;
}

int main(int argc, const char *argv[]) {
  double a, b;
  int n;
  thread_count = strtol(argv[1], NULL, 10);
  cin >> n;

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();
  double result1 = parallel_default(n);
  end = chrono::system_clock::now();
  double ms1 = chrono::duration_cast<chrono::microseconds>
      (end-start).count() / 1000.;

  start = chrono::system_clock::now();
  double result2 = parallel_cyclic(n);
  end = chrono::system_clock::now();
  double ms2 = chrono::duration_cast<chrono::microseconds>
      (end-start).count() / 1000.;

  cout << "Results: " << result1 << ' ' << result2 << endl;
  cout << "Time Default: " << ms1 << " ms" << endl;
  cout << "Time Cyclic: " << ms2 << " ms" << endl;
  return 0;
}

