#ifndef STRASSEN_MATRIX_PARALLEL_H_
#define STRASSEN_MATRIX_PARALLEL_H_

#include <vector>
#include <functional>

#include <pthread.h>
#include <unistd.h>

template<typename M>
struct Args {
	typedef typename M::Type T;
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
void *parallel_function_diff(void *v_arg) {
	Args<M> *args = (Args<M> *)v_arg;
	for (size_t i = args->begin; i < args->end; ++i)
		for (size_t j = 0; j < args->a->size(); ++j)
			(*args->ans)(i, j) = (*args->a)(i, j) - (*args->b)(i, j);
	return 0;
}

template<typename T>
class MatrixParallel1 {
	size_t n_;
	std::vector<T> data_;
	static const int REQ_PARALLEL = 256;
 public:
	typedef T Type;
  MatrixParallel1(size_t n) : n_(n), data_(n*n) {
	}
  T operator()(size_t i, size_t j) const {
		return data_[n_*i+j];
	}
	T& operator()(size_t i, size_t j) {
		return data_[n_*i+j];
	}
	size_t size() const {
		return n_;
	}
	inline MatrixParallel1<T> operator+(const MatrixParallel1 &other) const {
		const MatrixParallel1<T> &self = *this;
		MatrixParallel1<T> ans(n_);
		if (n_ > REQ_PARALLEL) {
			long NO_PROCESSORS = sysconf(_SC_NPROCESSORS_CONF);
			pthread_t *threads = new pthread_t[NO_PROCESSORS];
			Args<MatrixParallel1<T>> *args = new Args<MatrixParallel1<T>>[NO_PROCESSORS];
			size_t step = n_ / NO_PROCESSORS, extra = n_ % NO_PROCESSORS;
			for (size_t pos = 0, i = 0; i < NO_PROCESSORS; i++) {
				size_t next = pos + step + (i < extra);
				args[i] = {&self, &other, &ans, pos, next};
				pthread_create(&threads[i], NULL, parallel_function_sum<MatrixParallel1<T>>, &args[i]);
				pos = next;
			}
			for (size_t i = 0; i < NO_PROCESSORS; ++i)
				pthread_join(threads[i], NULL);
			delete[] threads;
			delete[] args;
		} else {
			for (size_t i = 0; i < n_; ++i)
				for (size_t j = 0; j < n_; ++j)
					ans(i, j) = self(i, j) + other(i, j);
		}
		return ans;
	}
	MatrixParallel1<T> operator-(const MatrixParallel1 &other) const {
		const MatrixParallel1<T> &self = *this;
		MatrixParallel1<T> ans(n_);
		if (n_ > REQ_PARALLEL) {
			long NO_PROCESSORS = sysconf(_SC_NPROCESSORS_CONF);
			pthread_t *threads = new pthread_t[NO_PROCESSORS];
			Args<MatrixParallel1<T>> *args = new Args<MatrixParallel1<T>>[NO_PROCESSORS];
			size_t step = n_ / NO_PROCESSORS, extra = n_ % NO_PROCESSORS;
			for (size_t pos = 0, i = 0; i < NO_PROCESSORS; i++) {
				size_t next = pos + step + (i < extra);
				args[i] = {&self, &other, &ans, pos, next};
				pthread_create(&threads[i], NULL, parallel_function_diff<MatrixParallel1<T>>, &args[i]);
				pos = next;
			}
			for (size_t i = 0; i < NO_PROCESSORS; ++i)
				pthread_join(threads[i], NULL);
			delete[] threads;
			delete[] args;
		} else {
			for (size_t i = 0; i < n_; ++i)
				for (size_t j = 0; j < n_; ++j)
					ans(i, j) = self(i, j) - other(i, j);
		}
		return ans;
	}
	MatrixParallel1<T> operator*(const MatrixParallel1 &other) const {
		const MatrixParallel1<T> &self = *this;
		MatrixParallel1<T> ans(n_);
		if (n_ == 1) {
			ans(0, 0) = self(0, 0) * other(0, 0);
			return ans;
		}
		size_t n2 = n_ >> 1;
		MatrixParallel1 a11 = self.Extract(n2, 0, 0),
					 a12 = self.Extract(n2, 0, n2),
					 a21 = self.Extract(n2, n2, 0),
					 a22 = self.Extract(n2, n2, n2);
		MatrixParallel1 b11 = other.Extract(n2, 0, 0),
					 b12 = other.Extract(n2, 0, n2),
					 b21 = other.Extract(n2, n2, 0),
					 b22 = other.Extract(n2, n2, n2);
		MatrixParallel1 m1 = (a11 + a22) * (b11 + b22),
					 m2 = (a21 + a22) * b11,
					 m3 = a11 * (b12 - b22),
					 m4 = a22 * (b21 - b11),
					 m5 = (a11 + a12) * b22,
					 m6 = (a21 - a11) * (b11 + b12),
					 m7 = (a12 - a22) * (b21 + b22);
		return ans
						.Add(m1+m4-m5+m7, 0, 0)
						.Add(m3+m5, 0, n2)
						.Add(m2+m4, n2, 0)
						.Add(m1-m2+m3+m6, n2, n2);
	}

 private:
	MatrixParallel1<T> Extract(size_t sz, size_t i0, size_t j0) const {
		const MatrixParallel1<T> &self = *this;
		MatrixParallel1<T> ans(sz);
		for (size_t i = 0; i < sz; ++i)
			for (size_t j = 0; j < sz; ++j)
				ans(i, j) = self(i0+i, j0+j);
		return ans;
	}
	MatrixParallel1<T> &Add(const MatrixParallel1 &other, size_t i0, size_t j0) {
		MatrixParallel1<T> &self = *this;
		for (size_t i = 0; i < other.n_; ++i)
			for (size_t j = 0; j < other.n_; ++j)
				self(i0+i, j0+j) = other(i, j);
		return self;
	}
};

#endif //STRASSEN_MATRIX_PARALLEL_H_
