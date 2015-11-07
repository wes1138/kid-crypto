/* Decoder for toy cryptosystem. */

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

/* compute all subsets that sum to t; return the number of such subsets */
size_t allSSums(const vector<size_t>& X, size_t t,
		vector< vector< vector<size_t> > >& S) {
	/* store table of all solutions (potentially exponential) for
	 * each sum less than t. */
	S.resize(t+1);
	/* only one way to get 0 -- the empty set: */
	S[0].push_back(vector<size_t>());
	size_t partial = 0; /* upper bound to work backwards from */
	for (size_t i = 0; i < X.size(); i++) {
		partial += X[i];
		if (partial > t) partial = t;
		for (size_t j = partial; j >= X[i]; j--) {
			for (size_t k = 0; k < S[j-X[i]].size(); k++) {
				/* copy S[j-X[i]][k] and add i */
				S[j].push_back(S[j-X[i]][k]);
				S[j][S[j].size()-1].push_back(i);
			}
		}
	}
	return S[t].size();
}

int main(int argc, char *argv[])
{
	/* read input */
	size_t len; cin >> len;
	uint32_t* b = new uint32_t[len/16];
	uint32_t* a = new uint32_t[len/16];
	for (size_t i = 0; i < len/16; i++) {
		cin >> hex >> b[i];
	}
	/* convert input to a polynomial */
	GF2X f;
	/* NOTE: this assumes little-endian byte ordering. */
	GF2XFromBytes(f,reinterpret_cast<unsigned char*>(b),len/4);
	/* expected f to be degree 2n-2. */
	size_t n = deg(f);
	if (IsZero(f)) {
		fprintf(stderr,"too many solutions x_x\n");
		return 1;
	}
	/* factor it */
	vec_pair_GF2X_long factors;
	CanZass(factors,f);
	/* use subset sum to find products with degree n-1 */
	vector<size_t> D;
	vector<GF2X> F; /* expand factors for convenience */
	for (int i = 0; i < factors.length(); i++) {
		for (int j = 0; j < factors[i].b; j++) {
			D.push_back(deg(factors[i].a));
			F.push_back(factors[i].a);
		}
	#if DBGPRINT
		printf("      degree f[%i] = %lu\n",i,deg(factors[i].a));
		printf("multiplicity f[%i] = %lu\n",i,factors[i].b);
	#endif
	}
	/* Each factor must have degree < len.  Possibilities:
	 * n/2-r ... n/2+r, where r = (len-1) - n/2. */
	vector< vector< vector<size_t> > > S;
	size_t r = (len - 1) - n/2;
	if (r > n/2) r = n/2; /* take care of really small n. */
	allSSums(D,len-1+r,S);
	GF2X g,h;
	for (size_t k = n/2-r; k <= n/2+r; k++) {
		for (size_t i = 0; i < S[k].size(); i++) {
			set(g); set(h);
			for (size_t j = 0; j < S[k][i].size(); j++) {
				g *= F[S[k][i][j]];
			}
			h = f / g;
			BytesFromGF2X(reinterpret_cast<unsigned char*>(a),g,len/8);
			BytesFromGF2X(reinterpret_cast<unsigned char*>(a+len/32),h,len/8);
			for (size_t l = 0; l < len/16; l++)
				printf("%08x ",a[l]);
			printf("\n");
		}
	}
	delete [] b;
	delete [] a;
	return 0;
}
