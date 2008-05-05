#include "VisWeightsMultiFrequency.h"

namespace askap
{
  namespace synthesis
  {

    VisWeightsMultiFrequency::VisWeightsMultiFrequency():
	    itsRefFreq((casa::Double)(1.405e+09)),itsOrder(0)
    {
    }

    VisWeightsMultiFrequency::VisWeightsMultiFrequency(casa::Double & reffreq)
    {
	    itsRefFreq = reffreq;
	    itsOrder = 0;
    }

    VisWeightsMultiFrequency::~VisWeightsMultiFrequency()
    {
    }
    
    VisWeightsMultiFrequency::VisWeightsMultiFrequency(const VisWeightsMultiFrequency &other) :
	    itsRefFreq(other.itsRefFreq), itsOrder(other.itsOrder)
    {
    }
    
    IVisWeights::ShPtr VisWeightsMultiFrequency::clone()
    {
	    return IVisWeights::ShPtr(new VisWeightsMultiFrequency(*this));
    }
    
    void VisWeightsMultiFrequency::setParameters(int order)
    {
	    //std::cout << "setting order from " << itsOrder << " to ";
	    itsOrder = order;
	    //std::cout << itsOrder << std::endl;
    }

    float VisWeightsMultiFrequency::getWeight(int i,double freq,int pol)
    {
	    // expensive..... choose fastest : if/else  or  'pow' operator.
	    if(itsOrder==0) 
               return 1.0;
	    else 
	    {
	       if(itsOrder==1)
		       return ((freq-itsRefFreq)/itsRefFreq);
	       else
		       return pow((freq-itsRefFreq)/itsRefFreq,itsOrder);
		       //return pow((itsFrequencyList[chan]-itsRefFreq)/itsRefFreq,itsOrder);
	    }
    }
    
  }
}
