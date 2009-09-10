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

#include <measurementequation/SynthesisParamsHelper.h>

// RVU
#include <gridding/VisWeightsMultiFrequency.h>

//#include <casa/OS/DynLib.h>        // for dynamic library loading
#include <gridding/DynLib.h>        // for dynamic library loading
#include <casa/BasicSL/String.h>   // for downcase


namespace askap {
namespace synthesis {

  // Define the static registry.
  std::map<std::string, VisGridderFactory::GridderCreator*>
  VisGridderFactory::theirRegistry;


  VisGridderFactory::VisGridderFactory() {
  }

  void VisGridderFactory::registerGridder (const std::string& name,
                                           VisGridderFactory::GridderCreator* creatorFunc)
  {
    ASKAPLOG_INFO_STR(logger, "     - Adding gridder "<<name<<" to the registry");
    theirRegistry[name] = creatorFunc;
  }

  IVisGridder::ShPtr VisGridderFactory::createGridder (const std::string& name,
                                    const LOFAR::ParameterSet& parset)
  {
    std::map<std::string,GridderCreator*>::const_iterator it = theirRegistry.find (name);
    if (it == theirRegistry.end()) {
      // Unknown gridder. Try to load the data manager from a dynamic library
      // with that lowercase name (without possible template extension).
      std::string tp(toLower(name));
      const std::string::size_type pos = tp.find ('<');
      if (pos != std::string::npos) {
        tp = tp.substr (0, pos);      // only take before <
      }
      // Try to load the dynamic library and execute its register function.
      // Do not dlclose the library.
      ASKAPLOG_INFO_STR(logger, "Gridder "<<name<<
                 " is not in the registry, attempting to load it dynamically");
      casa::DynLib dl(tp, string("libaskap_"), "register_"+tp, false);
      if (dl.getHandle()) {
        // Successfully loaded. Get the creator function.
        ASKAPLOG_INFO_STR(logger, "Dynamically loaded gridder " << name);
        // the first thing the gridder in the shared library is supposed to do is to
        // register itself. Therefore, its name will appear in the registry.
        it = theirRegistry.find (name);
      }
    }
    if (it == theirRegistry.end()) {
      ASKAPTHROW(AskapError, "Unknown gridder " << name);
    }
    // Execute the registered function.
    return it->second(parset);
  }

  // Make the gridder object for the gridder given in the parset file.
  // Currently the standard gridders are still handled by this function.
  // In the (near) future it should be done by putting creator functions
  // for these gridders in the registry and use that.
IVisGridder::ShPtr VisGridderFactory::make(const LOFAR::ParameterSet &parset) {
    if (theirRegistry.size() == 0) {
        // this is the first call of the method, we need to fill the registry with
        // all pre-defined gridders
        ASKAPLOG_INFO_STR(logger, "Filling the gridder registry with pre-defined gridders");
    }
    
	// buffer for the result
	IVisGridder::ShPtr gridder;
	/// @todo Better handling of string case
        string gridderName = parset.getString("gridder");
	if (gridderName == "WProject") {
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
	} else if (gridderName == "WStack") {
		double wmax=parset.getDouble("gridder.WStack.wmax", 35000.0);
		int nwplanes=parset.getInt32("gridder.WStack.nwplanes", 65);
		ASKAPLOG_INFO_STR(logger, "Gridding using W stacking ");
		gridder=IVisGridder::ShPtr(new WStackVisGridder(wmax, nwplanes));
	} else if (gridderName == "AWProject") {
	    gridder = AWProjectVisGridder::createGridder(parset.makeSubset("gridder.AWProject."));	    
	} else if (gridderName == "AProjectWStack") {
	    gridder = AProjectWStackVisGridder::createGridder(parset.makeSubset("gridder.AProjectWStack."));	    
	} else if (gridderName == "Box") {
		ASKAPLOG_INFO_STR(logger, "Gridding with Box function");
		gridder = BoxVisGridder::createGridder(parset.makeSubset("gridder.Box."));
	} else if (gridderName == "SphFunc") {
		ASKAPLOG_INFO_STR(logger, "Gridding with spheriodal function");
		gridder = SphFuncVisGridder::createGridder(parset.makeSubset("gridder.SphFunc."));
	} else {
            gridder = createGridder (gridderName, parset);
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
	} else {
            ASKAPLOG_INFO_STR(logger,"No padding at the gridder level");
        }
	if (parset.isDefined("gridder.alldatapsf")) {
	    const bool useAll = parset.getBool("gridder.alldatapsf");
	    if (useAll) {
	        ASKAPLOG_INFO_STR(logger, "Use all data for PSF calculations instead of the representative feed and field");
	    } else {
	        ASKAPLOG_INFO_STR(logger, "Use representative feed and field for PSF calculation");
	    }
	    boost::shared_ptr<TableVisGridder> tvg = 
	        boost::dynamic_pointer_cast<TableVisGridder>(gridder);
	    ASKAPCHECK(tvg, "Gridder type ("<<parset.getString("gridder")<<
	               ") is incompatible with the alldatapsf option");
	    tvg->useAllDataForPSF(useAll);
	} else {
	    ASKAPLOG_INFO_STR(logger, "gridder.alldatapsf option is not used, default to representative feed and field for PSF calculation");
	}	
	
	// Initialize the Visibility Weights
	if (parset.getString("visweights","")=="MFS")
	{
            double reffreq=parset.getDouble("visweights.MFS.reffreq", 1.405e+09);
	    ASKAPLOG_INFO_STR(logger, "Initialising for MFS with reference frequency " << reffreq << " Hz");
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
