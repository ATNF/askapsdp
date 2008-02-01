/// @file
/// 
/// @brief A structural type joining together GenericEquation and MultiChunkEquation
/// @details Because we deal here with double inheritance, we need to overload
/// explicitly predict and calcGenericEquation methods, otherwise the methods
/// in the scimath::Equation tree are left pure abstract. Theoretically, we can
/// get rid of the double inheritance here and the need of this method would 
/// disappear. However, MultiChunkEquation is envisaged to be a temporary
/// method before we start to work with accessors only. Therefore, the inheritance
/// is left double to highlight the future interface 
/// (derived from IMeasurementEquation).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <measurementequation/GenericMultiChunkEquation.h>
#include <conrad/ConradError.h>

namespace conrad {

namespace synthesis {

/// @brief Standard constructor, which remembers data iterator.
/// @param[in] idi data iterator
GenericMultiChunkEquation::GenericMultiChunkEquation(const IDataSharedIter& idi) :
         MultiChunkEquation(idi) {}

  
/// @brief Calculate the normal equations for the iterator
/// @details This version iterates through all chunks of data and
/// calls an abstract method declared in IMeasurementEquation for each 
/// individual accessor (each iteration of the iterator)
/// @param[in] ne Normal equations
void GenericMultiChunkEquation::calcGenericEquations(conrad::scimath::GenericNormalEquations& ne) const
{
  MultiChunkEquation::calcEquations(ne);
}

/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This calculation is done for a single chunk of
/// data only (one iteration).It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). This method overrides an abstract method
/// of MultiChunkEquation. It calls calcGenericEquation  with ne converted
/// to GenericNormalEquations
/// @param[in] chunk a read-write accessor to work with
/// @param[in] ne Normal equations
void GenericMultiChunkEquation::calcEquations(const IConstDataAccessor &chunk,
                   conrad::scimath::INormalEquations& ne) const
{
  try {
     calcGenericEquations(chunk, 
             dynamic_cast<conrad::scimath::GenericNormalEquations&>(ne));
  }
  catch (const std::bad_cast &bc) {
     CONRADTHROW(ConradError, "An attempt to use incompatible type of "
                 "the normal equations class with a derivative from "
                 "GenericMultiChunkEquation. It accepts only GenericNormalEquations "
                 "and derivatives. This exception probably indicates a logic error");    
  }
}

/// @brief Predict model visibility for the iterator.
/// @details This version of the predict method iterates
/// over all chunks of data and calls an abstract method declared
/// in IMeasurementEquation for each accessor. 
void GenericMultiChunkEquation::predict() const      
{
  MultiChunkEquation::predict();
}


} // namespace synthesis

} // namespace conrad
