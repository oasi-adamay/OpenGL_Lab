
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

