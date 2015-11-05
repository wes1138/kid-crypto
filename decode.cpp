/* Decoder for toy cryptosystem sent to me by Jean Pena */
#include <cstdio>
#include <NTL/GF2XFactoring.h>
#include <vector>
using std::vector;
#include <stddef.h> /* size_t */
#include <algorithm>
using std::max_element;
#include <utility>
using std::pair;
#include <iostream>
using std::cin;

typedef pair<int,int> intp;

/* compute a subset S of X whch sums to t.  Return value is the size
 * of the subset; thus a return value of 0 indicates failure. */
size_t sssum(const vector<size_t>& X, size_t t, vector<size_t>& S)
{
	// size_t m = max_element(X.begin(),X.end());
	size_t n = X.size();
	/* now allocate table for results. */
	vector< vector<bool> > A(n,vector<bool>(t+1,false));
	for (size_t v = 0; v <= t; v++) {
		A[0][v] = (X[0] == v);
	}
	for (size_t i = 1; i < n; i++) {
		for (size_t v = 0; v <= t; v++) {
			A[i][v] = A[i-1][v] || (X[i] == v);
			if (v >= X[i] && A[i-1][v-X[i]]) {
				A[i][v] = true;
			}
		}
	}
	/* now reconstruct the solution. */
	if (!A[n-1][t]) return 0;
	size_t i = n-1;
	while (t!=0) {
		while (i>0 && A[i-1][t]) i--;
		S.push_back(i);
		t -= X[i];
	}
	return S.size();
}

int main(int argc, char *argv[]) {
	vector<size_t> X = {1,7,4,13,8};
	vector<size_t> S;
	size_t t;
	while (cin >> t) {
		S.clear();
		size_t r = sssum(X,t,S);
		if (r) {
			printf("solution of size %lu:\n{ ",r);
			for (size_t j = 0; j < r; j++) {
				printf("%lu ",S[j]);
			}
			printf("}\n");
		}
	}
	return 0;
}
