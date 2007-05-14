//
// @file : Evolving demonstration program for synthesis capabilities
//

#include <measurementequation/ImageEquation.h>
#include <fitting/LinearSolver.h>
#include <dataaccess/DataAccessorStub.h>

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

void printVectorAsMatrix(const uint npix, const casa::Vector<double>& vec) {
	std::cout << std::fixed;
	std::cout << "      "; 
	for (int col=0;col<npix;col++) {
 		std::cout.width(7); 
 		std::cout << col << " ";
	}
	std::cout << std::endl;
	std::cout.precision(3); 

	for (int row=0;row<npix;row++) {
		std::cout << "[";
		std::cout.width(3); 
		std::cout << row << "] ";
		for (int col=0;col<npix;col++) {
 			std::cout.width(7); 
 			std::cout << vec(row*npix+col) << " ";
		}
		std::cout << std::endl;
	}
}

int main() {
	
	cout << "Synthesis demonstration program" << endl;

	boost::shared_ptr<IDataAccessor> ida(new DataAccessorStub(true));
    
	Params perfect;

	int npix=16;
	cout << "Making " << npix << " by " << npix << " pixel image" << endl;
		
    Domain imageDomain;
    imageDomain.add("RA", -120.0*casa::C::arcsec, +120.0*casa::C::arcsec, npix); 
    imageDomain.add("DEC", -120.0*casa::C::arcsec, +120.0*casa::C::arcsec, npix); 

	cout << "Adding two point sources" << endl;
	{
		casa::Vector<double> imagePixels1(npix*npix);
		imagePixels1.set(0.0);
		imagePixels1(npix/2+npix*npix/2)=1.0;
		imagePixels1(12+npix*3)=0.7;
		perfect.add("image.i.cena", imagePixels1, imageDomain);
		printVectorAsMatrix(npix, imagePixels1);
	}

	// Predict with the "perfect" parameters"
	cout << "Predicting data from perfect model" << endl;
    cout << endl;
	{
		ImageEquation perfecteq(perfect, ida);
		perfecteq.predict();
	}

	cout << "Making imperfect model" << endl;
    cout << endl;
	Params imperfect;
	{
		casa::Vector<double> imagePixels2(npix*npix);
		imagePixels2.set(0.0);
		imagePixels2(npix/2+npix*npix/2)=0.9;
		imagePixels2(12+npix*3)=0.75;
		imperfect.add("image.i.cena", imagePixels2, imageDomain);
		printVectorAsMatrix(npix, imagePixels2);
        cout << endl;
	}

	cout << "Calculating derivatives from imperfect model" << endl;
    cout << endl;
    
	NormalEquations normeq(perfect);
	{
		ImageEquation imperfecteq(imperfect, ida);
		imperfecteq.calcEquations(normeq);
	}
    cout << "Data vector (i.e. residual image):" << endl;
    printVectorAsMatrix(npix, normeq.dataVector()["image.i.cena"]);
    cout << "Slice of normal equations (i.e. dirty psf):" << endl;
    printVectorAsMatrix(npix, normeq.normalMatrix()["image.i.cena"]["image.i.cena"].column(npix/2+npix*npix/2));
    cout << endl;
	
    {
        cout << "Solving for updated parameters using SVD of normal equations" 
            << endl;
        Quality q3;
        LinearSolver solver3(imperfect);
        solver3.addNormalEquations(normeq);
        solver3.solveNormalEquations(q3, true);
        casa::Vector<double> improved3=solver3.parameters().value("image.i.cena");
        cout << q3 << endl;
        cout << "Updated model:" << endl;
        printVectorAsMatrix(npix, improved3);
        cout << endl;
    }

	std::cout << "Done" << std::endl;
	return 0;	  	  
};
