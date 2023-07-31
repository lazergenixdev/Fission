#include <cmath>
#include <complex>

void fft_rec(std::complex<double>* x, int N);

#define M_PI 3.14159265359
std::complex<double> temp_data[4096*8];
int offset = 0;

void begin_fft() {
	offset = 0;
}

void fft(float* x_in,
	std::complex<double>* x_out,
	int N) {

	// Make copy of array and apply window
	for (int i = 0; i < N; i++) {
		x_out[i] = std::complex<double>((double)x_in[i], 0);
		x_out[i] *= 0.5 * (1.0 - cos((2*M_PI*i)/(double)N)); // Window
	}

	// Start recursion
	fft_rec(x_out, N);
}

void fft_rec(std::complex<double>* x, int N) {
	// Check if it is splitted enough
	if (N <= 1) {
		return;
	}

	// Split even and odd
	std::complex<double>* odd = temp_data + offset;
	std::complex<double>* even = temp_data + offset + N / 2;
	offset += N;
	for (int i = 0; i < N / 2; i++) {
		even[i] = x[i * 2];
		odd[i] = x[i * 2 + 1];
	}

	// Split on tasks
	fft_rec(even, N / 2);
	fft_rec(odd, N / 2);


	// Calculate DFT
	for (int k = 0; k < N / 2; k++) {
		std::complex<double> t = exp(std::complex<double>(0, -2 * M_PI * k / N)) * odd[k];
		x[k] = even[k] + t;
		x[N / 2 + k] = even[k] - t;
	}
}