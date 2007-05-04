#include <gridding/TableVisGridder.h>

namespace conrad
{
namespace synthesis
{

TableVisGridder::TableVisGridder()
{
}

TableVisGridder::~TableVisGridder()
{
}

void TableVisGridder::forward(const IDataAccessor& ida,
		const casa::Vector<double>& cellSize,
		casa::Cube<casa::Complex>& grid,
		casa::Vector<float>& weights)
{
}
		
void TableVisGridder::forward(const IDataAccessor& ida,
		const casa::Vector<double>& cellSize,
		casa::Array<casa::Complex>& grid,
		casa::Matrix<float>& weights)
{
}
		
void TableVisGridder::reverse(IDataAccessor& ida, 
		const casa::Cube<casa::Complex>& grid, 
		const casa::Vector<double>& cellSize)
{
}

void TableVisGridder::reverse(IDataAccessor& ida, 
		const casa::Array<casa::Complex>& grid, 
		const casa::Vector<double>& cellSize)
{
}

void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
					const casa::Cube<casa::Complex>& visibility,
					const casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<float>& C,
					const int support,
					const int overSample,
					const casa::Matrix<uint>& cOffset,
					casa::Cube<casa::Complex>& grid,
					casa::Vector<float>& sumwt)
{

	const int gSize = grid.ncolumn();
	const int nSamples = uvw.size();
	const int nChan = freq.size();
	const int nPol = visibility.shape()(2);

	int cSize=2*(support+1)*overSample+1;

	int cCenter=(cSize-1)/2;
	
	sumwt.set(0.0);
	
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire support, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			for (int pol=0;pol<nPol;pol++) {

				int coff=cOffset(i,chan);
			
				double uScaled=freq[chan]*uvw(i)(0)/cellSize(0);
				int iu=int(uScaled);
				int fracu=int(overSample*(uScaled-double(iu)));
				iu+=gSize/2;

				double vScaled=freq[chan]*uvw(i)(0)/cellSize(1);
				int iv=int(vScaled);
				int fracv=int(overSample*(vScaled-double(iv)));
				iv+=gSize/2;

				for (int suppu=-support;suppu<+support;suppu++) {
					for (int suppv=-support;suppv<+support;suppv++) {
						float wt=C(iu+fracu+cCenter,iu+fracu+cCenter,coff)*visweight(i,chan,pol);
						grid(iu+suppu,iv+suppv,pol)+=wt*visibility(i,chan,pol);
						sumwt(pol)+=wt;
					}
				}
			}
		}
	}
}

void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
					casa::Cube<casa::Complex>& visibility,
					casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<float>& C,
					const int support,
					const int overSample,
					const casa::Matrix<uint>& cOffset,
					const casa::Cube<casa::Complex>& grid)
{

	const int gSize = grid.ncolumn();
	const int nSamples = uvw.size();
	const int nChan = freq.size();
	const int nPol = visibility.shape()(2);

	int cSize=2*(support+1)*overSample+1;

	int cCenter=(cSize-1)/2;
	
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire support, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			for (int pol=0;pol<nPol;pol++) {

				int coff=cOffset(i,chan);
			
				double uScaled=freq[chan]*uvw(i)(0)/cellSize(0);
				int iu=int(uScaled);
				int fracu=int(overSample*(uScaled-double(iu)));
				iu+=gSize/2;

				double vScaled=freq[chan]*uvw(i)(0)/cellSize(1);
				int iv=int(vScaled);
				int fracv=int(overSample*(vScaled-double(iv)));
				iv+=gSize/2;

				double sumviswt=0.0;
				for (int suppu=-support;suppu<+support;suppu++) {
					for (int suppv=-support;suppv<+support;suppv++) {
						float wt=C(iu+fracu+cCenter,iu+fracu+cCenter,coff);
						visibility(i,chan,pol)+=wt*grid(iu+suppu,iv+suppv,pol);
						sumviswt+=wt;
					}
				}
				visibility(i,chan,pol)=visibility(i,chan,pol)/casa::Complex(sumviswt);
				visweight(i,chan,pol)=sumviswt;
			}
		}
	}
}

}
}
