/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

// Include own header file first
#include <parallel/SynParallel.h>
#include <measurementequation/SynthesisParamsHelper.h>


#include <sstream>

#include <mwcommon/MPIConnection.h>
#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/MWIos.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

#include <askap/AskapError.h>
#include <askap_synthesis.h>

using namespace std;
using namespace askap;
using namespace askap::mwbase;
using namespace askap::scimath;

namespace askap
{
  namespace synthesis
  {

    SynParallel::SynParallel(askap::mwbase::AskapParallel& comms, const LOFAR::ParameterSet& parset) : 
                         itsComms(comms), itsParset(parset)
    {
      itsModel.reset(new Params());
      ASKAPCHECK(itsModel, "Model not defined correctly");
    }

    SynParallel::~SynParallel()
    {
    }

    askap::scimath::Params::ShPtr& SynParallel::params()
    {
      return itsModel;
    }

    // Send the model to all workers
    void SynParallel::broadcastModel()
    {
      if (itsComms.isParallel() && itsComms.isMaster())
      {
        ASKAPCHECK(itsModel, "Model not defined prior to broadcast")
        casa::Timer timer;
        timer.mark();

        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("model", 1);
        out << *itsModel;
        out.putEnd();
        itsComms.connectionSet()->writeAll(bs);
        ASKAPLOG_INFO_STR(logger, "Sent model to the workers via MPI in "<< timer.real()
                           << " seconds ");
      }
    }

    // Receive the model from the master
    void SynParallel::receiveModel()
    {
      if (itsComms.isParallel() && itsComms.isWorker())
      {
        ASKAPCHECK(itsModel, "Model not defined prior to receiving")
        casa::Timer timer;
        timer.mark();
        ASKAPLOG_INFO_STR(logger, "Wait to receive the model from the master via MPI");
        LOFAR::BlobString bs;
        bs.resize(0);
        itsComms.connectionSet()->read(0, bs);
        LOFAR::BlobIBufString bib(bs);
        LOFAR::BlobIStream in(bib);
        int version=in.getStart("model");
        ASKAPASSERT(version==1);
        in >> *itsModel;
        in.getEnd();
        ASKAPLOG_INFO_STR(logger, "Received model from the master via MPI in "<< timer.real()
                           << " seconds ");
      }
    }

    std::string SynParallel::substitute(const std::string& s) const
    {
       return itsComms.substitute(s);
    }
    
    /// @brief read the models from parset file to the given params object
    /// @details The model can be composed from both images and components. This
    /// method populates Params object by adding model data read from the parset file.
    /// The model is given by shared pointer because the same method can be used for both
    /// simulations and calibration (the former populates itsModel, the latter populates
    /// itsPerfectModel) 
    /// @param[in] pModel shared pointer to the params object (must exist)
    void SynParallel::readModels(const scimath::Params::ShPtr &pModel) const
    {
      ASKAPCHECK(pModel, "model is not initialised prior to call to SynParallel::readModels");
      
      LOFAR::ParameterSet parset(itsParset);
  
      if (itsParset.isDefined("sources.definition")) {
          parset = LOFAR::ParameterSet(substitute(itsParset.getString("sources.definition")));
      }
      
      const std::vector<std::string> sources = parset.getStringVector("sources.names");
      for (size_t i=0; i<sources.size(); ++i) {
	       const std::string modelPar = std::string("sources.")+sources[i]+".model";
	       const std::string compPar = std::string("sources.")+sources[i]+".components";
	       // check that only one is defined
	       ASKAPCHECK(parset.isDefined(compPar) != parset.isDefined(modelPar),
	            "The model should be defined with either image (via "<<modelPar<<") or components (via "<<
	             compPar<<"), not both");
	       // 
           if (parset.isDefined(modelPar)) {
               const std::string model=substitute(parset.getString(modelPar));
               ASKAPLOG_INFO_STR(logger, "Adding image " << model << " as model for "<< sources[i] );
               const std::string paramName = "image.i."+sources[i];
               SynthesisParamsHelper::loadImageParameter(*pModel, paramName, model);
           } else {
               // loop through components
               ASKAPLOG_INFO_STR(logger, "Adding components as model for "<< sources[i] );
               const vector<string> compList = parset.getStringVector(compPar);
               for (vector<string>::const_iterator cmp = compList.begin(); cmp != compList.end(); ++cmp) {
                    ASKAPLOG_INFO_STR(logger, "Loading component " << *cmp << " as part of the model for " << sources[i]);
                    SynthesisParamsHelper::copyComponent(pModel, parset,*cmp,"sources.");
                }
           }
      }
      ASKAPLOG_INFO_STR(logger, "Successfully read models");      
    }
  }
}
