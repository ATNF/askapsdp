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

void TableVisGridder::initConvolutionFunction() {
}

void TableVisGridder::reverse(IDataSharedIter& idi,
        const casa::Vector<double>& cellSize,
		casa::Cube<casa::Complex>& grid,
		casa::Vector<float>& weights)
{
}
		
void TableVisGridder::reverse(IDataSharedIter& idi,
        const casa::Vector<double>& cellSize,
		casa::Array<casa::Complex>& grid,
		casa::Matrix<float>& weights)
{
}
		
void TableVisGridder::forward(IDataSharedIter& idi,
    const casa::Vector<double>& cellSize, 
    const casa::Cube<casa::Complex>& grid)
{
}

void TableVisGridder::forward(IDataSharedIter& idi,
    const casa::Vector<double>& cellSize,
    const casa::Array<casa::Complex>& grid) 
{
}

void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
					const casa::Cube<casa::Complex>& visibility,
					const casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
                    casa::Cube<casa::Complex>& grid,
					casa::Vector<float>& sumwt)
{

	const int gSize = grid.ncolumn();
	const int nSamples = uvw.size();
	const int nChan = freq.size();
	const int nPol = visibility.shape()(2);

	int cSize=2*(itsSupport+1)*itsOverSample+1;

	int cCenter=(cSize-1)/2;
	
	sumwt.set(0.0);
	
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire itsSupport, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			for (int pol=0;pol<nPol;pol++) {

				int coff=cOffset(i,chan);
			
				double uScaled=freq[chan]*uvw(i)(0)/cellSize(0);
				int iu=int(uScaled);
				int fracu=int(itsOverSample*(uScaled-double(iu)));
				iu+=gSize/2;

				double vScaled=freq[chan]*uvw(i)(0)/cellSize(1);
				int iv=int(vScaled);
				int fracv=int(itsOverSample*(vScaled-double(iv)));
				iv+=gSize/2;

				for (int suppu=-itsSupport;suppu<+itsSupport;suppu++) {
					for (int suppv=-itsSupport;suppv<+itsSupport;suppv++) {
						float wt=itsC(iu+fracu+cCenter,iu+fracu+cCenter,coff)*visweight(i,chan,pol);
						grid(iu+suppu,iv+suppv,pol)+=wt*visibility(i,chan,pol);
						sumwt(pol)+=wt;
					}
				}
			}
		}
	}
}

void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
					casa::Cube<casa::Complex>& visibility,
					casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<casa::Complex>& grid)
{

	const int gSize = grid.ncolumn();
	const int nSamples = uvw.size();
	const int nChan = freq.size();
	const int nPol = visibility.shape()(2);

	int cSize=2*(itsSupport+1)*itsOverSample+1;

	int cCenter=(cSize-1)/2;
	
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire itsSupport, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			for (int pol=0;pol<nPol;pol++) {

				int coff=cOffset(i,chan);
			
				double uScaled=freq[chan]*uvw(i)(0)/cellSize(0);
				int iu=int(uScaled);
				int fracu=int(itsOverSample*(uScaled-double(iu)));
				iu+=gSize/2;

				double vScaled=freq[chan]*uvw(i)(0)/cellSize(1);
				int iv=int(vScaled);
				int fracv=int(itsOverSample*(vScaled-double(iv)));
				iv+=gSize/2;

				double sumviswt=0.0;
				for (int suppu=-itsSupport;suppu<+itsSupport;suppu++) {
					for (int suppv=-itsSupport;suppv<+itsSupport;suppv++) {
						float wt=itsC(iu+fracu+cCenter,iu+fracu+cCenter,coff);
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
