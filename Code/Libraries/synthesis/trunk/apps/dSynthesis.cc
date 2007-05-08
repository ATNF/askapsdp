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

	DataAccessorStub ida(true);
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
	{
		ImageEquation perfecteq(perfect);
		perfecteq.predict(ida);
	}

	cout << "Making imperfect model" << endl;
	Params imperfect1;
	Params imperfect2;
	Params imperfect3;
	{
		casa::Vector<double> imagePixels2(npix*npix);
		imagePixels2.set(0.0);
		imagePixels2(npix/2+npix*npix/2)=0.9;
		imagePixels2(12+npix*3)=0.75;
		imperfect1.add("image.i.cena", imagePixels2, imageDomain);
		imperfect2.add("image.i.cena", imagePixels2, imageDomain);
		imperfect3.add("image.i.cena", imagePixels2, imageDomain);
		printVectorAsMatrix(npix, imagePixels2);
	}

	cout << "Calculating derivatives from imperfect model" << endl;
	DesignMatrix dm(perfect);
	{
		ImageEquation imperfecteq(imperfect1);
		imperfecteq.calcEquations(ida, dm);
	}
	NormalEquations normeq(dm, NormalEquations::COMPLETE);
	
	{
		cout << "Solving for updated parameters using SVD of the design matrix" << endl;
		Quality q1;
		LinearSolver solver1(imperfect1);
		solver1.addDesignMatrix(dm);
		solver1.solveDesignMatrix(q1);
		casa::Vector<double> improved1=solver1.parameters().value("image.i.cena");
		cout << q1 << endl;
		cout << "Updated model:" << endl;
		printVectorAsMatrix(npix, improved1);
	}
	
	{
		cout << "Solving for updated parameters using Cholesky decomposition of normal equations" 
			<< endl;
		Quality q2;
		LinearSolver solver2(imperfect2);
		cout << "Data vector (i.e. residual image):" << endl;
		printVectorAsMatrix(npix, normeq.dataVector()["image.i.cena"]);
		cout << "Slice of normal equations (i.e. dirty psf):" << endl;
		printVectorAsMatrix(npix, normeq.normalMatrix()["image.i.cena"]["image.i.cena"].column(npix/2+npix*npix/2));
		solver2.addNormalEquations(normeq);
		solver2.solveNormalEquations(q2);
		casa::Vector<double> improved2=solver2.parameters().value("image.i.cena");
		cout << q2 << endl;
		cout << "Updated model:" << endl;
		printVectorAsMatrix(npix, improved2);
	}

	{
		cout << "Solving for updated parameters using SVD of normal equations" 
			<< endl;
		Quality q3;
		LinearSolver solver3(imperfect3);
		solver3.addNormalEquations(normeq);
		solver3.solveNormalEquations(q3, true);
		casa::Vector<double> improved3=solver3.parameters().value("image.i.cena");
		cout << q3 << endl;
		cout << "Updated model:" << endl;
		printVectorAsMatrix(npix, improved3);
	}

	std::cout << "Done" << std::endl;
	return 0;	  	  
};
