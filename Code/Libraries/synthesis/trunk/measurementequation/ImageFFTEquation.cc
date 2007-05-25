#include <dataaccess/SharedIter.h>
#include <fitting/Params.h>
#include <measurementequation/ImageFFTEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Domain.h>

#include <gridding/SphFuncVisGridder.h>

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

ImageFFTEquation::ImageFFTEquation(const ImageFFTEquation& other)
{
    operator=(other);
}
    
ImageFFTEquation& ImageFFTEquation::operator=(const ImageFFTEquation& other)
{
    if(this!=&other) {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
        itsIdi=other.itsIdi;
    }
}

void ImageFFTEquation::init()
{
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("image.i");
}

void ImageFFTEquation::predict() 
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
            
            casa::Cube<casa::Complex> uvGrid(raCells, decCells, 1);
            casa::Vector<double> uvCellsize(2);
            uvCellsize(0)=double(raCells)/(raStart-raEnd);
            uvCellsize(1)=double(decCells)/(decStart-decEnd);
    
            SphFuncVisGridder tvg(itsIdi);
            
            itsIdi.chooseBuffer("model");
            
            tvg.forward(uvCellsize, uvGrid);
    	}
    }
};

void ImageFFTEquation::calcEquations(NormalEquations& ne) 
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

        IDataSharedIter modelIdi(itsIdi);
        modelIdi.chooseBuffer("model");

        IDataSharedIter residualIdi(itsIdi);
        residualIdi.chooseBuffer("residual");

        SphFuncVisGridder tvgModel(modelIdi);
        SphFuncVisGridder tvgResidual(residualIdi);
       
    	const casa::Vector<double>& freq=itsIdi->frequency();	
    	const uint nChan=freq.nelements();
    	const uint nRow=itsIdi->nRow();

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
            
            casa::Cube<casa::Complex> uvGrid(raCells, decCells, 1);
            casa::Vector<double> uvCellsize(2);
            uvCellsize(0)=double(raCells)/(raStart-raEnd);
            uvCellsize(1)=double(decCells)/(decStart-decEnd);
            
            tvgModel.forward(uvCellsize, uvGrid);
            residualIdi->rwVisibility()=itsIdi->visibility()-modelIdi->visibility();
            
            casa::Vector<float> uvWeights;
            tvgResidual.reverse(uvCellsize, uvGrid, uvWeights);

            casa::Matrix<double> imageDeriv(raCells, decCells);

            // Now we can add the design matrix, residual, and weights
//            ne.add(designmatrix, NormalEquations::COMPLETE);
        }
	}
};

}

}
