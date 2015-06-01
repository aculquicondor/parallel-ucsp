#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace std;

double fun(double x) {
  return x * x;
}

int thread_count;

double trapezoid(double a, double b, int n) {
  double h = (b-a) / n;
  double sum = (fun(a) + fun(b)) / 2;

# pragma omp parallel for num_threads(thread_count) \
    reduction(+: sum)
  for (int i = 1; i < n; ++i)
    sum += fun(a + h * i);

  return sum * h;
}

int main(int argc, const char *argv[]) {
  double a, b;
  int n;
  thread_count = strtol(argv[1], NULL, 10);
  cin >> a >> b >> n;

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();
  double result = trapezoid(a, b, n);
  end = chrono::system_clock::now();
  double ms = chrono::duration_cast<chrono::microseconds>
      (end-start).count() / 1000.;

  cout << fixed << setprecision(2);
  cout << "Result: " << result << endl;
  cout << "Time: " << ms << " ms" << endl;
  return 0;
}

