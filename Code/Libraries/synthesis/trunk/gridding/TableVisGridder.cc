#include <synthesis/gridding/TableVisGridder.h>

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

void TableVisGridder::generic(const bool forward,
					const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
					casa::Cube<casa::Complex>& data,
					casa::Cube<casa::Float>& weight,
					const casa::Vector<casa::Double>& freq,
					const Coord cellSize,
					const casa::Cube<Real>& C,
					const int support,
					const int overSample,
					const casa::Matrix<casa::uInt>& cOffset,
					casa::Cube<Value>& grid,
					casa::Vector<float>& sumwt)
{

	const int gSize = grid.ncolumn();
	const int nSamples = uvw.size();
	const int nChan = freq.size();
	const int nPol = data.shape()(2);

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
			
				Coord uScaled=freq[chan]*uvw(i)(0)/cellSize;
				int iu=int(uScaled);
				int fracu=int(overSample*(uScaled-Coord(iu)));
				iu+=gSize/2;

				Coord vScaled=freq[chan]*uvw(i)(0)/cellSize;
				int iv=int(vScaled);
				int fracv=int(overSample*(vScaled-Coord(iv)));
				iv+=gSize/2;

				if(forward) {
					for (int suppu=-support;suppu<+support;suppu++) {
						for (int suppv=-support;suppv<+support;suppv++) {
							Real wt=C(iu+fracu+cCenter,iu+fracu+cCenter,coff)*weight(i,chan,pol);
							grid(iu+suppu,iv+suppv,pol)+=wt*data(i,chan,pol);
							sumwt(pol)+=wt;
						}
					}
				}
				else {
					double sumviswt=0.0;
					for (int suppu=-support;suppu<+support;suppu++) {
						for (int suppv=-support;suppv<+support;suppv++) {
							Real wt=C(iu+fracu+cCenter,iu+fracu+cCenter,coff);
							data(i,chan,pol)+=wt*grid(iu+suppu,iv+suppv,pol);
							sumviswt+=wt;
						}
					}
					data(i,chan,pol)=data(i,chan,pol)/casa::Complex(sumviswt);
					weight(i,chan,pol)=sumviswt;
				}
			}
		}
	}
}

}
}
