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
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <gridding/VisGridderFactory.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/WProjectVisGridder.h>
#include <gridding/AWProjectVisGridder.h>
#include <gridding/WStackVisGridder.h>
#include <gridding/AProjectWStackVisGridder.h>
#include <gridding/DiskIllumination.h>
#include <gridding/ATCAIllumination.h>

#include <measurementequation/SynthesisParamsHelper.h>

// RVU
#include <gridding/VisWeightsMultiFrequency.h>

using namespace LOFAR::ACC::APS;
namespace askap {
namespace synthesis {

VisGridderFactory::VisGridderFactory() {
}

VisGridderFactory::~VisGridderFactory() {
}

/// @brief a helper factory of illumination patterns
/// @details Illumination model is required for a number of gridders. This
/// method allows to avoid duplication of code and encapsulates all 
/// functionality related to illumination patterns. 
/// @param[in] parset ParameterSet containing description of illumination to use
/// @return shared pointer to illumination interface
boost::shared_ptr<IBasicIllumination> 
VisGridderFactory::makeIllumination(const LOFAR::ACC::APS::ParameterSet &parset)
{
   const std::string illumType = parset.getString("illumination", "disk");
   const double diameter=parset.getDouble("diameter");
   const double blockage=parset.getDouble("blockage");

   if (illumType == "disk") {
   	    ASKAPLOG_INFO_STR(logger,
					"Using disk illumination model, diameter="<<
					diameter<<" metres, blockage="<<blockage<<" metres");
   
       	return boost::shared_ptr<IBasicIllumination>(new DiskIllumination(diameter,blockage));
   } else if (illumType == "ATCA") {
   	    ASKAPLOG_INFO_STR(logger,
					"Using ATCA illumination model, diameter="<<
					diameter<<" metres, blockage="<<blockage<<" metres");

   	    boost::shared_ptr<ATCAIllumination> illum(new ATCAIllumination(diameter,blockage)); 
   	    ASKAPDEBUGASSERT(illum);
   	    if (parset.getBool("illumination.tapering", true)) {
   	        const double maxDefocusingPhase =
   	             SynthesisParamsHelper::convertQuantity(parset.getString("illumination.tapering.defocusing",
   	                           "0rad"),"rad");
	        illum->simulateTapering(maxDefocusingPhase);
	        ASKAPLOG_INFO_STR(logger,"Tapering of the illumination is simulated, maximum defocusing phase = "<<
	                  maxDefocusingPhase/M_PI*180.<<" deg."); 
	    } else {
	        ASKAPLOG_INFO_STR(logger,"Tapering of the illumination is not simulated");
	    }
	    if (parset.getBool("illumination.feedlegs", true)) {
	        const double width = SynthesisParamsHelper::convertQuantity(
	           parset.getString("illumination.feedlegs.width","1.8m"),"m");
	        const double rotation = SynthesisParamsHelper::convertQuantity(
	           parset.getString("illumination.feedlegs.rotation","45deg"),"rad");   
	        const double shadowingFactor = 
	           parset.getDouble("illumination.feedlegs.shadowing",0.75);   
	        illum->simulateFeedLegShadows(width,rotation,shadowingFactor);
	        ASKAPLOG_INFO_STR(logger,"Feed legs are simulated. Width = "<<width<<" metres, rotated at "<<
	           rotation/M_PI*180.<<" deg, shadowing factor (how much attenuation caused) = "<<shadowingFactor);
	        if (parset.getBool("illumination.feedlegs.wedges", true)) {
	            const double defaultWedgeShadowing[2] = {0.6,0.5};
	            std::vector<double> wedgeShadowing = 
	                parset.getDoubleVector("illumination.feedlegs.wedges.shadowing", 
	                std::vector<double>(defaultWedgeShadowing,defaultWedgeShadowing+2));
	            const double angle = SynthesisParamsHelper::convertQuantity(
	                parset.getString("illumination.feedlegs.wedges.angle","15deg"),"rad");    
	            const double startRadius = SynthesisParamsHelper::convertQuantity(
	                parset.getString("illumination.feedlegs.wedges.startradius","3.5m"),"m");
	            ASKAPCHECK(wedgeShadowing.size() && wedgeShadowing.size()<3, 
	                 "illumination.feedlegs.wedges.shadowing can have either 1 or 2 elements only, "
	                 "you have "<<wedgeShadowing.size());
	            if (wedgeShadowing.size() == 1) {
	                wedgeShadowing.push_back(wedgeShadowing[0]);
	            }     
	            ASKAPDEBUGASSERT(wedgeShadowing.size() == 2);    
          	    illum->simulateFeedLegWedges(wedgeShadowing[0],wedgeShadowing[1],angle,startRadius);	            
          	    ASKAPLOG_INFO_STR(logger,"Feed leg wedges are simulated. Shadowing factors are "<<
          	           wedgeShadowing<<", opening angle is "<<angle/M_PI*180.<<" deg, start radius is "<<
          	           startRadius<<" metres");
	        } else {
	            ASKAPLOG_INFO_STR(logger,"Feed leg wedges are not simulated.");
	        }
	    } else {
	       ASKAPLOG_INFO_STR(logger,"Feed legs are not simulated.");
	    }
	    return illum;
   }
   
   ASKAPTHROW(AskapError, "Unknown illumination type "<<illumType);
   return boost::shared_ptr<IBasicIllumination>(); // to keep the compiler happy
}

IVisGridder::ShPtr VisGridderFactory::make(
		const LOFAR::ACC::APS::ParameterSet &parset) {
	IVisGridder::ShPtr gridder;
	/// @todo Better handling of string case
	if (parset.getString("gridder")=="WProject") {
		double wmax=parset.getDouble("gridder.WProject.wmax", 35000.0);
		int nwplanes=parset.getInt32("gridder.WProject.nwplanes", 65);
		double cutoff=parset.getDouble("gridder.WProject.cutoff", 1e-3);
		int oversample=parset.getInt32("gridder.WProject.oversample", 8);
		int maxSupport=parset.getInt32("gridder.WProject.maxsupport", 256);
		int limitSupport=parset.getInt32("gridder.WProject.limitsupport", 0);
		string tablename=parset.getString("gridder.WProject.tablename", "");
		ASKAPLOG_INFO_STR(logger, "Gridding using W projection");
		gridder=IVisGridder::ShPtr(new WProjectVisGridder(wmax, nwplanes, cutoff, oversample,
								  maxSupport, limitSupport, tablename));
	} else if (parset.getString("gridder")=="WStack") {
		double wmax=parset.getDouble("gridder.WStack.wmax", 35000.0);
		int nwplanes=parset.getInt32("gridder.WStack.nwplanes", 65);
		ASKAPLOG_INFO_STR(logger, "Gridding using W stacking ");
		gridder=IVisGridder::ShPtr(new WStackVisGridder(wmax, nwplanes));
	} else if (parset.getString("gridder")=="AWProject") {
		double pointingTol=parset.getDouble("gridder.AProjectWStack.pointingtolerance", 0.0001);
		double wmax=parset.getDouble("gridder.AWProject.wmax", 10000.0);
		int nwplanes=parset.getInt32("gridder.AWProject.nwplanes", 65);
		double cutoff=parset.getDouble("gridder.AWProject.cutoff", 1e-3);
		int oversample=parset.getInt32("gridder.AWProject.oversample", 8);
		int maxSupport=parset.getInt32("gridder.AWProject.maxsupport", 128);
		int limitSupport=parset.getInt32("gridder.AWProject.limitsupport", 0);
		bool freqDep=parset.getBool("gridder.AWProject.frequencydependent",
				true);
		int maxFeeds=parset.getInt32("gridder.AWProject.maxfeeds", 1);
		int maxFields=parset.getInt32("gridder.AWProject.maxfields", 1);
		string tablename=parset.getString("gridder.AWProject.tablename", "");
		ASKAPLOG_INFO_STR(logger,
				"Gridding with Using Antenna Illumination and W projection");
		if (freqDep) {
			ASKAPLOG_INFO_STR(logger,
					"Antenna illumination scales with frequency");
		} else {
			ASKAPLOG_INFO_STR(logger,
					"Antenna illumination independent of frequency");
		}
				
		gridder=IVisGridder::ShPtr(new AWProjectVisGridder(
		        makeIllumination(parset.makeSubset("gridder.AWProject.")),
				wmax, nwplanes, cutoff, oversample,
				maxSupport, limitSupport, maxFeeds, maxFields, pointingTol,
			        freqDep, tablename));
	} else if (parset.getString("gridder")=="AProjectWStack") {
		double pointingTol=parset.getDouble("gridder.AProjectWStack.pointingtolerance", 0.0001);
		double wmax=parset.getDouble("gridder.AProjectWStack.wmax", 10000.0);
		int nwplanes=parset.getInt32("gridder.AProjectWStack.nwplanes", 65);
		int oversample=parset.getInt32("gridder.AProjectWStack.oversample", 8);
		int maxSupport= parset.getInt32("gridder.AProjectWStack.maxsupport",
				128);
		int limitSupport= parset.getInt32("gridder.AProjectWStack.limitsupport",
						  0);
		int maxFeeds=parset.getInt32("gridder.AProjectWStack.maxfeeds", 1);
		int maxFields=parset.getInt32("gridder.AProjectWStack.maxfields", 1);
		bool freqDep=parset.getBool(
				"gridder.AProjectWStack.frequencydependent", true);
		string tablename=parset.getString("gridder.AProjectWStack.tablename",
				"");
		ASKAPLOG_INFO_STR(logger,
				"Gridding with Antenna Illumination projection and W stacking");
		if (freqDep) {
			ASKAPLOG_INFO_STR(logger,
					"Antenna illumination scales with frequency");
		} else {
			ASKAPLOG_INFO_STR(logger,
					"Antenna illumination independent of frequency");
		}

		gridder=IVisGridder::ShPtr(new AProjectWStackVisGridder(
		        makeIllumination(parset.makeSubset("gridder.AProjectWStack.")),
				wmax, nwplanes, oversample,
			        maxSupport, limitSupport, maxFeeds, maxFields, pointingTol,
			        freqDep, tablename));
	} else if (parset.getString("gridder")=="Box") {
		ASKAPLOG_INFO_STR(logger, "Gridding with Box function");
		gridder=IVisGridder::ShPtr(new BoxVisGridder());
	} else {
		ASKAPLOG_INFO_STR(logger, "Gridding with spheriodal function");
		gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
	}
	ASKAPASSERT(gridder);
	if (parset.isDefined("gridder.padding")) {
	    const int padding =parset.getInt32("gridder.padding");
	    ASKAPLOG_INFO_STR(logger, "Use padding at the gridder level, padding factor = "<<padding);
	    boost::shared_ptr<TableVisGridder> tvg = 
	        boost::dynamic_pointer_cast<TableVisGridder>(gridder);
	    ASKAPCHECK(tvg, "Gridder type ("<<parset.getString("gridder")<<
	               ") is incompatible with the padding option");
	    tvg->setPaddingFactor(padding);           
	}
	
	// Initialize the Visibility Weights
	if (parset.getString("visweights","")=="MFS")
	{
		double reffreq=parset.getDouble("visweights.MFS.reffreq", 1.405e+09);
		gridder->initVisWeights(IVisWeights::ShPtr(new VisWeightsMultiFrequency(reffreq)));
	}
	else // Null....
	{
	  //		gridder->initVisWeights(IVisWeights::ShPtr(new VisWeightsMultiFrequency()));
	}
	return gridder;
}
}
}
