#include <dataaccess/IDataAccessor.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEImageEquation.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEDomain.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>

namespace conrad
{
namespace synthesis
{

MEImageEquation::~MEImageEquation()
{
}

void MEImageEquation::init()
{
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("image.i");
}

void MEImageEquation::predict(IDataAccessor& ida) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}
	const casa::Vector<double>& freq=ida.frequency();	
	const casa::Vector<double>& time=ida.time();	
	const uint nChan=freq.nelements();
	const uint nRow=ida.nRow();
	casa::Matrix<casa::DComplex> vis(nRow,nChan);
	casa::Matrix<casa::DComplex> noDeriv(0,0);

	vector<string> completions(parameters().completions("image.i"));
	vector<string>::iterator it;
	for (it=completions.begin();it!=completions.end();it++) {
	
		string imageName("image.i"+(*it));
		MEDomain domain(parameters().domain(imageName));
		if(!domain.has("RA")||!domain.has("DEC")) {
			throw(std::invalid_argument("RA and DEC specification not present for "+imageName));
		}
		double raStart=domain.start("RA");
		double raEnd=domain.end("RA");
		int raCells=domain.cells("RA");

		double decStart=domain.start("DEC");
		double decEnd=domain.end("DEC");
		int decCells=domain.cells("DEC");

		const casa::Vector<double> imagePixels=parameters().value(imageName);
		const uint nPixels=imagePixels.nelements();

		this->calcVis(imagePixels, raStart, raEnd, raCells, decStart, decEnd, decCells, 
			freq, ida.uvw(), vis, false, noDeriv);

		for (uint row=0;row<nRow;row++) {
			for (uint i=0;i<nChan;i++) {
				ida.visibility()(row,i,0) += casa::Complex(vis(row,i));
			}
		}
	}
};

void MEImageEquation::calcEquations(IDataAccessor& ida, MEDesignMatrix& designmatrix) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

	const casa::Vector<double>& freq=ida.frequency();	
	const uint nChan=freq.nelements();
	const uint nRow=ida.nRow();
	const casa::Vector<double>& time=ida.time();	
		
	// Set up arrays to hold the output values
	// Row, Two values (complex) per channel, single pol
	casa::Vector<casa::DComplex> residual(nRow*nChan);
	casa::Vector<double> weights(nRow*nChan);
	weights.set(1.0);
	casa::Matrix<casa::DComplex> vis(nRow,nChan);

	// Loop over all completions i.e. all sources
	vector<string> completions(parameters().completions("image.i"));
	vector<string>::iterator it;
	for (it=completions.begin();it!=completions.end();it++) {
		string imageName("image.i"+(*it));
		MEDomain domain(parameters().domain(imageName));

		double raStart=domain.start("RA");
		double raEnd=domain.end("RA");
		int raCells=domain.cells("RA");

		double decStart=domain.start("DEC");
		double decEnd=domain.end("DEC");
		int decCells=domain.cells("DEC");

		const casa::Vector<double> imagePixels=parameters().value(imageName);
		const uint nPixels=imagePixels.nelements();
		casa::Matrix<casa::DComplex> imageDeriv(nRow*nChan,nPixels);
		
		this->calcVis(imagePixels, raStart, raEnd, raCells, 
			decStart, decEnd, decCells, freq, ida.uvw(),
			vis, true, imageDeriv);

		for (uint row=0;row<ida.nRow();row++) {
			for (uint i=0;i<freq.nelements();i++) {
				residual(nChan*row+i)=casa::DComplex(ida.visibility()(row,i,0))-vis(row,i);
			}
		}

		// Now we can add the design matrix, residual, and weights
		designmatrix.addDerivative(imageName, imageDeriv);
		designmatrix.addResidual(residual, weights);
	}
};

void MEImageEquation::calcEquations(IDataAccessor& ida, MENormalEquations& normeq) 
{
	// We can only make a relatively poor approximation to the normal equations
	normeq.setApproximation(MENormalEquations::DIAGONAL_SLICE);
};

void MEImageEquation::calcVis(const casa::Vector<double>& imagePixels, 
	const double raStart, const double raEnd, const int raCells, 
	const double decStart, const double decEnd, const int decCells, 
	const casa::Vector<double>& freq,  
	const casa::Vector<casa::RigidVector<double, 3> >& uvw,
	casa::Matrix<casa::DComplex>& vis, bool doDeriv, casa::Matrix<casa::DComplex>& imageDeriv) 
{
	double raInc=(raStart-raEnd)/double(raCells);
	double decInc=(decStart-decEnd)/double(decCells);
	const uint nRow=uvw.nelements();
	const uint nChan=freq.nelements();

	vis.set(0.0);
	
	for (uint row=0;row<nRow;row++) {
		uint pixel=0;
		double u=uvw(row)(0);
		double v=uvw(row)(1);
		double w=uvw(row)(2);
		for (uint l=0;l<raCells;l++) {
			double ra = raStart + l * raInc;
			for (uint m=0;m<decCells;m++) {
				double dec = decStart + m * decInc;
				double delay = casa::C::_2pi * (ra * u + dec * v + sqrt(1 - ra * ra - dec * dec) * w)/casa::C::c;
				double flux = imagePixels(pixel);
				if(doDeriv) {
					for (uint i=0;i<nChan;i++) {
						double phase = delay * freq(i);
						vis(row,i) += casa::Complex(flux * cos(phase), flux * sin(phase));
						imageDeriv(nChan*row+i,pixel) = casa::Complex(cos(phase), sin(phase));
					}
				}
				else {
					for (uint i=0;i<nChan;i++) {
						double phase = delay * freq(i);
						vis(row,i) += casa::Complex(flux * cos(phase), flux * sin(phase));
					}
				}
				pixel++;
			}
		}
	}
}

}

}
