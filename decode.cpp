/* Decoder for toy cryptosystem.  Problem was sent to me by Jean Pena
 * (and was originally posed by Nintendo, I guess) */

#include <cstdio>
#include <NTL/GF2XFactoring.h>
using namespace NTL;
#include <vector>
using std::vector;
#include <stddef.h> /* size_t */
#include <iostream>
using std::cin;
using std::cout;
#include <iomanip>
using std::hex;
using std::setw;
using std::setfill;
#include <inttypes.h> /* uint32_t */

// #define DBGPRINT 1

/* compute a subset S of X whch sums to t.  Return value is the size
 * of the subset; thus a return value of 0 indicates failure. */
size_t sssum(const vector<size_t>& X, size_t t, vector<size_t>& S)
{
	size_t n = X.size();
	/* now allocate table for results. */
	vector< vector<bool> > A(n,vector<bool>(t+1,false));
	for (size_t v = 0; v <= t; v++) {
		A[0][v] = (X[0] == v);
	}
	for (size_t i = 1; i < n; i++) {
		for (size_t v = 0; v <= t; v++) {
			A[i][v] = A[i-1][v] || (X[i] == v) || (v>=X[i] && A[i-1][v-X[i]]);
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

int subsetSumTest() {
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

int main(int argc, char *argv[]) {
	// return subsetSumTest();
	/* read input */
	size_t n; cin >> n;
	uint32_t* b = new uint32_t[n/16];
	uint32_t* a = new uint32_t[n/16];
	for (size_t i = 0; i < n/16; i++) {
		cin >> hex >> b[i];
	}
	/* convert input to a polynomial */
	GF2X f;
	/* NOTE: this assumes little-endian byte ordering. */
	GF2XFromBytes(f,reinterpret_cast<unsigned char*>(b),n/4);
	if (IsZero(f)) {
		fprintf(stderr,"too many solutions x_x\n");
		return 1;
	}
	#if DBGPRINT
	cout << f << "\n";
	#endif
	/* factor it */
	vec_pair_GF2X_long factors;
	CanZass(factors,f);
	/* use subset sum to find products with degree n-1 */
	vector<size_t> D;
	for (int i = 0; i < factors.length(); i++) {
		for (int j = 0; j < factors[i].b; j++) {
			D.push_back(deg(factors[i].a));
		}
	#if DBGPRINT
		printf("      degree f[%i] = %lu\n",i,deg(factors[i].a));
		printf("multiplicity f[%i] = %lu\n",i,factors[i].b);
	#endif
	}
	vector<size_t> S;
	size_t r = sssum(D,n-1,S);
	if (!r) {
		fprintf(stderr,"no solution -_-\n");
		return 1;
	}
	GF2X g,h;
	set(g); set(h);
	for (size_t i = 0; i < S.size(); i++) {
		g *= factors[S[i]].a;
	}
	h = f / g;
	BytesFromGF2X(reinterpret_cast<unsigned char*>(a),g,n/8);
	BytesFromGF2X(reinterpret_cast<unsigned char*>(a+n/32),h,n/8);
	for (size_t i = 0; i < n/16; i++) {
		printf("%x ",a[i]);
	}
	printf("\n");
	delete [] b;
	delete [] a;
	return 0;
}
