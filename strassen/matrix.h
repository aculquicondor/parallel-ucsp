#ifndef STRASSEN_MATRIX_H_
#define STRASSEN_MATRIX_H_

#include <stddef.h>

template<typename T>
class Matrix {
 protected:
  size_t n_;
  static const size_t REQ_RECURSION = 64;
 public:
  typedef T Type;
  Matrix(size_t n) : n_(n) {
  }
  virtual T operator()(size_t i, size_t j) const = 0;
  virtual T& operator()(size_t i, size_t j) = 0;
  size_t size() const {
    return n_;
  }
  virtual Matrix<T>* sum(Matrix<T> *p_first, Matrix<T> *p_second) = 0;
  virtual Matrix<T>* sum(Matrix<T> *p_other) = 0;
  virtual Matrix<T>* dif(Matrix<T> *p_first, Matrix<T> *p_second) = 0;
  virtual Matrix<T>* dif(Matrix<T> *p_other) = 0;
  virtual Matrix<T>* mul(Matrix<T> *p_first, Matrix<T> *p_second) = 0;
  virtual Matrix<T>* mul(Matrix<T> *p_other) = 0;
  virtual Matrix<T>* extract(size_t n, size_t i0, size_t j0) = 0;
};

#endif //STRASSEN_MATRIX_H_
