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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".deconvolver");

#include <images/Images/PagedImage.h>

#include <deconvolution/DeconvolverHelpers.h>
#include <deconvolution/DeconvolverFactory.h>
#include <deconvolution/DeconvolverBase.h>
#include <deconvolution/DeconvolverHogbom.h>
#include <deconvolution/DeconvolverControl.h>
#include <deconvolution/DeconvolverMonitor.h>

#include <askap/AskapError.h>

#include <casa/BasicSL/String.h>   // for downcase


namespace askap {
  namespace synthesis {
    
    DeconvolverFactory::DeconvolverFactory() {
    }
    
    DeconvolverBase<Float, Complex>::ShPtr DeconvolverFactory::make(const LOFAR::ParameterSet &parset) {

      DeconvolverBase<Float, Complex>::ShPtr deconvolver;
      
      if(parset.getString("solver", "Clean")=="Clean") {
        Array<Float> dirty(DeconvolverHelpers::getArrayFromImage("dirty", parset));
        Array<Float> psf(DeconvolverHelpers::getArrayFromImage("psf", parset));
        
        string algorithm=parset.getString("solver.Clean.algorithm","Hogbom");
        
        if (algorithm=="Hogbom"){
          ASKAPLOG_INFO_STR(logger, "Constructing Hogbom Clean deconvolver");
          deconvolver=boost::shared_ptr<DeconvolverBase<Float, Complex> >(new DeconvolverHogbom<Float, Complex>(dirty, psf));
        }
        // Get the mask and weights images
        deconvolver->setMask(DeconvolverHelpers::getArrayFromImage("mask", parset));
        deconvolver->setWeight(DeconvolverHelpers::getArrayFromImage("weight", parset));
        
        // Now get the control parameters
        deconvolver->control()->setGain(parset.getFloat("solver.Clean.gain", 0.1));
        deconvolver->control()->setTolerance(parset.getFloat("solver.Clean.tolerance", 1e-3));
        deconvolver->control()->setTargetIter(parset.getInt32("solver.Clean.niter", 100));
        deconvolver->control()->setTargetObjectiveFunction(parset.getFloat("solver.Clean.threshold", 0.1));
      }
      ASKAPASSERT(deconvolver);
      return deconvolver;
      
    }
  }

}
