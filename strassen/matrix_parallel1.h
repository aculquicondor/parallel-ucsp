#ifndef STRASSEN_MATRIX_PARALLEL_H_
#define STRASSEN_MATRIX_PARALLEL_H_

#include <vector>

#include <pthread.h>
#include <unistd.h>

#include "matrix.h"

template<typename M>
struct Args {
  const M *a, *b;
  M *ans;
  size_t begin, end;
};

template<typename M>
void *parallel_function_sum(void *v_arg) {
  Args<M> *args = (Args<M> *)v_arg;
  for (size_t i = args->begin; i < args->end; ++i)
    for (size_t j = 0; j < args->a->size(); ++j)
      (*args->ans)(i, j) = (*args->a)(i, j) + (*args->b)(i, j);
  return 0;
}
template<typename M>
void *parallel_function_dif(void *v_arg) {
  Args<M> *args = (Args<M> *)v_arg;
  for (size_t i = args->begin; i < args->end; ++i)
    for (size_t j = 0; j < args->a->size(); ++j)
      (*args->ans)(i, j) = (*args->a)(i, j) - (*args->b)(i, j);
  return 0;
}

template<typename T>
class RealMatrixParallel1;

template<typename T>
class ProxyMatrixParallel1;

template<typename T>
class MatrixParallel1 : public Matrix<T> {
 protected:
  static const size_t REQ_PARALLEL = 64;
  static long NO_PROCESSORS;

