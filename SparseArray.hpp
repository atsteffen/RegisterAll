#ifndef SPARSEARRAY_HPP
#define SPARSEARRAY_HPP

#include <vector>

class SparseArray
{
public:
	SparseArray(void){
		size = 0;
		std::vector<int> idx;
		std::vector<float> val;
	}
	//~SparseArray(void);
	SparseArray(int sz){
		size = sz;
		idx = std::vector<int>(sz,-1);
		val = std::vector<float>(sz,0.0);
	}
	SparseArray(const SparseArray& clone){
		size = clone.size;
		idx = std::vector<int>(clone.idx);
		val = std::vector<float>(clone.val);
	}
	SparseArray operator+(const SparseArray& sa) const {
		int i = 0;
		int j = 0;
		int k = 0;
		std::vector<int> resultidx(size+sa.size);
		std::vector<float> resultval(size+sa.size);
		while(true) {
			if (i<size && (j == sa.size || idx[i] < sa.idx[j])) {
				resultidx[k] = idx[i];
				resultval[k] = val[i];
				i++; k++;
			}
			else if (j<sa.size && (i == size || idx[i] > sa.idx[j])) {
				resultidx[k] = sa.idx[j];
				resultval[k] = sa.val[j];
				j++; k++;
			}
			else if (i<size && j<sa.size && idx[i] == sa.idx[j]) {
				resultidx[k] = idx[i];
				resultval[k] = val[i] + sa.val[j];
				i++; j++; k++;
			}
			else {
				break;
			}
		}
		SparseArray result = SparseArray(k);
		for (int m=0; m < k; m++){
			result.idx[m] = resultidx[m];
			result.val[m] = resultval[m];
		}

		return result;
	}
		SparseArray operator-(const SparseArray& sa) const {
		int i = 0;
		int j = 0;
		int k = 0;
		std::vector<int> resultidx(size+sa.size);
		std::vector<float> resultval(size+sa.size);
		while(true) {
			if (i<size && (j == sa.size || idx[i] < sa.idx[j])) {
				resultidx[k] = idx[i];
				resultval[k] = val[i];
				i++; k++;
			}
			else if (j<sa.size && (i == size || idx[i] > sa.idx[j])) {
				resultidx[k] = sa.idx[j];
				resultval[k] = -sa.val[j];
				j++; k++;
			}
			else if (i<size && j<sa.size && idx[i] == sa.idx[j]) {
				resultidx[k] = idx[i];
				resultval[k] = val[i] - sa.val[j];
				i++; j++; k++;
			}
			else {
				break;
			}
		}
		SparseArray result = SparseArray(k);
		for (int m=0; m < k; m++){
			result.idx[m] = resultidx[m];
			result.val[m] = resultval[m];
		}

		return result;
	}

	SparseArray operator/(const float divisor) const {

		std::vector<float> resultval(size);
		for (int i = 0; i < size; i++) {
			resultval[i] = val[i]/divisor;
		}
		SparseArray result = SparseArray(size);
		result.idx = idx;
		result.val = resultval;

		return result;
	}

	SparseArray operator*(const float mult) const {

		std::vector<float> resultval(size);
		for (int i = 0; i < size; i++) {
			resultval[i] = val[i]*mult;
		}
		SparseArray result = SparseArray(size);
		result.idx = idx;
		result.val = resultval;

		return result;
	}

	int size;
	std::vector<int> idx;
	std::vector<float> val;
};

#endif
