/// @file
/// 
/// @brief Calibration effect: Identity Mueller matrix.
/// @details This is a simple effect which doesn't change anything.
/// it is used mainly for debugging. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef IDENTITY_COMPONENT_H
#define IDENTITY_COMPONENT_H

// own includes
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/Params.h>
#include <dataaccess/IConstDataAccessor.h>
#include <conrad/ConradError.h>
#include <measurementequation/MEComponent.h>


// std includes
#include <string>

namespace conrad {

namespace synthesis {

/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
/// @ingroup measurementequation
struct IdentityComponent : public MEComponent {
   
   /// @brief constructor, parameters are actually ignored
   /// @param[in] par const reference to parameters
   inline explicit IdentityComponent(const scimath::Params &)  {}
   
   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary. It returns 
   /// @param[in] chunk accessor to work with
   /// @param[in] row row of the chunk to work with
   /// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
   /// this effect
   inline scimath::ComplexDiffMatrix get(const IConstDataAccessor &chunk, 
                                casa::uInt row) const;
};

/// @brief main method returning Mueller matrix and derivatives
/// @details This method has to be overloaded (in the template sense) for
/// all classes representing various calibration effects. CalibrationME
/// template will call it when necessary. It returns 
/// @param[in] chunk accessor to work with
/// @param[in] row row of the chunk to work with
/// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
/// this effect
inline scimath::ComplexDiffMatrix IdentityComponent::get(const IConstDataAccessor &chunk, 
                                      casa::uInt row) const
{
   
   scimath::ComplexDiffMatrix calFactor(chunk.nPol(), chunk.nPol(), 0.);

   for (casa::uInt pol=0; pol<chunk.nPol(); ++pol) {            
        calFactor(pol,pol) = scimath::ComplexDiff(1.);            
   }
   return calFactor;
}

} // namespace synthesis

} // namespace conrad



#endif // #ifndef IDENTITY_COMPONENT_H
