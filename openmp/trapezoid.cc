#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <cmath>

using namespace std;

double fun(double x) {
  return x * x;
}

int main(int argc, const char *argv[]) {
  double global_result = 0.0;
  double a, b, h;
  int n, thread_count;
  thread_count = strtol(argv[1], NULL, 10);
  cin >> a >> b >> n;
  global_result = (fun(a) + fun(b)) / 2;
  h = (b - a) / n;

# pragma omp parallel for num_threads(thread_count) \
    reduction(+: global_result)
  for (int i = 1; i < n; ++i)
    global_result += fun(a + h * i);

  global_result *= h;
  cout << global_result << endl;
  return 0;
}

