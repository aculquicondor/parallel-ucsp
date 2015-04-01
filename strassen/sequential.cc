#include <iostream>

#include "matrix.h"

using namespace std;

int main() {
	ios::sync_with_stdio(0);
	int n;
	cin >> n;
	Matrix<int> a(n), b(n);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			cin >> a(i, j);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			cin >> b(i, j);
	Matrix<int> c = a * b;
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j)
			cout << c(i, j) << ' ';
		cout << endl;
	}
	return 0;
}
