#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <omp.h>
#include <chrono>

using namespace std;

int thread_count;

void parallel_sort(vector<int> &a) {
  size_t phase, i, n = a.size();
  for (phase = 0; phase < n; ++phase) {
    if (phase & 1) {
#     pragma omp parallel for num_threads(thread_count) \
        shared(a, n) private(i, phase)
      for (i = 1; i < n - 1; i += 2)
        if (a[i] > a[i+1])
          swap(a[i], a[i+1]);
    } else {
#     pragma omp parallel for num_threads(thread_count) \
        shared(a, n) private(i, phase)
      for (i = 1; i < n; i += 2)
        if (a[i-1] > a[i])
          swap(a[i-1], a[i]);
    }
  }
}

int main(int argc, const char *argv[]) {
  thread_count = strtol(argv[1], NULL, 10);
  int n;
  cin >> n;
  vector<int> a(n);
  for (int i = 0; i < n; ++i)
    a[i] = rand() % 1000;

  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();
  parallel_sort(a);
  end = chrono::system_clock::now();
  double ms = chrono::duration_cast<chrono::microseconds>
      (end-start).count() / 1000.;

  cout << fixed << setprecision(2);
  cout << "Time: " << ms << " ms" << endl;

  return 0;
}

