/// @file
/// 
/// @brief Measurement equation adapter filling calibration parameters on demand
/// @details This is an adapter intended to be used when calibration effects
/// are simulated. It implements IMeasurementEquation interface, but only
/// predict method is expected to be used. An exception is thrown if 
/// one requests normal equations to be computed with this class. The predict
/// method checks whether new antenna/beam combinations are present in the 
/// current visibility chunk. It then creates new parameters (this class follows
/// the name convension enforced by accessors::CalParamNameHelper, but this 
/// behavior can be overridden in derived classes, if necessary) and updates
/// the existing ones using calibration solution source supplied at construction.
/// After parameters are created, the predict method of the wrapped measurement
/// equation is called to predict visibilities. 
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

// own includes
#include <measurementequation/CalibParamsMEAdapter.h>
#include <askap/AskapError.h>
#include <fitting/Equation.h>

namespace askap {

namespace synthesis {

/// @brief standard constructor
/// @details initialises the class with the given solution source and iterator
/// (if necessary). It also initialises the shared pointer to the slave measurement 
/// equation which does the actual prediction of visibilities. Only accessor-based
/// functionality of this slave measurement equation is used.
/// @param[in] ime shared pointer to the slave measurement equation
/// @param[in] css shared pointer to solution source
/// @param[in] idi data iterator (if iterator-based interface is required)
CalibParamsMEAdapter::CalibParamsMEAdapter(const boost::shared_ptr<IMeasurementEquation> &ime,
                const boost::shared_ptr<accessors::ICalSolutionConstSource> &css, 
                const accessors::IDataSharedIter& idi) :                 
                MultiChunkEquation(idi), CalibrationSolutionHandler(css), itsSlaveME(ime) 
{
  ASKAPCHECK(itsSlaveME, "An attempt to initialise CalibParamsMEAdapter with a void shared pointer to the slave ME");
  // deliberately reuse shared pointer between the slaved measurement equation and this adapter to
  // ensure reference semantics for parameters if we could. It is not clear what to do if we can't extract the
  // parameters. Throwing an exception for now because it is not our intended use case.
  boost::shared_ptr<scimath::Equation> eqn = boost::dynamic_pointer_cast<scimath::Equation>(itsSlaveME);
  ASKAPCHECK(eqn, "Attempt to initialise CalibParamsMEAdapter with an incompatible type of slave measurement equation");
  rwParameters() = eqn->rwParameters();
}
   
/// @brief Predict model visibilities for one accessor (chunk).
/// @details This prediction is done for single chunk of data only.
/// This method overrides pure virtual method of the base class. 
/// @param[in] chunk a read-write accessor to work with
void CalibParamsMEAdapter::predict(accessors::IDataAccessor &chunk) const
{
  itsSlaveME->predict(chunk);
}
   
/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This method is not supposed to be used. It overrides
/// pure virtual method of the base class and throws an exception if called 
void CalibParamsMEAdapter::calcEquations(const accessors::IConstDataAccessor &,
                          askap::scimath::INormalEquations&) const
{
  ASKAPTHROW(AskapError, "CalibParamsMEAdapter::calcEquations is not supposed to be used");
}                          

/// @brief process parameters for a given antenna/beam   
/// @details This method encapsulates update of the parameters 
/// corresponding to the given antenna/beam pair according to 
/// the current calibration solution accessor. Override this
/// method for non-standard parameter names (use updateParameter
/// for the actual update).
/// @param[in] ant antenna index
/// @param[in] beam beam index
void CalibParamsMEAdapter::processAntBeamPair(const casa::uInt ant, const casa::uInt beam) const
{
}
   
/// @brief helper method to update a given parameter if necessary
/// @details This method checks whether the parameter is new and
/// adds or updates as required to the parameter class held by the
/// slave measurement equation.
/// @param[in] name parameter name
/// @param[in] val new value
void CalibParamsMEAdapter::updateParameter(const std::string &name, const casa::Complex &val) const
{
}


} // namespace synthesis

} // namespace askap


