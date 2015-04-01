#ifndef STRASSEN_MATRIX_H_
#define STRASSEN_MATRIX_H_

#include <vector>

template<typename T>
class Matrix {
	size_t n_;
	std::vector<T> data_;
 public:
  Matrix(size_t n) : n_(n), data_(n*n) {
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
	Matrix<T> operator+(const Matrix &other) const {
		const Matrix<T> &self = *this;
		Matrix<T> ans(n_);
		for (size_t i = 0; i < n_; ++i)
			for (size_t j = 0; j < n_; ++j)
				ans(i, j) = self(i, j) + other(i, j);
		return ans;
	}
	Matrix<T> operator-(const Matrix &other) const {
		const Matrix<T> &self = *this;
		Matrix<T> ans(n_);
		for (size_t i = 0; i < n_; ++i)
			for (size_t j = 0; j < n_; ++j)
				ans(i, j) = self(i, j) - other(i, j);
		return ans;
	}
	Matrix<T> operator*(const Matrix &other) const {
		const Matrix<T> &self = *this;
		Matrix<T> ans(n_);
		if (n_ == 1) {
			ans(0, 0) = self(0, 0) * other(0, 0);
			return ans;
		}
		size_t n2 = n_ >> 1;
		Matrix a11 = self.extract(n2, 0, 0),
					 a12 = self.extract(n2, 0, n2),
					 a21 = self.extract(n2, n2, 0),
					 a22 = self.extract(n2, n2, n2);
		Matrix b11 = other.extract(n2, 0, 0),
					 b12 = other.extract(n2, 0, n2),
					 b21 = other.extract(n2, n2, 0),
					 b22 = other.extract(n2, n2, n2);
		Matrix m1 = (a11 + a22) * (b11 + b22),
					 m2 = (a21 + a22) * b11,
					 m3 = a11 * (b12 - b22),
					 m4 = a22 * (b21 - b11),
					 m5 = (a11 + a12) * b22,
					 m6 = (a21 - a11) * (b11 + b12),
					 m7 = (a12 - a22) * (b21 + b22);
		return ans
						.add(m1+m4-m5+m7, 0, 0)
						.add(m3+m5, 0, n2)
						.add(m2+m4, n2, 0)
						.add(m1-m2+m3+m6, n2, n2);
	}
	Matrix<T> extract(size_t sz, size_t i0, size_t j0) const {
		const Matrix<T> &self = *this;
		Matrix<T> ans(sz);
		for (size_t i = 0; i < sz; ++i)
			for (size_t j = 0; j < sz; ++j)
				ans(i, j) = self(i0+i, j0+j);
		return ans;
	}
	Matrix<T> &add(const Matrix &other, size_t i0, size_t j0) {
		Matrix<T> &self = *this;
		for (size_t i = 0; i < other.n_; ++i)
			for (size_t j = 0; j < other.n_; ++j)
				self(i0+i, j0+j) = other(i, j);
		return self;
	}
};

#endif //STRASSEN_MATRIX_H_
