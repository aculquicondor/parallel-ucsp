#ifndef STRASSEN_PARALLE2_H_
#define STRASSEN_PARALLE2_H_

#include <vector>

#include <pthread.h>
#include <unistd.h>

#include "matrix.h"

template<typename T>
class RealMatrixParallel2;

template<typename T>
class ProxyMatrixParallel2;

template<typename T>
class MatrixParallel2;

template<typename T>
struct Args2 {
  Matrix<T> **a, **b;
  MatrixParallel2<T> **m;
  const size_t n2;
};

template<typename T>
void *parallel_function1(void *v_arg) {
  Args2<T> *args = (Args2<T> *)v_arg;
  Matrix<T> **a = args->a, **b = args->b;
  MatrixParallel2<T> **m = args->m;
  MatrixParallel2<T> *tmp1 = new RealMatrixParallel2<T>(args->n2),
      *tmp2 = new RealMatrixParallel2<T>(args->n2);
  tmp1->sum(a[2], a[3]); m[1]->seq_mul(tmp1, b[0]);
  tmp1->dif(a[2], a[0]); tmp2->sum(b[0], b[1]); m[5]->seq_mul(tmp1, tmp2);
  delete tmp1;
  delete tmp2;
  return 0;
}
template<typename T>
void *parallel_function2(void *v_arg) {
  Args2<T> *args = (Args2<T> *)v_arg;
  Matrix<T> **a = args->a, **b = args->b;
  MatrixParallel2<T> **m = args->m;
  MatrixParallel2<T> *tmp1 = new RealMatrixParallel2<T>(args->n2),
      *tmp2 = new RealMatrixParallel2<T>(args->n2);
  tmp2->dif(b[1], b[3]); m[2]->seq_mul(a[0], tmp2);
  tmp1->dif(a[1], a[3]); tmp2->sum(b[2], b[3]); m[6]->seq_mul(tmp1, tmp2);
  delete tmp1;
  delete tmp2;
  return 0;
}
template<typename T>
void *parallel_function3(void *v_arg) {
  Args2<T> *args = (Args2<T> *)v_arg;
  Matrix<T> **a = args->a, **b = args->b;
  MatrixParallel2<T> **m = args->m;
  MatrixParallel2<T> *tmp1 = new RealMatrixParallel2<T>(args->n2),
      *tmp2 = new RealMatrixParallel2<T>(args->n2);
  tmp2->dif(b[2], b[0]); m[3]->seq_mul(a[3], tmp2);
  tmp1->sum(a[0], a[1]); m[4]->seq_mul(tmp1, b[3]);
  delete tmp1;
  delete tmp2;
  return 0;
}

