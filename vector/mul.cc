#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>

using namespace std;

template <typename T>
void vector_prod(T *__restrict__ r, const T *__restrict__ a,
    const T *__restrict__ b, size_t sz) {
  __builtin_assume_aligned(a, 64);
  __builtin_assume_aligned(b, 64);
  __builtin_assume_aligned(r, 64);
  for (int i = 0; i < sz; ++i)
    r[i] = a[i] * b[i];
}

template <typename T>
T vector_reduce(const T *__restrict__ a, size_t sz) {
  __builtin_assume_aligned(a, 64);
  T r = 0;
  for (int i = 0; i < sz; ++i)
    r += a[i];
  return r;
}

template<typename T, size_t sz>
class Vector {
  T data[sz] __attribute__((aligned(64)));
 public:
  Vector() {
    memset(data, 0, sizeof data);
  }
  Vector(initializer_list<T> l) : Vector() {
    size_t limit = min(l.size(), sz);
    typename initializer_list<T>::iterator it = l.begin();
    for (size_t i = 0; i < limit; ++i, ++it)
      data[i] = *it;
  }
  Vector operator*(const Vector<T, sz> & other) const {
    Vector<T, sz> res;
    vector_prod<T>(res.data, data, other.data, sz);
    return res;
  }
  T reduce() const {
    return vector_reduce<T>(data, sz);
  }
  const T& operator[](size_t i) const {
    return data[i];
  }
  T& operator[](size_t i) {
    return data[i];
  }
};

const size_t sz = 400000;

int main() {
  Vector<float, sz> a;
  Vector<float, sz> b;
  Vector<float, sz> c;
  float r;
  for (size_t i = 0; i < sz; ++i) {
    a[i] = (float)rand() / RAND_MAX;
    b[i] = (float)rand() / RAND_MAX;
  }


  long us = 1000000;
  chrono::time_point<chrono::system_clock> start, end;

  for (int i = 0; i < 1000; ++i) {
    start = chrono::system_clock::now();
    c = a * b;
    end = chrono::system_clock::now();
    us = min(us, (long)
        chrono::duration_cast<chrono::microseconds>(end-start).count());
  }

  r = c.reduce();
  cout << us << endl;
  cout << r << endl;
  return 0;
}

