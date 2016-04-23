#include "stdafx.h"


#include <complex>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>


typedef std::complex<double> complex_t;

void fft0_type0_dif(int n, int s, bool eo, complex_t* x, complex_t* y)
{
	const int m = n / 2;
	const double theta0 = 2 * M_PI / n;

	if (n == 1) { if (eo) for (int q = 0; q < s; q++) y[q] = x[q]; }
	else {
		cout << "---------------" << endl;


		for (int p = 0; p < m; p++) {
			const complex_t wp = complex_t(cos(p*theta0), -sin(p*theta0));
			for (int q = 0; q < s; q++) {
				int ix0 = q + s*(p + 0);
				int ix1 = q + s*(p + m);
				int iy0 = q + s*(2 * p + 0);
				int iy1 = q + s*(2 * p + 1);
				cout << "src(" << ix0 << "," << ix1 << ")\t";
				cout << "dst(" << iy0 << "," << iy1 << ")\t";
				cout << endl;



				const complex_t a = x[q + s*(p + 0)];
				const complex_t b = x[q + s*(p + m)];
				y[q + s*(2 * p + 0)] = a + b;
				y[q + s*(2 * p + 1)] = (a - b) * wp;
			}
		}
		fft0_type0_dif(n / 2, 2 * s, !eo, y, x);
	}
}

void fft_type0_dif(int n, complex_t* x)
{
	complex_t* y = new complex_t[n];
	fft0_type0_dif(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] /= n;
}

void ifft_type0_dif(int n, complex_t* x)
{
	for (int p = 0; p < n; p++) x[p] = conj(x[p]);
	complex_t* y = new complex_t[n];
	fft0_type0_dif(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] = conj(x[k]);
}

//-------------------------------------------------------------------
void fft0_type0_dit(int n, int s, bool eo, complex_t* x, complex_t* y)
{
	const int m = n / 2;
	const double theta0 = 2 * M_PI / n;

	if (n == 1) { if (eo) for (int q = 0; q < s; q++) x[q] = y[q]; }
	else {

		fft0_type0_dit(n / 2, 2 * s, !eo, y, x);

		cout << "---------------" << endl;
		for (int p = 0; p < m; p++) {
			const complex_t wp = complex_t(cos(p*theta0), -sin(p*theta0));
			cout << "n:" << n <<"\t";
			cout << "p:" << p << "\t";
			cout << endl;

			for (int q = 0; q < s; q++) {
				int ix0 = q + s*(2 * p + 0);
				int ix1 = q + s*(2 * p + 1);
				int iy0 = q + s*(p + 0);
				int iy1 = q + s*(p + m);
				cout << "src(" << ix0 << "," << ix1 << ")\t";
				cout << "dst(" << iy0 << "," << iy1 << ")\t";
				cout << endl;


				const complex_t a = y[q + s*(2 * p + 0)];
				const complex_t b = y[q + s*(2 * p + 1)] * wp;
				x[q + s*(p + 0)] = a + b;
				x[q + s*(p + m)] = a - b;
			}
		}
	}
}

void fft_type0_dit(int n, complex_t* x)
{
	complex_t* y = new complex_t[n];
	fft0_type0_dit(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] /= n;
}

void ifft_type0_dit(int n, complex_t* x)
{
	for (int p = 0; p < n; p++) x[p] = conj(x[p]);
	complex_t* y = new complex_t[n];
	fft0_type0_dit(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] = conj(x[k]);
}

//-------------------------------------------------------
void fft0_type1_dif(int n, int s, bool eo, complex_t* x, complex_t* y)
{
	const int m = n / 2;
	const double theta0 = M_PI / s;

	if (n == 1) { if (eo) for (int q = 0; q < s; q++) y[q] = x[q]; }
	else {
		cout << "---------------" << endl;
		for (int p = 0; p < m; p++) {
			for (int q = 0; q < s; q++) {
				int ix0 = q + s*(p + 0);
				int ix1 = q + s*(p + m);
				int iy0 = q + s*(2 * p + 0);
				int iy1 = q + s*(2 * p + 1);
				cout << "src(" << ix0 << "," << ix1 << ")\t";
				cout << "dst(" << iy0 << "," << iy1 << ")\t";
				cout << endl;


				const complex_t wq = complex_t(cos(q*theta0), -sin(q*theta0));
				const complex_t a = x[q + s*(p + 0)];
				const complex_t b = x[q + s*(p + m)] * wq;
				y[q + s*(2 * p + 0)] = a + b;
				y[q + s*(2 * p + 1)] = a - b;
			}
		}
		fft0_type1_dif(n / 2, 2 * s, !eo, y, x);
	}
}

