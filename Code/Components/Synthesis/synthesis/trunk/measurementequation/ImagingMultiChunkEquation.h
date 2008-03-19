/// @file
/// 
/// @brief A structural type joining together ImagingEquation and MultiChunkEquation
/// @details Because we deal with the double inheritance here, we need to overload
/// explicitly predict and calcImagingEquation methods, otherwise the methods
/// in the scimath::Equation tree are left pure abstract. Theoretically, we can
/// get rid of the double inheritance here and the need of this class would 
/// disappear. However, MultiChunkEquation is envisaged to be a temporary
/// class before we start to work with accessors only. Therefore, the inheritance
/// is left double to highlight the future interface 
/// (derived from IMeasurementEquation).
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef IMAGING_MULTI_CHUNK_EQUATION_H
#define IMAGING_MULTI_CHUNK_EQUATION_H

// own include
#include <measurementequation/MultiChunkEquation.h>
#include <fitting/ImagingEquation.h>
#include <fitting/ImagingNormalEquations.h>


namespace askap {

namespace synthesis {


/// @brief A structural type joining together ImagingEquation and MultiChunkEquation
/// @details Because we deal with the double inheritance here, we need to overload
/// explicitly predict and calcImagingEquation methods, otherwise the methods
/// in the scimath::Equation tree are left pure abstract. Theoretically, we can
/// get rid of the double inheritance here and the need of this class would 
/// disappear. However, MultiChunkEquation is envisaged to be a temporary
/// class before we start to work with accessors only. Therefore, the inheritance
/// is left double to highlight the future interface 
/// (derived from IMeasurementEquation).
/// @ingroup measurementequation
struct ImagingMultiChunkEquation : virtual public MultiChunkEquation,
                                   virtual public askap::scimath::ImagingEquation    
{  
  /// @brief Standard constructor, which remembers data iterator.
  /// @param[in] idi data iterator
  ImagingMultiChunkEquation(const IDataSharedIter& idi);

  /// @brief Calculate the normal equations for the iterator
  /// @details This version iterates through all chunks of data and
  /// calls an abstract method declared in IMeasurementEquation for each 
  /// individual accessor (each iteration of the iterator). 
  /// @param[in] ne Normal equations
  virtual void calcImagingEquations(askap::scimath::ImagingNormalEquations& ne) const;

  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). This method overrides an abstract method
  /// of MultiChunkEquation. It calls calcImagingEquations(chunk,ne)  with 
  /// ne converted to ImagingNormalEquations
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcEquations(const IConstDataAccessor &chunk,
                          askap::scimath::INormalEquations& ne) const;
  
  /// @brief Calculate the normal equation for one accessor (chunk).
  /// @details This calculation is done for a single chunk of
  /// data only (one iteration).It seems that all measurement
  /// equations should work with accessors rather than iterators
  /// (i.e. the iteration over chunks should be moved to the higher
  /// level, outside this class). 
  /// This class translates calls to calcEquations(chunk, ne) to 
  /// this method and does type conversion.
  /// @param[in] chunk a read-write accessor to work with
  /// @param[in] ne Normal equations
  virtual void calcImagingEquations(const IConstDataAccessor &chunk,
                          askap::scimath::ImagingNormalEquations& ne) const = 0;

  /// @brief Predict model visibility for the iterator.
  /// @details This version of the predict method iterates
  /// over all chunks of data and calls an abstract method declared
  /// in IMeasurementEquation for each accessor. 
  virtual void predict() const;  
 
  /// @brief Calculate the normal equations for the iterator
  /// @details This version iterates through all chunks of data and
  /// calls an abstract method declared in IMeasurementEquation for each 
  /// individual accessor (each iteration of the iterator)
  /// @param[in] ne Normal equations
  virtual void calcEquations(askap::scimath::INormalEquations& ne) const;
  
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef IMAGING_MULTI_CHUNK_EQUATION_H
