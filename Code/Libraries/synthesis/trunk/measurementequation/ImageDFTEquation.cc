#include <dataaccess/SharedIter.h>
#include <fitting/Params.h>
#include <measurementequation/ImageDFTEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Domain.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>

using conrad::scimath::Params;
using conrad::scimath::Domain;
using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;


namespace conrad
{
namespace synthesis
{

ImageDFTEquation::ImageDFTEquation(const ImageDFTEquation& other)
{
    operator=(other);
}
    
ImageDFTEquation& ImageDFTEquation::operator=(const ImageDFTEquation& other)
{
    if(this!=&other) {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
        itsIdi=other.itsIdi;
    }
}

void ImageDFTEquation::init()
{
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("image.i");
}

void ImageDFTEquation::predict() 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

	vector<string> completions(parameters().completions("image.i"));
	vector<string>::iterator it;
    
    itsIdi.chooseBuffer("model");
    
    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {

        const casa::Vector<double>& freq=itsIdi->frequency();   
        const casa::Vector<double>& time=itsIdi->time();    
        const uint nChan=freq.nelements();
        const uint nRow=itsIdi->nRow();
        casa::Matrix<double> vis(nRow,2*nChan);
    
    	for (it=completions.begin();it!=completions.end();it++) {
    	
    		string imageName("image.i"+(*it));
    		Domain domain(parameters().domain(imageName));
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
    
            casa::Matrix<double> noDeriv(0,0);
            this->calcVisDFT(imagePixels, raStart, raEnd, raCells, decStart, decEnd, decCells, 
                freq, itsIdi->uvw(), vis, false, noDeriv);
    
    		for (uint row=0;row<nRow;row++) {
    			for (uint i=0;i<nChan;i++) {
    				itsIdi->rwVisibility()(row,i,0) += casa::Complex(vis(row,2*i), vis(row,2*i+1));
    			}
    		}
    	}
    }
};

void ImageDFTEquation::calcEquations(NormalEquations& ne) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

    // Loop over all completions i.e. all sources
    vector<string> completions(parameters().completions("image.i"));
    vector<string>::iterator it;

    itsIdi.chooseOriginal();

    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {

    	const casa::Vector<double>& freq=itsIdi->frequency();	
    	const uint nChan=freq.nelements();
    	const uint nRow=itsIdi->nRow();
    	const casa::Vector<double>& time=itsIdi->time();	
    		
    	// Set up arrays to hold the output values
    	// Row, Two values (complex) per channel, single pol
    	casa::Vector<double> residual(2*nRow*nChan);
    	casa::Vector<double> weights(2*nRow*nChan);
    	weights.set(1.0);
    	casa::Matrix<double> vis(nRow,2*nChan);
    
    	for (it=completions.begin();it!=completions.end();it++) {
            
    		string imageName("image.i"+(*it));
    		Domain domain(parameters().domain(imageName));
    
    		double raStart=domain.start("RA");
    		double raEnd=domain.end("RA");
    		int raCells=domain.cells("RA");
    
    		double decStart=domain.start("DEC");
    		double decEnd=domain.end("DEC");
    		int decCells=domain.cells("DEC");
    
    		const casa::Vector<double> imagePixels=parameters().value(imageName);
    		const uint nPixels=imagePixels.nelements();
    		
            DesignMatrix designmatrix(parameters());
            casa::Matrix<double> imageDeriv(2*nRow*nChan,nPixels);
            
            this->calcVisDFT(imagePixels, raStart, raEnd, raCells, 
                 decStart, decEnd, decCells, freq, itsIdi->uvw(),
                 vis, true, imageDeriv);
            for (uint row=0;row<itsIdi->nRow();row++) {
                for (uint i=0;i<freq.nelements();i++) {
                    residual(nChan*row+2*i)=real(itsIdi->visibility()(row,i,0))-vis(row,2*i);
                    residual(nChan*row+2*i+1)=imag(itsIdi->visibility()(row,i,0))-vis(row,2*i+1);
                }
            }
            for (uint row=0;row<itsIdi->nRow();row++) {
                for (uint i=0;i<freq.nelements();i++) {
                    residual(nChan*row+2*i)=real(itsIdi->visibility()(row,i,0))-vis(row,2*i);
                    residual(nChan*row+2*i+1)=imag(itsIdi->visibility()(row,i,0))-vis(row,2*i+1);
                }
            }
            // Now we can add the design matrix, residual, and weights
            designmatrix.addDerivative(imageName, imageDeriv);
            designmatrix.addResidual(residual, weights);
            ne.add(designmatrix, NormalEquations::COMPLETE);
        }
	}
};

void ImageDFTEquation::calcVisDFT(const casa::Vector<double>& imagePixels, 
    const double raStart, const double raEnd, const int raCells, 
    const double decStart, const double decEnd, const int decCells, 
    const casa::Vector<double>& freq,  
    const casa::Vector<casa::RigidVector<double, 3> >& uvw,
    casa::Matrix<double>& vis, bool doDeriv, casa::Matrix<double>& imageDeriv) 
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
                        vis(row,2*i) += flux * cos(phase);
                        vis(row,2*i+1) += flux * sin(phase);
                        imageDeriv(nChan*row+2*i,pixel) = cos(phase);
                        imageDeriv(nChan*row+2*i+1,pixel) = sin(phase);
                    }
                }
                else {
                    for (uint i=0;i<nChan;i++) {
                        double phase = delay * freq(i);
                        vis(row,2*i) += flux * cos(phase);
                        vis(row,2*i+1) += flux * sin(phase);
                    }
                }
                pixel++;
            }
        }
    }
}

}

}