void fft_type1_dif(int n, complex_t* x)
{
	complex_t* y = new complex_t[n];
	fft0_type1_dif(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] /= n;
}

void ifft_type1_dif(int n, complex_t* x)
{
	for (int p = 0; p < n; p++) x[p] = conj(x[p]);
	complex_t* y = new complex_t[n];
	fft0_type1_dif(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] = conj(x[k]);
}


//------------------------------------------------------------------------
void fft0_type1_dit(int n, int s, bool eo, complex_t* x, complex_t* y)
{
	const int m = n / 2;
	const double theta0 = M_PI / s;

	if (n == 1) { if (eo) for (int q = 0; q < s; q++) x[q] = y[q]; }
	else {
		fft0_type1_dit(n / 2, 2 * s, !eo, y, x);
		cout << "---------------" << endl;
		for (int p = 0; p < m; p++) {
			for (int q = 0; q < s; q++) {
				int ix0 = q + s*(2 * p + 0);
				int ix1 = q + s*(2 * p + 1);
				int iy0 = q + s*(p + 0);
				int iy1 = q + s*(p + m);
				cout << "src(" << ix0 << "," << ix1 << ")\t";
				cout << "dst(" << iy0 << "," << iy1 << ")\t";
				cout << endl;

				const complex_t wq = complex_t(cos(q*theta0), -sin(q*theta0));
				const complex_t a = y[q + s*(2 * p + 0)];
				const complex_t b = y[q + s*(2 * p + 1)];
				x[q + s*(p + 0)] = a + b;
				x[q + s*(p + m)] = (a - b)*wq;
			}
		}
	}
}

void fft_type1_dit(int n, complex_t* x)
{
	complex_t* y = new complex_t[n];
	fft0_type1_dit(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] /= n;
}

void ifft_type1_dit(int n, complex_t* x)
{
	for (int p = 0; p < n; p++) x[p] = conj(x[p]);
	complex_t* y = new complex_t[n];
	fft0_type1_dit(n, 1, 0, x, y);
	delete[] y;
	for (int k = 0; k < n; k++) x[k] = conj(x[k]);
}


//-----------------------------------------------------------------------


//---------------------------------------------------------------------------
//value is power of 2
static bool IsPow2(unsigned int x){
	return (((x)&(x - 1)) == 0);
}



static unsigned int insertZeroBits(
	const unsigned int src,		//src
	const int idx,				//‘}“ü‚·‚éˆÊ’u
	const int num				//‘}“ü‚·‚é”
	)
{
	unsigned int ret = src << num;
	ret &= ~((1 << (idx + num)) - 1);
	ret |= src & ((1 << (idx)) - 1);
	return ret;
}

//http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
 
static unsigned int bitSwap(
	const unsigned int b,		 // bits to swap reside in b
	const int i,	// positions of bit sequences to swap
	const int j,	//
	const int n		// number of consecutive bits in each sequence
	)
{
	unsigned int x = ((b >> i) ^ (b >> j)) & ((1U << n) - 1); // XOR temporary
	return  b ^ ((x << i) | (x << j));
}

static unsigned int modq(unsigned int x, unsigned int q) { return  x& ((1 << q) - 1); }
static unsigned int divq(unsigned int x, unsigned int q) { return  x >> q; }



