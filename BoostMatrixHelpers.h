#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;

/* refactored third party code, 
 Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
template<class T>
bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
{
	typedef permutation_matrix<std::size_t> pmatrix;

	matrix<T> copy(input);

	// create a permutation matrix for the LU-factorization
	pmatrix pm(copy.size1());

	// perform LU-factorization
	int res = lu_factorize(copy, pm);
	if (res != 0) {
		std::cout << "Matrix inversion failed" << std::endl;
		return false;
	}

	// create identity matrix of "inverse"
	inverse.assign(identity_matrix<T> (copy.size1()));

	// backsubstitute to get the inverse
	lu_substitute(copy, pm, inverse);
	std::cout << "Matrix inversion complete" << std::endl;
	return true;
}

template<class T>
bool InvertDiagonalMatrix(const matrix<T>& input, matrix<T>& inverse)
{
	int dimension = input.size1();
	for (int i = 0; i < dimension; ++i) {
		if (!input(i,i)) {
			std::cout << "ERROR< there's a zero in the diagonal" << std::endl;
			return false;
		}
		inverse(i,i) = 1 / input(i,i);
	}
	return true;
}

template<typename T>
T l2Norm(const matrix<T>& m1, const matrix<T>& m2) {
	T ret = 0;
	for (int i = 0; i < m1.size1(); ++i) {
		for (int j = 0; j < m1.size2(); ++j) {
			ret += std::pow(m1(i, j) - m2(i, j), 2);
		}
	}
	return std::sqrt(ret);
}

template<typename T> 
void printVector(const std::vector<T>& v, std::ostream& out=std::cout, char delim=',') {
  if (v.empty()) return;
  for (auto it = v.cbegin(); it != v.cend() - 1; ++it) {
  	out << *it << delim;
  }
  out << v.back() << std::endl;
}

template<typename T>
void printMatrix(const matrix<T>& m, std::ostream& out=std::cout, char delim=',') {
	for (int i = 0; i < m.size1(); ++i) {
		for (int j = 0; j < m.size2(); ++j) {
			out << m(i,j) << delim;
		}
		out << std::endl;
	}
}