template<typename T>
class MatrixParallel2 : public Matrix<T> {
 public:
  MatrixParallel2(size_t n) : Matrix<T>(n) {
  }
  Matrix<T>* sum(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    for (size_t i = 0; i < this->n_; ++i)
      for (size_t j = 0; j < this->n_; ++j)
        ans(i, j) = first(i, j) + second(i, j);
    return this;
  }
  Matrix<T>* sum(Matrix<T> *p_other) {
    MatrixParallel2<T> *p_ans = new RealMatrixParallel2<T>(this->n_);
    p_ans->sum(this, p_other);
    return p_ans;
  }
  Matrix<T>* dif(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    for (size_t i = 0; i < this->n_; ++i)
      for (size_t j = 0; j < this->n_; ++j)
        ans(i, j) = first(i, j) - second(i, j);
    return this;
  }
  Matrix<T>* dif(Matrix<T> *p_other) {
    MatrixParallel2<T> *p_ans = new RealMatrixParallel2<T>(this->n_);
    p_ans->dif(this, p_other);
    return p_ans;
  }
  Matrix<T>* mul(Matrix<T> *p_first, Matrix<T> *p_second) {
    Matrix<T> &ans = *this, &first = *p_first, &second = *p_second;
    if (this->n_ == 1) {
      ans(0, 0) = first(0, 0) * second(0, 0);
      return this;
    }
    size_t n2 = this->n_ >> 1;
    Matrix<T> *a[] = {
        first.extract(n2, 0, 0),
        first.extract(n2, 0, n2),
        first.extract(n2, n2, 0),
        first.extract(n2, n2, n2),
    };
    Matrix<T> *b[] = {
        second.extract(n2, 0, 0),
        second.extract(n2, 0, n2),
        second.extract(n2, n2, 0),
        second.extract(n2, n2, n2),
    };
    Matrix<T> *c[] = {
        ans.extract(n2, 0, 0),
        ans.extract(n2, 0, n2),
        ans.extract(n2, n2, 0),
        ans.extract(n2, n2, n2),
    };
    MatrixParallel2<T> *m[] = {
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
    };
    Args2<T> args{a, b, m, n2};
    pthread_t threads[3];
    pthread_create(&threads[0], NULL,
                   parallel_function1<T>, &args);
    pthread_create(&threads[1], NULL,
                   parallel_function2<T>, &args);
    pthread_create(&threads[2], NULL,
                   parallel_function3<T>, &args);
    Matrix<T> *tmp1 = new RealMatrixParallel2<T>(n2),
        *tmp2 = new RealMatrixParallel2<T>(n2);
    tmp1->sum(a[0], a[3]); tmp2->sum(b[0], b[3]); m[0]->seq_mul(tmp1, tmp2);
    for (size_t i = 0; i < 3; ++i)
      pthread_join(threads[i], NULL);

    tmp1->sum(m[0], m[3]); tmp2->dif(m[6], m[4]); c[0]->sum(tmp1, tmp2);
    tmp1->dif(m[0], m[1]); tmp2->sum(m[2], m[5]); c[3]->sum(tmp1, tmp2);
    c[1]->sum(m[2], m[4]);
    c[2]->sum(m[1], m[3]);
    for (size_t i = 0; i < 4; ++i) {
      delete a[i];
      delete b[i];
      delete c[i];
    }
    for (size_t i = 0; i < 7; ++i)
      delete m[i];
    delete tmp1; delete tmp2;
    return this;
  }
  Matrix<T>* seq_mul(Matrix<T> *p_first, Matrix<T> *p_second) {
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
    Matrix<T> *a[] = {
        first.extract(n2, 0, 0),
        first.extract(n2, 0, n2),
        first.extract(n2, n2, 0),
        first.extract(n2, n2, n2),
    };
    Matrix<T> *b[] = {
        second.extract(n2, 0, 0),
        second.extract(n2, 0, n2),
        second.extract(n2, n2, 0),
        second.extract(n2, n2, n2),
    };
    Matrix<T> *c[] = {
        ans.extract(n2, 0, 0),
        ans.extract(n2, 0, n2),
        ans.extract(n2, n2, 0),
        ans.extract(n2, n2, n2),
    };
    MatrixParallel2<T> *m[] = {
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
        new RealMatrixParallel2<T>(n2),
    };
    Matrix<T> *tmp1 = new RealMatrixParallel2<T>(n2),
        *tmp2 = new RealMatrixParallel2<T>(n2);
    tmp1->sum(a[0], a[3]); tmp2->sum(b[0], b[3]); m[0]->seq_mul(tmp1, tmp2);
    tmp1->sum(a[2], a[3]); m[1]->seq_mul(tmp1, b[0]);
    tmp2->dif(b[1], b[3]); m[2]->seq_mul(a[0], tmp2);
    tmp2->dif(b[2], b[0]); m[3]->seq_mul(a[3], tmp2);
    tmp1->sum(a[0], a[1]); m[4]->seq_mul(tmp1, b[3]);
    tmp1->dif(a[2], a[0]); tmp2->sum(b[0], b[1]); m[5]->seq_mul(tmp1, tmp2);
    tmp1->dif(a[1], a[3]); tmp2->sum(b[2], b[3]); m[6]->seq_mul(tmp1, tmp2);

    tmp1->sum(m[0], m[3]); tmp2->dif(m[6], m[4]); c[0]->sum(tmp1, tmp2);
    tmp1->dif(m[0], m[1]); tmp2->sum(m[2], m[5]); c[3]->sum(tmp1, tmp2);
    c[1]->sum(m[2], m[4]);
    c[2]->sum(m[1], m[3]);
    for (size_t i = 0; i < 4; ++i) {
      delete a[i];
      delete b[i];
      delete c[i];
    }
    for (size_t i = 0; i < 7; ++i)
      delete m[i];
    delete tmp1; delete tmp2;
    return this;
  }
  Matrix<T>* mul(Matrix<T> *p_other) {
    MatrixParallel2<T> *p_ans = new RealMatrixParallel2<T>(this->n_);
    p_ans->mul(this, p_other);
    return p_ans;
  }
};

template<typename T>
class RealMatrixParallel2 : public MatrixParallel2<T> {
  std::vector<T> data_;
public:
  RealMatrixParallel2(size_t n) : MatrixParallel2<T>(n), data_(n*n) {
  }
  T operator()(size_t i, size_t j) const {
    return data_[this->n_*i+j];
  }
  T& operator()(size_t i, size_t j) {
    return data_[this->n_*i+j];
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrixParallel2<T>(this, n, i0, j0);
  }
};

template<typename T>
class ProxyMatrixParallel2 : public MatrixParallel2<T> {
  MatrixParallel2<T> *original_;
  size_t i0_, j0_;
public:
  ProxyMatrixParallel2(MatrixParallel2<T> *original, size_t n, size_t i0,
                       size_t j0) :
      MatrixParallel2<T>(n), original_(original), i0_(i0), j0_(j0) {
  }
  T operator()(size_t i, size_t j) const {
    return (*original_)(i0_+i, j0_+j);
  }
  T& operator()(size_t i, size_t j) {
    return (*original_)(i0_+i, j0_+j);
  }
  Matrix<T>* extract(size_t n, size_t i0, size_t j0) {
    return new ProxyMatrixParallel2(original_, n, i0_+i0, j0_+j0);
  }
};

#endif //STRASSEN_PARALLE2_H_
