/// @file
/// 
/// @brief Calibration effect: Mueller matrix filled with zeros
/// @details This is a simple effect which doesn't change anything
/// after an addition to another effect. It is mainly intended for debugging. 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ZERO_COMPONENT_H
#define ZERO_COMPONENT_H

// own includes
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/Params.h>
#include <dataaccess/IConstDataAccessor.h>
#include <askap/AskapError.h>
#include <measurementequation/MEComponent.h>


// std includes
#include <string>

namespace askap {

namespace synthesis {


/// @brief Calibration effect: Mueller matrix filled with zeros
/// @details This is a simple effect which doesn't change anything
/// after an addition to another effect. It is mainly intended for debugging. 
/// @ingroup measurementequation
struct ZeroComponent : public MEComponent {
   
   /// @brief constructor, parameters are actually ignored
   /// @param[in] par const reference to parameters
   inline explicit ZeroComponent(const scimath::Params & par)  {}
   
   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary. 
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
/// template will call it when necessary. 
/// @param[in] chunk accessor to work with
/// @param[in] row row of the chunk to work with
/// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
/// this effect
inline scimath::ComplexDiffMatrix ZeroComponent::get(const IConstDataAccessor &chunk, 
                                      casa::uInt) const
{
  return scimath::ComplexDiffMatrix(chunk.nPol(), chunk.nPol(), 0.);
}

} // namespace synthesis

} // namespace askap



#endif // #ifndef ZERO_COMPONENT_H
