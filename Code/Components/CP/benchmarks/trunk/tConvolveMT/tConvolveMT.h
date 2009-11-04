/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

#ifndef TCONVOLVEMT_H
#define TCONVOLVEMT_H

// System includes
#include <vector>

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

class tConvolveMT
{
public:
	tConvolveMT();

    int randomInt();
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

    unsigned long next;
};
#endif
