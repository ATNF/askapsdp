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
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <measurementequation/ImageSolverFactory.h>

#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <measurementequation/ImageMSMFSolver.h>
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianTaperPreconditioner.h>
#include <measurementequation/SynthesisParamsHelper.h>

using namespace askap::scimath;

using namespace LOFAR::ACC::APS;

namespace askap
{
  namespace synthesis
  {

    ImageSolverFactory::ImageSolverFactory()
    {
    }

    ImageSolverFactory::~ImageSolverFactory()
    {
    }
    
    Solver::ShPtr ImageSolverFactory::make(askap::scimath::Params &ip, const LOFAR::ACC::APS::ParameterSet &parset) {
      ImageSolver::ShPtr solver;
      if(parset.getString("solver")=="Clean") {
         std::vector<float> defaultScales(3);
         defaultScales[0]=0.0;
         defaultScales[1]=10.0;
         defaultScales[2]=30.0;
        
	     string algorithm=parset.getString("solver.Clean.algorithm","MultiScale");
	     std::vector<float> scales=parset.getFloatVector("solver.Clean.scales", defaultScales);
	
	     if(algorithm=="MSMFS"){
            int nterms=parset.getInt32("solver.Clean.nterms",2);
            solver = ImageSolver::ShPtr(new ImageMSMFSolver(ip, casa::Vector<float>(scales),int(nterms)));
            ASKAPLOG_INFO_STR(logger, "Constructed image multiscale multi-frequency solver" );
            solver->setAlgorithm(algorithm);
	     } else {
            solver = ImageSolver::ShPtr(new ImageMultiScaleSolver(ip, casa::Vector<float>(scales)));
            ASKAPLOG_INFO_STR(logger, "Constructed image multiscale solver" );
            //solver->setAlgorithm(algorithm);
            solver->setAlgorithm(parset.getString("solver.Clean.algorithm", "MultiScale"));
	     }
        
	     solver->setTol(parset.getFloat("solver.Clean.tolerance", 0.1));
         solver->setGain(parset.getFloat("solver.Clean.gain", 0.7));
         solver->setVerbose(parset.getBool("solver.Clean.verbose", true));
         solver->setNiter(parset.getInt32("solver.Clean.niter", 100));
         casa::Quantity threshold;
         casa::Quantity::read(threshold, parset.getString("solver.Clean.threshold", "0Jy"));
         solver->setThreshold(threshold);
         if (parset.isDefined("threshold.minorcycle")) {
             boost::shared_ptr<ImageCleaningSolver> ics = 
                   boost::dynamic_pointer_cast<ImageCleaningSolver>(solver);
             if (ics) {
                 const double fractionalThreshold = SynthesisParamsHelper::convertQuantity(
	                            parset.getString("threshold.minorcycle"),"");
                 ics->setFractionalThreshold(fractionalThreshold);
	             ASKAPLOG_INFO_STR(logger, "Will stop minor cycle at the fractional threshold of "<<fractionalThreshold);
             } else {
                 ASKAPLOG_INFO_STR(logger, "The type of the image solver used does not allow to specify "
                            "a fractional threshold, ignoring threshold.minorcycle = "<<parset.getString("threshold.minorcycle"));
             }      
         }
      } else {
          solver = ImageSolver::ShPtr(new ImageSolver(ip));
          casa::Quantity threshold;
          casa::Quantity::read(threshold, parset.getString("solver.Dirty.threshold", "0Jy"));
          solver->setTol(parset.getFloat("solver.Dirty.tolerance", 0.1));
          solver->setThreshold(threshold);
          ASKAPLOG_INFO_STR(logger, "Constructed dirty image solver" );
      }
      
      // Set Up the Preconditioners - a whole list of 'em
      // Any changes here must also be copied to ImagerParallel
      const vector<string> preconditioners=parset.getStringVector("preconditioner.Names",std::vector<std::string>());
      if(preconditioners.size())
      {
        for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) 
        {
          if ( (*pc)=="Wiener" ) {
	          float noisepower = parset.getFloat("preconditioner.Wiener.noisepower",0.0);
              solver->addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner(noisepower)));
	      }
	      if ( (*pc) == "GaussianTaper") {
	          // at this stage we have to define tapers in uv-cells, rather than in klambda
	          // because the physical cell size is unknown to solver factory. 
	          // Theoretically we could parse the parameters here and extract the cell size and
	          // shape, but it can be defined separately for each image. We need to find
	          // the way of dealing with this complication.
	          ASKAPCHECK(parset.isDefined("preconditioner.GaussianTaper"), 
	                "preconditioner.GaussianTaper showwing the taper size should be defined to use GaussianTaper");
	          const vector<double> taper = SynthesisParamsHelper::convertQuantity(
	                 parset.getStringVector("preconditioner.GaussianTaper"),"rad");
	          ASKAPCHECK((taper.size() == 3) || (taper.size() == 1), 
	                     "preconditioner.GaussianTaper can have either single element or "
	                     " a vector of 3 elements. You supplied a vector of "<<taper.size()<<" elements");     
	          ASKAPCHECK(parset.isDefined("Images.shape") && parset.isDefined("Images.cellsize"),
	                 "Imager.shape and Imager.cellsize should be defined to convert the taper fwhm specified in "
	                 "angular units in the image plane into uv cells");
	          const std::vector<double> cellsize = SynthesisParamsHelper::convertQuantity(
	                    parset.getStringVector("Images.cellsize"),"rad");
	          const std::vector<int> shape = parset.getInt32Vector("Images.shape");
	          ASKAPCHECK((cellsize.size() == 2) && (shape.size() == 2), 
	              "Images.cellsize and Images.shape parameters should have exactly two values");
	          // factors which appear in nominator are effectively half sizes in radians
	          const double xFactor = cellsize[0]*double(shape[0])/2.;
	          const double yFactor = cellsize[1]*double(shape[1])/2.;
	          
	          if (taper.size() == 3) {

	              ASKAPDEBUGASSERT((taper[0]!=0) && (taper[1]!=0));              
	              solver->addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(
	                     xFactor/taper[0],yFactor/taper[1],taper[2])));	                         
	          } else {
	              ASKAPDEBUGASSERT(taper[0]!=0);              	              
                  if (std::abs(xFactor-yFactor)<4e-15) {
                      // the image is square, can use the short cut
	                  solver->addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0])));	                                
                  } else {
                      // the image is rectangular. Although the gaussian taper is symmetric in
                      // angular coordinates, it will be elongated along the vertical axis in 
                      // the uv-coordinates.
	                  solver->addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0],
	                         yFactor/taper[0],0.)));	                                                      
                  } 
	          }          
	      }
	  /*
	     if( (*pc)=="ApproxPsf" ) //later, add the option of specifying a beam, or fitting for it.
	     {
	     solver->addPreconditioner(IImagePreconditioner::ShPtr(new ApproxPsfPreconditioner()));
	     }
	   */
	}
      }
      else
      {
        solver->addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner()));
      }
      
      return solver;
    }
  }
}