 public:
  MatrixParallel1(size_t n) : Matrix<T>(n) {
  }
  Matrix<T>* sum(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    if (this->n_ > REQ_PARALLEL) {
      pthread_t *threads = new pthread_t[NO_PROCESSORS];
      Args<Matrix<T>> *args = new Args<Matrix<T>>[NO_PROCESSORS];
      size_t step = this->n_ / NO_PROCESSORS, extra = this->n_ % NO_PROCESSORS;
      for (size_t pos = 0, i = 0; i < NO_PROCESSORS; i++) {
        size_t next = pos + step + (i < extra);
        args[i] = {&first, &second, &ans, pos, next};
        pthread_create(&threads[i], NULL, parallel_function_sum<Matrix<T>>, &args[i]);
        pos = next;
      }
      for (size_t i = 0; i < NO_PROCESSORS; ++i)
        pthread_join(threads[i], NULL);
      delete[] threads;
      delete[] args;
    } else {
      for (size_t i = 0; i < this->n_; ++i)
        for (size_t j = 0; j < this->n_; ++j)
          ans(i, j) = first(i, j) + second(i, j);
    }
    return this;
  }
  Matrix<T>* sum(Matrix<T> *p_other) {
    MatrixParallel1<T> *p_ans = new RealMatrixParallel1<T>(this->n_);
    p_ans->sum(this, p_other);
    return p_ans;
  }
  Matrix<T>* dif(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    if (this->n_ > REQ_PARALLEL) {
      pthread_t *threads = new pthread_t[NO_PROCESSORS];
      Args<Matrix<T>> *args = new Args<Matrix<T>>[NO_PROCESSORS];
      size_t step = this->n_ / NO_PROCESSORS, extra = this->n_ % NO_PROCESSORS;
      for (size_t pos = 0, i = 0; i < NO_PROCESSORS; i++) {
        size_t next = pos + step + (i < extra);
        args[i] = {&first, &second, &ans, pos, next};
        pthread_create(&threads[i], NULL, parallel_function_dif<Matrix<T>>, &args[i]);
        pos = next;
      }
      for (size_t i = 0; i < NO_PROCESSORS; ++i)
        pthread_join(threads[i], NULL);
      delete[] threads;
      delete[] args;
    } else {
      for (size_t i = 0; i < this->n_; ++i)
        for (size_t j = 0; j < this->n_; ++j)
          ans(i, j) = first(i, j) - second(i, j);
    }
    return this;
  }
  Matrix<T>* dif(Matrix<T> *p_other) {
    MatrixParallel1<T> *p_ans = new RealMatrixParallel1<T>(this->n_);
    p_ans->dif(this, p_other);
    return p_ans;
  }
  Matrix<T>* mul(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    if (this->n_ == 1) {
      ans(0, 0) = first(0, 0) * second(0, 0);
      return this;
    } else if (this->n_ <= Matrix<T>::REQ_RECURSION) {
      for (size_t i = 0; i < this->n_; ++i)
        for (size_t j = 0; j < this->n_; ++j) {
          for (size_t k = 0; k < this->n_; ++k)
            ans(i, j) += first(i, k) * second(k, j);
        }
      return this;
    }
    size_t n2 = this->n_ >> 1;
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
    Matrix<T> *tmp1 = new RealMatrixParallel1<T>(n2),
              *tmp2 = new RealMatrixParallel1<T>(n2),
              *m1 = new RealMatrixParallel1<T>(n2),
              *m2 = new RealMatrixParallel1<T>(n2),
              *m3 = new RealMatrixParallel1<T>(n2),
              *m4 = new RealMatrixParallel1<T>(n2),
              *m5 = new RealMatrixParallel1<T>(n2),
              *m6 = new RealMatrixParallel1<T>(n2),
              *m7 = new RealMatrixParallel1<T>(n2);
    tmp1->sum(a11, a22); tmp2->sum(b11, b22); m1->mul(tmp1, tmp2);
    tmp1->sum(a21, a22); m2->mul(tmp1, b11);
    tmp2->dif(b12, b22); m3->mul(a11, tmp2);
    tmp2->dif(b21, b11); m4->mul(a22, tmp2);
    tmp1->sum(a11, a12); m5->mul(tmp1, b22);
    tmp1->dif(a21, a11); tmp2->sum(b11, b12); m6->mul(tmp1, tmp2);
    tmp1->dif(a12, a22); tmp2->sum(b21, b22); m7->mul(tmp1, tmp2);
    delete a11; delete a12; delete a21; delete a22;
    delete b11; delete b12; delete b21; delete b22;
    tmp1->sum(m1, m4); tmp2->dif(m7, m5); c11->sum(tmp1, tmp2);
    tmp1->dif(m1, m2); tmp2->sum(m3, m6); c22->sum(tmp1, tmp2);
    c12->sum(m3, m5);
    c21->sum(m2, m4);
    delete tmp1; delete tmp2;
    delete m1; delete m2; delete m3; delete m4; delete m5; delete m6; delete m7;
    delete c11; delete c12; delete c21; delete c22;
    return this;
  }
  Matrix<T>* mul(Matrix<T> *p_other) {
    MatrixParallel1<T> *p_ans = new RealMatrixParallel1<T>(this->n_);
    p_ans->mul(this, p_other);
    return p_ans;
  }
};

template<typename T>
long MatrixParallel1<T>::NO_PROCESSORS = sysconf(_SC_NPROCESSORS_CONF);

template<typename T>
class RealMatrixParallel1 : public MatrixParallel1<T> {
  std::vector<T> data_;
 public:
  RealMatrixParallel1(size_t n) : MatrixParallel1<T>(n), data_(n*n) {
  }
  T operator()(size_t i, size_t j) const {
    return data_[this->n_*i+j];
  }
  T& operator()(size_t i, size_t j) {
    return data_[this->n_*i+j];
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrixParallel1<T>(this, n, i0, j0);
  }
};

template<typename T>
class ProxyMatrixParallel1 : public MatrixParallel1<T> {
  MatrixParallel1<T> *original_;
  size_t i0_, j0_;
 public:
  ProxyMatrixParallel1(MatrixParallel1<T> *original, size_t n, size_t i0,
                       size_t j0) :
      MatrixParallel1<T>(n), original_(original), i0_(i0), j0_(j0) {
  }
  T operator()(size_t i, size_t j) const {
    return (*original_)(i0_+i, j0_+j);
  }
  T& operator()(size_t i, size_t j) {
    return (*original_)(i0_+i, j0_+j);
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrixParallel1(original_, n, i0_+i0, j0_+j0);
  }
};

#endif //STRASSEN_MATRIX_PARALLEL_H_
