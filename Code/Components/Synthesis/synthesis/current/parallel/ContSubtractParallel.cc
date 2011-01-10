/// @file
///
/// ContSubtractParallel: Support for parallel continuum subtraction using model
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <parallel/ContSubtractParallel.h>
#include <parallel/SimParallel.h>
#include <fitting/NormalEquationsStub.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>
#include <measurementequation/ImageFFTEquation.h>
#include <fitting/Equation.h>
#include <measurementequation/ComponentEquation.h>
#include <askap/AskapError.h>
#include <measurementequation/SynthesisParamsHelper.h>

// logging stuff
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");


using namespace askap;
using namespace askap::synthesis;

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// The command line inputs are needed solely for MPI - currently no
/// application specific information is passed on the command line.
/// @param comms communication object 
/// @param parset ParameterSet for inputs
ContSubtractParallel::ContSubtractParallel(askap::mwbase::AskapParallel& comms,
      const LOFAR::ParameterSet& parset) : MEParallelApp(comms,parset)
{
  // the stub allows to reuse MEParallelApp code although we're not solving
  // for the normal equations here
  itsNe.reset(new scimath::NormalEquationsStub);
  
  itsModelReadByMaster = parset.getBool("modelReadByMaster", true);  
}

/// @brief Initialise continuum subtractor
/// @details The parameters are taken from the parset file supplied in the constructor.
/// This method does initialisation which may involve communications in the parallel case
/// (i.e. distribution of the models between workers). Technically, we could've done this in 
/// the constructor.
void ContSubtractParallel::init()
{
  if (itsComms.isMaster()) {
      if (itsModelReadByMaster) {
          readModels();
          broadcastModel();
      }
  }
  if (itsComms.isWorker()) {
      if (itsModelReadByMaster) {
          receiveModel();
      } else {
          readModels();
      }
  }
}

/// @brief initialise measurement equation
/// @details This method initialises measurement equation for the given measurement set
/// @param[in] ms measurement set name
void ContSubtractParallel::initMeasurementEquation(const std::string &ms)
{
   ASKAPLOG_INFO_STR(logger, "Creating measurement equation" );
   TableDataSource ds(ms, TableDataSource::WRITE_PERMITTED, dataColumn());
   ds.configureUVWMachineCache(uvwMachineCacheSize(),uvwMachineCacheTolerance());      
   IDataSelectorPtr sel=ds.createSelector();
   sel << parset();
   IDataConverterPtr conv=ds.createConverter();
   conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
       "Hz");
   conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
   IDataSharedIter it=ds.createIterator(sel, conv);
   ASKAPCHECK(itsModel, "Model is not defined");
   ASKAPCHECK(gridder(), "Gridder is not defined");
   
   // a part of the equation defined via image
   askap::scimath::Equation::ShPtr imgEquation;
   
   if (SynthesisParamsHelper::hasImage(itsModel)) {
       ASKAPLOG_INFO_STR(logger, "Sky model contains at least one image, building an image-specific equation");
       // it should ignore parameters which are not applicable (e.g. components)
       imgEquation.reset(new ImageFFTEquation(*itsModel, it, gridder()));
   }

   // a part of the equation defined via components
   boost::shared_ptr<ComponentEquation> compEquation;

   if (SynthesisParamsHelper::hasComponent(itsModel)) {
       // model is a number of components
       ASKAPLOG_INFO_STR(logger, "Sky model contains at least one component, building a component-specific equation");
       // it doesn't matter which iterator is passed below. It is not used
       // it should ignore parameters which are not applicable (e.g. images)
       compEquation.reset(new ComponentEquation(*itsModel, it));
   }

   if (imgEquation && !compEquation) {
       ASKAPLOG_INFO_STR(logger, "Pure image-based model (no components defined)");
       itsEquation = imgEquation;
   } else if (compEquation && !imgEquation) {
       ASKAPLOG_INFO_STR(logger, "Pure component-based model (no images defined)");
       itsEquation = compEquation;
   } else if (imgEquation && compEquation) {
       ASKAPLOG_INFO_STR(logger, "Making a sum of image-based and component-based equations");
       itsEquation = imgEquation;
       SimParallel::addEquation(itsEquation, compEquation, it);
   } else {
       ASKAPTHROW(AskapError, "No sky models are defined");
   }
}



