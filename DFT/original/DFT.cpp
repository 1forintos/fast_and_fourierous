#include <complex>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "lodepng.h"

#ifndef M_PI
constexpr auto  M_PI = 3.14159265358979323846;
#endif

void loadPNG(std::string filename, unsigned& w, unsigned& h, std::vector<unsigned char>& image)
{
	unsigned char* imageData;
	unsigned err = lodepng_decode_file(&imageData, &w, &h, filename.c_str(), LCT_GREY, 8);
	if (err != 0)
	{
		std::cout << "Image decoder error: " << err << std::endl;
		exit(-1);
	}
	image.resize(w * h);
	memcpy(&image[0], imageData, w * h);
}

void savePNG(std::string filename, unsigned& w, unsigned& h, std::vector<unsigned char>& image)
{
	unsigned err = lodepng_encode_file(filename.c_str(), image.data(), w, h, LCT_GREY, 8);
	if (err != 0)
	{
		std::cout << "Image encoder error: " << err << std::endl;
		exit(-1);
	}
}

void RealToComplex(std::vector<unsigned char>& in, std::vector<std::complex<double>>& out)
{
	out.clear();
	out.reserve(in.size());
	for (auto r : in)
	{
		out.push_back(std::complex<double>(static_cast<double>(r), 0.0));
	}
}

void ComplexToReal(std::vector<std::complex<double>>&in, std::vector<unsigned char>& out)
{
	out.clear();
	out.reserve(in.size());
	for (auto c : in)
	{
		double length = sqrt(c.real() * c.real() + c.imag() * c.imag());
		out.push_back(length);
	}
}

void DFT(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out, unsigned w, unsigned h, bool horizontal, bool inverse)
{
	out.clear();
	out.resize(in.size());

	for (unsigned i = 0; i < h; ++i)
	{
		for (unsigned k = 0; k < w; ++k)
		{
			std::complex<double> sum(0.0, 0.0);
			for (unsigned j = 0; j < w; ++j)
			{
				size_t addr = horizontal ? j + i * w : i + j * w;
				auto angle = (inverse ? -2.0f : 2.0) * M_PI * j * k / w;
				sum.real(sum.real() + in[addr].real() * cos(angle) - in[addr].imag() * sin(angle));
				sum.imag(sum.imag() + in[addr].real() * sin(angle) + in[addr].imag() * cos(angle));
			}

			if (!inverse)
			{
				sum *= 1.0 / w;
			}

			size_t addr = horizontal ? k + i * w : i + k * w;
			out[k + i * w] = sum;
		}
		std::cout << i << std::endl;
	}
}

int main()
{
	std::string inName("lena.png");
	std::string dftName("lenaDFT.png");
	std::string outName("lenaOut.png");

	unsigned w = 0, h = 0;
	std::vector<unsigned char> image;
	loadPNG(inName, w, h, image);

	std::vector<std::complex<double>> f1;
	std::vector<std::complex<double>> f2;
	RealToComplex(image, f1);
	DFT(f1, f2, w, h, true, false);
	DFT(f2, f1, w, h, false, false);
	std::cout << "DFT finished" << std::endl;
	ComplexToReal(f1, image);
	savePNG(dftName, w, h, image);

	DFT(f1, f2, w, h, true, true);
	DFT(f2, f1, w, h, false, true);
	std::cout << "IDFT finished" << std::endl;
	ComplexToReal(f1, image);
	savePNG(outName, w, h, image);

	return 0;
}

