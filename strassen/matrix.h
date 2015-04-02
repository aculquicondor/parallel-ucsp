#ifndef STRASSEN_MATRIX_H_
#define STRASSEN_MATRIX_H_

#include <vector>

template<typename T>
class RealMatrix;

template<typename T>
class ProxyMatrix;

template<typename T>
class Matrix {
 protected:
  size_t n_;
 public:
  Matrix(size_t n) : n_(n) {
  }
  virtual T operator()(size_t i, size_t j) const = 0;
  virtual T& operator()(size_t i, size_t j) = 0;
  size_t size() const {
    return n_;
  }
  Matrix<T>* sum(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    for (size_t i = 0; i < n_; ++i)
      for (size_t j = 0; j < n_; ++j)
        ans(i, j) = first(i, j) + second(i, j);
    return this;
  }
  Matrix<T>* sum(Matrix<T> *p_other) {
    Matrix<T> *p_ans = new RealMatrix<T>(n_);
    p_ans->sum(this, p_other);
    return p_ans;
  }
  Matrix<T>* dif(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    for (size_t i = 0; i < n_; ++i)
      for (size_t j = 0; j < n_; ++j)
        ans(i, j) = first(i, j) - second(i, j);
    return this;
  }
  Matrix<T>* dif(Matrix<T> *p_other) {
    Matrix<T> *p_ans = new RealMatrix<T>(n_);
    p_ans->dif(this, p_other);
    return p_ans;
  }
  Matrix<T>* mul(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    if (n_ == 1) {
      ans(0, 0) = first(0, 0) * second(0, 0);
      return this;
    }
    size_t n2 = n_ >> 1;
    Matrix<T> *a11 = first.extract(n2, 0, 0),
              *a12 = first.extract(n2, 0, n2),
              *a21 = first.extract(n2, n2, 0),
              *a22 = first.extract(n2, n2, n2),
              *b11 = second.extract(n2, 0, 0),
              *b12 = second.extract(n2, 0, n2),
              *b21 = second.extract(n2, n2, 0),
              *b22 = second.extract(n2, n2, n2),
              *c11 = ans.extract(n2, 0, 0),
              *c12 = ans.extract(n2, 0, n2),
              *c21 = ans.extract(n2, n2, 0),
              *c22 = ans.extract(n2, n2, n2);
    Matrix<T> *tmp1 = new RealMatrix<T>(n2),
              *tmp2 = new RealMatrix<T>(n2),
              *m1 = new RealMatrix<T>(n2),
              *m2 = new RealMatrix<T>(n2),
              *m3 = new RealMatrix<T>(n2),
              *m4 = new RealMatrix<T>(n2),
              *m5 = new RealMatrix<T>(n2),
              *m6 = new RealMatrix<T>(n2),
              *m7 = new RealMatrix<T>(n2);
    tmp1->sum(a11, a22); tmp2->sum(b11, b22); m1->mul(tmp1, tmp2);
    tmp1->sum(a21, a22); m2->mul(tmp1, b11);
    tmp2->dif(b12, b22); m3->mul(a11, tmp2);
    tmp2->dif(b21, b11); m4->mul(a22, tmp2);
    tmp1->sum(a11, a12); m5->mul(tmp1, b22);
    tmp1->dif(a21, a11); tmp2->sum(b11, b12); m6->mul(tmp1, tmp2);
    tmp1->dif(a12, a22); tmp2->sum(b21, b22); m7->mul(tmp1, tmp2);
    delete a11, a12, a21, a22, b11, b12, b21, b22;
    tmp1->sum(m1, m4); tmp2->dif(m7, m5); c11->sum(tmp1, tmp2);
    c12->sum(m3, m5);
    c21->sum(m2, m4);
    tmp1->dif(m1, m2); tmp2->sum(m3, m6); c22->sum(tmp1, tmp2);
    delete c11, c12, c21, c22, tmp1, tmp2;
    return this;
  }
  Matrix<T>* mul(Matrix<T> *p_other) {
    Matrix<T> *p_ans = new RealMatrix<T>(n_);
    p_ans->mul(this, p_other);
    return p_ans;
  }
  virtual Matrix<T>* extract(size_t n, size_t i0, size_t j0) = 0;
};

template<typename T>
class RealMatrix : public Matrix<T> {
  std::vector<T> data_;
 public:
  RealMatrix(size_t n) : Matrix<T>(n), data_(n*n) {
  }
  T operator()(size_t i, size_t j) const {
    return data_[this->n_*i+j];
  }
  T& operator()(size_t i, size_t j) {
    return data_[this->n_*i+j];
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrix<T>(this, n, i0, j0);
  }
};

template<typename T>
class ProxyMatrix : public Matrix<T> {
  Matrix<T> *original_;
  size_t i0_, j0_;
 public:
  ProxyMatrix(Matrix<T> *original, size_t n, size_t i0, size_t j0) :
      Matrix<T>(n), original_(original), i0_(i0), j0_(j0) {
  }
  T operator()(size_t i, size_t j) const {
    return (*original_)(i0_+i, j0_+j);
  }
  T& operator()(size_t i, size_t j) {
    return (*original_)(i0_+i, j0_+j);
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrix(original_, n, i0_+i0, j0_+j0);
  }
};

#endif //STRASSEN_MATRIX_H_
