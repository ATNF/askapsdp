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
ASKAP_LOGGER(logger, ".deconvolver.factory");

#include <images/Images/PagedImage.h>

#include <deconvolution/DeconvolverHelpers.h>
#include <deconvolution/DeconvolverFactory.h>
#include <deconvolution/DeconvolverBase.h>
#include <deconvolution/DeconvolverBasisFunction.h>
#include <deconvolution/DeconvolverFista.h>
#include <deconvolution/DeconvolverHogbom.h>
#include <deconvolution/DeconvolverControl.h>
#include <deconvolution/DeconvolverMonitor.h>

#include <deconvolution/MultiScaleBasisFunction.h>

#include <askap/AskapError.h>

#include <casa/BasicSL/String.h>   // for downcase


namespace askap {
  namespace synthesis {
    
    DeconvolverFactory::DeconvolverFactory() {
    }
    
    DeconvolverBase<Float, Complex>::ShPtr DeconvolverFactory::make(const LOFAR::ParameterSet &parset) {

      DeconvolverBase<Float, Complex>::ShPtr deconvolver;
      
      Array<Float> dirty(DeconvolverHelpers::getArrayFromImage("dirty", parset));
      Array<Float> psf(DeconvolverHelpers::getArrayFromImage("psf", parset));

      if(parset.getString("solver", "Fista")=="Fista") {        
        ASKAPLOG_INFO_STR(logger, "Constructing Fista deconvolver");
        deconvolver=boost::shared_ptr<DeconvolverBase<Float, Complex> >(new DeconvolverFista<Float, Complex>(dirty, psf));
        ASKAPASSERT(deconvolver);

        // Get the mask and weights images
        deconvolver->setMask(DeconvolverHelpers::getArrayFromImage("mask", parset));
        deconvolver->setWeight(DeconvolverHelpers::getArrayFromImage("weight", parset));
        
        // Now get the control parameters
	LOFAR::ParameterSet subset(parset.makeSubset("solver.Fista"));
        deconvolver->configure(parset);

	// Now set up controller
	boost::shared_ptr<DeconvolverControl<Float> > controller(new DeconvolverControl<Float>());
        ASKAPASSERT(controller);
	controller->configure(parset);
	deconvolver->setControl(controller);

	// Now set up monitor
	boost::shared_ptr<DeconvolverMonitor<Float> > monitor(new DeconvolverMonitor<Float>());
        ASKAPASSERT(monitor);
	monitor->configure(parset);
	deconvolver->setMonitor(monitor);

      }
      else if(parset.getString("solver", "Basisfunction")=="Basisfunction") {        
        ASKAPLOG_INFO_STR(logger, "Constructing Basis Function deconvolver");
        deconvolver=boost::shared_ptr<DeconvolverBase<Float, Complex> >(new DeconvolverBasisFunction<Float, Complex>(dirty, psf));
        ASKAPASSERT(deconvolver);

	// Get the mask and weights images
	deconvolver->setMask(DeconvolverHelpers::getArrayFromImage("solver.Basisfunction.mask", parset));
	deconvolver->setWeight(DeconvolverHelpers::getArrayFromImage("solver.Basisfunction.weight", parset));
        
        // Now configure the deconvolver
	LOFAR::ParameterSet subset(parset.makeSubset("solver.Basisfunction"));
	deconvolver->configure(parset);

	// Now set up controller
	boost::shared_ptr<DeconvolverControl<Float> > controller(new DeconvolverControl<Float>());
        ASKAPASSERT(controller);
	controller->configure(parset);
	deconvolver->setControl(controller);

	// Now set up monitor
	boost::shared_ptr<DeconvolverMonitor<Float> > monitor(new DeconvolverMonitor<Float>());
        ASKAPASSERT(monitor);
	monitor->configure(parset);
	deconvolver->setMonitor(monitor);

      }
      else if(parset.getString("solver", "Clean")=="Clean") {
        ASKAPLOG_INFO_STR(logger, "Constructing Clean deconvolver");
        string algorithm=parset.getString("solver.Clean.algorithm","Hogbom");
        
        if (algorithm=="Hogbom"){
          ASKAPLOG_INFO_STR(logger, "Constructing Hogbom Clean deconvolver");
          deconvolver=boost::shared_ptr<DeconvolverBase<Float, Complex> >(new DeconvolverHogbom<Float, Complex>(dirty, psf));
        }
        ASKAPASSERT(deconvolver);
        // Get the mask and weights images
        deconvolver->setMask(DeconvolverHelpers::getArrayFromImage("mask", parset));
        deconvolver->setWeight(DeconvolverHelpers::getArrayFromImage("weight", parset));
        
        // Now configure the deconvolver
	LOFAR::ParameterSet subset(parset.makeSubset("solver.Clean"));
	deconvolver->configure(parset);

	// Now set up controller
	boost::shared_ptr<DeconvolverControl<Float> > controller(new DeconvolverControl<Float>());
        ASKAPASSERT(controller);
	controller->configure(parset);
	deconvolver->setControl(controller);

	// Now set up monitor
	boost::shared_ptr<DeconvolverMonitor<Float> > monitor(new DeconvolverMonitor<Float>());
        ASKAPASSERT(monitor);
	monitor->configure(parset);
	deconvolver->setMonitor(monitor);

      }
      ASKAPASSERT(deconvolver);
      return deconvolver;
      
    }
  }

}
