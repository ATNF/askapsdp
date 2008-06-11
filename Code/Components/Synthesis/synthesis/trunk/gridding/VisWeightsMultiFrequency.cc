/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

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
