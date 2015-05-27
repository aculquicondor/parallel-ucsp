#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <omp.h>

using namespace std;

int main(int argc, const char *argv[]) {
  int thread_count = strtol(argv[1], NULL, 10);
  int n, phase, i;
  cin >> n;
  vector<int> a(n);
  for (int i = 0; i < n; ++i)
    cin >> a[i];
# pragma omp parallel num_threads(thread_count) \
    default(none) shared(a, n) private(i, phase)
  for (phase = 0; phase < n; ++phase) {
    if (phase & 1) {
#     pragma omp for
      for (i = 1; i < n - 1; i += 2)
        if (a[i] > a[i+1])
          swap(a[i], a[i+1]);
    } else {
#     pragma omp for
      for (i = 1; i < n; i += 2)
        if (a[i-1] > a[i])
          swap(a[i-1], a[i]);
    }
  }
  for (int i = 0; i < n; ++i)
    cout << ' ' << a[i];
  cout << endl;
  return 0;
}

