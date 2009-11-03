#ifndef TGRIDPERF_G
#define TGRIDPERF_G

#include <vector>

// Typedefs for easy testing
// Cost of using double for Coord is low, cost for
// double for Real is also low
typedef double Coord;
typedef float Real;
typedef std::complex<Real> Value;

struct Sample
{
	Value data;
	int iu;
	int iv;
	int cOffset;
};

class GridPerf
{
public:
	GridPerf() {};

	void init();
	void runGrid();
	void runDegrid();

	void gridKernel(const int support,
		const std::vector<Value>& C,
		std::vector<Value>& grid, const int gSize);

	void degridKernel(const std::vector<Value>& grid, const int gSize, const int support,
		const std::vector<Value>&C, std::vector<Value>& data);

	void initC(const int nSamples, const std::vector<Coord>& w,
		const std::vector<Coord>& freq, const Coord cellSize,
		const Coord baseline,
		const int wSize, const int gSize, int& support, int& overSample,
		Coord& wCellSize, std::vector<Value>& C);

	void initCOffset(const std::vector<Coord>& u, const std::vector<Coord>& v,
		const std::vector<Coord>& w, const std::vector<Coord>& freq,
		const Coord cellSize, const Coord wCellSize, const Coord baseline,
		const int wSize, const int gSize, const int support, const int overSample);

	std::vector<Value> grid;
	std::vector<Coord> u;
	std::vector<Coord> v;
	std::vector<Coord> w;
	std::vector<Sample> samples;
	std::vector<Value> outdata;

	std::vector<std::complex<float> > C;
	int support, overSample;

	Coord wCellSize;
};
#endif