// FFT Stockham,DIT
void fft_dit_Stockham_radix2_type0(const Mat& src, Mat &dst){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);
	int N = src.cols;
	CV_Assert(IsPow2(N));

	enum { _re_ = 0, _im_ = 1 };


	Mat buf[2];
	buf[0] = src.clone();
	buf[1] = Mat(src.size(), src.type());


	vector<vec2> w(N / 2);


	// --- twidle ----
	for (int n = 0; n < N / 2; n++){
		float jw = (float)(-2 * M_PI * n / N);
		w[n][_re_] = cos(jw);
		w[n][_im_] = sin(jw);
	}


	int Q = 0;
	while ((1 << Q) < N){ Q++; }

	/* FFT‚ÌŒvŽZ */

	// --- FFT rows ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int row = 0; row < src.rows; row++){
		int bank = 0;

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(row);
			vec2* y = (vec2*)buf[bank ^ 1].ptr<Vec2f>(row);

			int a = 1 << p;
			int b = 1 << q;

			//cout << "------------------------" << endl;
			//cout << "p:" << p << "\t";
			//cout << "q:" << q << "\t";
			//cout << "a:" << a << "\t";
			//cout << "b:" << b << "\t";
			//cout << endl;

			for (int n = 0; n < N / 2; n = n++) {
				int iw = modq(n, p)*b;
				int ix0 = n;
				int ix1 = ix0 + N / 2;
				int iy0 = insertZeroBits(n, p, 1);
				int iy1 = iy0 + (1 << p);

				//cout << "w(" << iw << ")\t";
				//cout << "src(" << ix0 << "," << ix1 << ")\t";
				//cout << "dst(" << iy0 << "," << iy1 << ")\t";
				//cout << endl;

				vec2 tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];


				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;
			}
		}

	}
	// --- FFT cols ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int col = 0; col < src.cols; col++){
		int bank = (Q & 1);	//FFT rows ‚Ì‰ñ”‚ªŠï”‚È‚ç‚ÎŠï”bank‚©‚ç

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(0, col);
			vec2* y = (vec2*)buf[bank ^ 1].ptr<Vec2f>(0, col);

			int a = 1 << p;
			int b = 1 << q;

			//cout << "------------------------" << endl;
			//cout << "p:" << p << "\t";
			//cout << "q:" << q << "\t";
			//cout << "a:" << a << "\t";
			//cout << "b:" << b << "\t";
			//cout << endl;

			for (int n = 0; n < N / 2; n = n++) {
				int iw = modq(n, p)*b;
				int ix0 = n;
				int ix1 = ix0 + N / 2;
				int iy0 = insertZeroBits(n, p, 1);
				int iy1 = iy0 + (1 << p);
				ix0 *= src.cols;
				ix1 *= src.cols;
				iy0 *= src.cols;
				iy1 *= src.cols;

				//cout << "w(" << iw << ")\t";
				//cout << "src(" << ix0 << "," << ix1 << ")\t";
				//cout << "dst(" << iy0 << "," << iy1 << ")\t";
				//cout << endl;

				vec2 tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];


				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;
			}
		}

	}
	dst = buf[0];

}



// FFT Stockham,DIT
void fft_dit_Stockham_radix2_type1(const Mat& src, Mat &dst){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);
	int N = src.cols;
	CV_Assert(IsPow2(N));

	enum { _re_ = 0, _im_ = 1 };

	Mat buf[2];
	buf[0] = src.clone();
	buf[1] = Mat(src.size(), src.type());

	vector<vec2> w(N / 2);

	// --- twidle ----
	for (int n = 0; n < N / 2; n++){
		float jw = (float)(-2 * M_PI * n / N);
		w[n][_re_] = cos(jw);
		w[n][_im_] = sin(jw);
	}

	int Q = 0;
	while ((1 << Q) < N){ Q++; }

	// --- FFT rows ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int row = 0; row < src.rows; row++){
		int bank = 0;

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {

			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(row);
			vec2* y = (vec2*)buf[bank ^ 1].ptr<Vec2f>(row);

			//cout << "------------------------" << endl;
			//cout << "p:" << p << "\t";
			//cout << "q:" << q << "\t";
			//cout << endl;

			for (int n = 0; n<N / 2; n = n++) {
				int iw = (n >> q) << q;
				int ix0 = insertZeroBits(n, q, 1);
				int ix1 = ix0 + (1 << q);
				int iy0 = n;
				int iy1 = iy0 + N / 2;

				//cout << "w(" << iw << ")\t";
				//cout << "src(" << ix0 << "," << ix1 << ")\t";
				//cout << "dst(" << iy0 << "," << iy1 << ")\t";
				//cout << endl;

				vec2 tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];

				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;

			}
		}
	}
#if 1
	// --- FFT cols ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int col = 0; col < src.cols; col++){

		int bank = (Q & 1);	//FFT rows ‚Ì‰ñ”‚ªŠï”‚È‚ç‚ÎŠï”bank‚©‚ç

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(0, col);
			vec2* y = (vec2*)buf[bank ^ 1].ptr<Vec2f>(0, col);

			for (int n = 0; n<N / 2; n = n++) {
				int iw = (n >> q) << q;
				int ix0 = insertZeroBits(n, q, 1);
				int ix1 = ix0 + (1 << q);
				int iy0 = n;
				int iy1 = iy0 + N / 2;

				ix0 *= src.cols;
				ix1 *= src.cols;
				iy0 *= src.cols;
				iy1 *= src.cols;

				vec2 tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];

				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;

			}
		}
	}
#endif

	dst = buf[0];

}




