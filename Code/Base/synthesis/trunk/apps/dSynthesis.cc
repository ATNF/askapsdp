//
// @file : Evolving demonstration program for synthesis capabilities
//

#include <measurementequation/ImageDFTEquation.h>

#include <fitting/ParamsCASATable.h>
#include <fitting/LinearSolver.h>

#include <dataaccess/DataIteratorStub.h>


#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

using namespace conrad::scimath;

using namespace conrad::synthesis;

void printArray(const casa::Array<double>& arr) {
    casa::IPosition shape(arr.shape());
    std::cout << std::fixed;
    std::cout << "      "; 
    for (int col=0;col<shape(1);col++) {
        std::cout.width(7); 
        std::cout << col << " ";
    }
    std::cout << std::endl;
    std::cout.precision(3); 

    for (int row=0;row<shape(1);row++) {
        std::cout << "[";
        std::cout.width(3); 
        std::cout << row << "] ";
        for (int col=0;col<shape(0);col++) {
            std::cout.width(7); 
            std::cout << arr(casa::IPosition(2, row, col)) << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
	
	cout << "Synthesis demonstration program" << endl;

	IDataSharedIter idi(new DataIteratorStub(1));
    
    int npix=16;
	cout << "Making " << npix << " by " << npix << " pixel image" << endl;
		
    Domain imageDomain;
    imageDomain.add("RA",  -120.0*casa::C::arcsec, +120.0*casa::C::arcsec); 
    imageDomain.add("DEC", -120.0*casa::C::arcsec, +120.0*casa::C::arcsec); 

    cout << "Adding two point sources" << endl;
	{
		casa::Array<double> imagePixels1(casa::IPosition(2, npix, npix));
		imagePixels1.set(0.0);
        imagePixels1(casa::IPosition(2, npix/2, npix/2))=1.0;
        imagePixels1(casa::IPosition(2, 12, 3))=0.7;
        Params perfect;
		perfect.add("image.i.cena", imagePixels1, imageDomain);
		printArray(imagePixels1);
    	// Predict with the "perfect" parameters"
    	cout << "Predicting data from perfect model" << endl;
		ImageDFTEquation perfecteq(perfect, idi);
		perfecteq.predict();
	}

    cout << "Making imperfect model" << endl;
    cout << endl;
	Params imperfect;
	{
		casa::Array<double> imagePixels2(casa::IPosition(2, npix, npix));
		imagePixels2.set(0.0);
        imagePixels2(casa::IPosition(2, npix/2, npix/2))=0.9;
        imagePixels2(casa::IPosition(2, 12, 3))=0.75;
		imperfect.add("image.i.cena", imagePixels2, imageDomain);
		printArray(imagePixels2);
        cout << endl;
	}

	cout << "Calculating derivatives from imperfect model" << endl;
    cout << endl;
    
	NormalEquations normeq(imperfect);
	{
		ImageDFTEquation imperfecteq(imperfect, idi);
		imperfecteq.calcEquations(normeq);
	}
    casa::Array<double> dv(normeq.dataVector()["image.i.cena"].reform(casa::IPosition(2, npix, npix)));
    cout << "Data vector (i.e. residual image):" << endl;
    printArray(dv);
    casa::Array<double> psf(normeq.normalMatrix()["image.i.cena"]["image.i.cena"].column(npix/2+npix*npix/2).reform(casa::IPosition(2, npix, npix)));
    cout << "Slice of normal equations (i.e. dirty psf):" << endl;
    printArray(psf);
    cout << endl;
	
    {
        Quality q3;
        LinearSolver solver3(imperfect);
        solver3.addNormalEquations(normeq);
        cout << "Solving for updated parameters using SVD of normal equations" 
            << endl;
        solver3.solveNormalEquations(q3, true);
        cout << q3 << endl;
        cout << "Updated model:" << endl;
        casa::Array<double> improved3=solver3.parameters().value("image.i.cena");
        printArray(improved3);
        cout << endl;
        ParamsCASATable pt("dSynthesis_params.tab");
        pt.setParameters(solver3.parameters());
    }

	std::cout << "Done" << std::endl;
	return 0;	  	  
};
