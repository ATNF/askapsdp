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
	    illum->simulateTapering(0.);
	    illum->simulateFeedLegShadows(1.8,M_PI/4.,0.75);
	    illum->simulateFeedLegWedges(0.6,0.5,M_PI/12.,3.5);
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
		string tablename=parset.getString("gridder.WProject.tablename", "");
		ASKAPLOG_INFO_STR(logger, "Gridding using W projection");
		gridder=IVisGridder::ShPtr(new WProjectVisGridder(wmax, nwplanes, cutoff, oversample,
				maxSupport, tablename));
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
				maxSupport, maxFeeds, maxFields, pointingTol, freqDep, tablename));
	} else if (parset.getString("gridder")=="AProjectWStack") {
		double pointingTol=parset.getDouble("gridder.AProjectWStack.pointingtolerance", 0.0001);
		double wmax=parset.getDouble("gridder.AProjectWStack.wmax", 10000.0);
		int nwplanes=parset.getInt32("gridder.AProjectWStack.nwplanes", 65);
		int oversample=parset.getInt32("gridder.AProjectWStack.oversample", 8);
		int maxSupport= parset.getInt32("gridder.AProjectWStack.maxsupport",
				128);
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
				maxSupport, maxFeeds, maxFields, pointingTol, freqDep, tablename));
	} else if (parset.getString("gridder")=="Box") {
		ASKAPLOG_INFO_STR(logger, "Gridding with Box function");
		gridder=IVisGridder::ShPtr(new BoxVisGridder());
	} else {
		ASKAPLOG_INFO_STR(logger, "Gridding with spheriodal function");
		gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
	}
	
	// Initialize the Visibility Weights
	if (parset.getString("visweights","")=="MFS")
	{
		double reffreq=parset.getDouble("visweights.MFS.reffreq", 1.405e+09);
		gridder->initVisWeights(IVisWeights::ShPtr(new VisWeightsMultiFrequency(reffreq)));
	}
	else // Null....
	{
		gridder->initVisWeights(IVisWeights::ShPtr(new VisWeightsMultiFrequency()));
	}
	return gridder;
}
}
}
