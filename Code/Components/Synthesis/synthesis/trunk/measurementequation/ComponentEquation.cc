#include <fitting/Params.h>
#include <dataaccess/SharedIter.h>
#include <measurementequation/ComponentEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>

#include <stdexcept>

using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
namespace synthesis
{

ComponentEquation::ComponentEquation(const ComponentEquation& other)
{
    operator=(other);
}
    
ComponentEquation& ComponentEquation::operator=(const ComponentEquation& other)
{
    if(this!=&other) {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
        itsIdi=other.itsIdi;
    }
}

void ComponentEquation::init()
{
	// The default parameters serve as a holder for the patterns to match the actual
	// parameters. Shell pattern matching rules apply.
	itsDefaultParams.reset();
	itsDefaultParams.add("flux.i");
	itsDefaultParams.add("direction.ra");
	itsDefaultParams.add("direction.dec");
}

void ComponentEquation::predict() 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}
	vector<string> completions(parameters().completions("flux.i"));
	vector<string>::iterator it;
    
    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {
    // This outer loop is over all strings that complete the flux.i.* pattern 
    // correctly. An exception will be throw if the parameters are not
    // consistent 
	   for (it=completions.begin();it!=completions.end();it++) {
	
            string fluxName("flux.i"+(*it));
    		string raName("direction.ra"+(*it));
    		string decName("direction.dec"+(*it));

    		const double ra=parameters().scalarValue(raName);
    		const double dec=parameters().scalarValue(decName);
    		const double fluxi=parameters().scalarValue(fluxName);

            for (uint row=0;row<itsIdi->nRow();row++) {        
        
                const casa::Vector<double>& freq=itsIdi->frequency();   
                const casa::Vector<double>& time=itsIdi->time();    
                casa::Vector<float> vis(2*freq.nelements());

    			this->calcRegularVis<float>(ra, dec, fluxi, freq, itsIdi->uvw()(row)(0), itsIdi->uvw()(row)(1), vis);

                for (uint i=0;i<freq.nelements();i++) {
				    itsIdi->rwVisibility()(row,i,0) += casa::Complex(vis(2*i), vis(2*i+1));
                }
			}
		}
	}
};

void ComponentEquation::calcEquations(NormalEquations& ne) 
{
	if(parameters().isCongruent(itsDefaultParams))
	{
		throw std::invalid_argument("Parameters not consistent with this equation");
	}

    // Loop over all completions i.e. all sources
    vector<string> completions(parameters().completions("flux.i"));
    vector<string>::iterator it;

    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {

    	const casa::Vector<double>& freq=itsIdi->frequency();	
    	const casa::Vector<double>& time=itsIdi->time();	
    	
    	const uint nParameters=3;
    	
    	// Define AutoDiff's for the output visibilities.
    	casa::Vector<casa::AutoDiff<double> > av(2*freq.nelements());
    	for (uint i=0;i<2*freq.nelements();i++) {
    		av[i]=casa::AutoDiff<double>(0.0, nParameters);
    	}
    
    	// Set up arrays to hold the output values
    	// Two values (complex) per row, channel, pol
    	uint nData=itsIdi->nRow()*freq.nelements()*2;
    	casa::Vector<casa::Double> raDeriv(nData);
    	casa::Vector<casa::Double> decDeriv(nData);
    	casa::Vector<casa::Double> fluxiDeriv(nData);
    	casa::Vector<casa::Double> residual(nData);
    	casa::Vector<double> weights(nData);
    	
    	for (it=completions.begin();it!=completions.end();it++) {
            
            DesignMatrix designmatrix(parameters());
            
    		uint offset=0;
    	
    		string raName("direction.ra"+(*it));
    		string decName("direction.dec"+(*it));
    		string fluxName("flux.i"+(*it));
    
    		// Define AutoDiff's for the three unknown parameters
    		casa::AutoDiff<double> ara(parameters().scalarValue(raName), nParameters, 0);
    		casa::AutoDiff<double> adec(parameters().scalarValue(decName), nParameters, 1);
    		casa::AutoDiff<double> afluxi(parameters().scalarValue(fluxName), nParameters, 2);
    			
    		for (uint row=0;row<itsIdi->nRow();row++) {
    
    			this->calcRegularVis<casa::AutoDiff<double> >(ara, adec, afluxi, freq, 
    				itsIdi->uvw()(row)(0), itsIdi->uvw()(row)(1), av);
    
    			for (uint i=0;i<freq.nelements();i++) {
                    residual(2*i+offset)=real(itsIdi->visibility()(row,i,0))-av(2*i).value();
                    residual(2*i+1+offset)=imag(itsIdi->visibility()(row,i,0))-av(2*i+1).value();
                    raDeriv(2*i+offset)=av[2*i].derivative(0);
                    raDeriv(2*i+1+offset)=av(2*i+1).derivative(0);     
                    decDeriv(2*i+offset)=av[2*i].derivative(1);
                    decDeriv(2*i+1+offset)=av(2*i+1).derivative(1);     
                    fluxiDeriv(2*i+offset)=av[2*i].derivative(2);
                    fluxiDeriv(2*i+1+offset)=av(2*i+1).derivative(2);     
                    weights(2*i+offset)=1.0;  
                    weights(2*i+1+offset)=1.0;  
    			}
    
    			offset+=2*freq.nelements();
    		}
    		// Now we can add the design matrix, residual, and weights
    		designmatrix.addDerivative(raName, raDeriv);
    		designmatrix.addDerivative(decName, decDeriv);
    		designmatrix.addDerivative(fluxName, fluxiDeriv);
    		designmatrix.addResidual(residual, weights);
                
            ne.add(designmatrix);
    	}
    }
};

// This can be done easily by hand (and we should do for production) but I'm leaving
// it in this form for the moment to show how the differentiation is done using
// casa::AutoDiff
template<class T>
void ComponentEquation::calcRegularVis(const T& ra, const T& dec, const T& flux, 
	const casa::Vector<double>& freq, const double u, const double v, 
	casa::Vector<T>& vis) 
{
	T  delay;
	delay = casa::C::_2pi * (ra * u + dec * v)/casa::C::c;
	T phase;
	for (uint i=0;i<freq.nelements();i++) {
		phase = delay * freq(i);
		vis(2*i)   = flux * cos(phase);
		vis(2*i+1) = flux * sin(phase);
	}
}

}

}
