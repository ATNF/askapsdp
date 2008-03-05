/// @file
/// 
/// @brief An adapter to make imaging equation a derivative from IMeasurementEquation
/// @details The current imaging code works with iterators, rather than accessors.
/// Although ImagingMultiChunkEquation allows to take this iterator dependency
/// in stages, it is still a lot of work to convert calcEquations and predict
/// methods of a typical imaging measurement equation to be able to derive it 
/// from this class. This adapter allows to translate calls to the virtual
/// methods of IMeasurementEquation to the appropriate call of the 
/// iterator-dependent measurement equation. I hope this adapter is temporary.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <measurementequation/ImagingEquationAdapter.h>

using namespace askap;
using namespace askap::synthesis;

#include <dataaccess/FakeSingleStepIterator.h>
#include <askap/AskapError.h>

/// @brief constructor
/// @details This constructor initializes a fake iterator. Actual measurement
/// equation is set up via a call to assign method (templated)
ImagingEquationAdapter::ImagingEquationAdapter() : 
        itsIterAdapter(new FakeSingleStepIterator) {}

/// @brief access to parameters
/// @details This call is translated to itsActualEquation. We need to override
/// this method of scimath::Equation, as otherwise an empty parameter class
/// initialized in the default constructor would always be returned.
/// @return const reference to paramters of this equation
const scimath::Params& ImagingEquationAdapter::parameters() const
{
  ASKAPCHECK(itsActualEquation, 
     "assign method should be called before first usage of ImagingEquationAdapter");
  return itsActualEquation->parameters();
}
   
/// @brief set parameters
/// @details This call is translated to itsActualEquation.
/// @params[in] par new parameters
void ImagingEquationAdapter::setParameters(const scimath::Params &par)
{
  ASKAPCHECK(itsActualEquation, 
     "assign method should be called before first usage of ImagingEquationAdapter");
  itsActualEquation->setParameters(par);  
}
   
/// @brief predict visibilities
/// @details This call is translated to itsActualEquation.
void ImagingEquationAdapter::predict() const
{
  ASKAPCHECK(itsActualEquation, 
     "assign method should be called before first usage of ImagingEquationAdapter");
  // there will be an exception if this class is initialized with the type,
  // which works with the iterator directly and bypasses accessor-based method.
  itsActualEquation->predict();
}
   
/// @brief calculate normal equations
/// @details This call is translated to itsActualEquation.
/// @params[in] ne normal equations to be updated
void ImagingEquationAdapter::calcEquations(scimath::INormalEquations &ne) const
{ 
  ASKAPCHECK(itsActualEquation, 
     "assign method should be called before first usage of ImagingEquationAdapter");
  
  // there will be an exception if this class is initialized with the type,
  // which works with the iterator directly and bypasses accessor-based method.
  itsActualEquation->calcEquations(ne);
}
   
/// @brief clone this "composite" equation
/// @details The operations performed by this method are more complex 
/// than just copy constructor, because we store shared pointers to 
/// the iterator adapter and underlying measurement equation. They both
/// have to be cloned properly.
/// @return a shared pointer with the cloned version
scimath::Equation::ShPtr ImagingEquationAdapter::clone() const
{
  boost::shared_ptr<ImagingEquationAdapter> result(new ImagingEquationAdapter(*this));
  try {
     if (itsIterAdapter) {
         // we need an explicit type conversion, because shared iter would
         // dereference the iterator according to its interface
         const boost::shared_ptr<IDataIterator> basicIt = itsIterAdapter;
         ASKAPDEBUGASSERT(basicIt);
     
         const FakeSingleStepIterator &it = dynamic_cast<const FakeSingleStepIterator&>(*basicIt);
         result->itsIterAdapter = IDataSharedIter(new FakeSingleStepIterator(it));
     }
     if (itsActualEquation) {
         result->itsActualEquation = itsActualEquation->clone();
     }
  }
  catch (const std::bad_cast &bc) {
     ASKAPTHROW(AskapError, "Bad cast inside ImagingEquationAdapter::clone, most likely this means "
                 "there is a logical error");
  }
  return result;
}
   
/// @brief accessor-based version of predict
/// @details This version of predict is implemented via iterator-based
/// version of itsIterAdapter.
/// @params[in] chunk a chunk to be filled with predicted data
void ImagingEquationAdapter::predict(IDataAccessor &chunk) const
{     
   boost::shared_ptr<FakeSingleStepIterator> it = 
             itsIterAdapter.dynamicCast<FakeSingleStepIterator>();
   if (!it) {
       ASKAPTHROW(AskapError, "Bad cast inside ImagingEquationAdapter::predict, most likely this means "
              "there is a logical error"); 
   }
   it->assignDataAccessor(chunk);  
   predict();
   it->detachAccessor();      
}
   
/// @brief accessor-based version of calcEquations
/// @details This version of calcEquations is implemented via iterator-based
/// version of itsIterAdapter.
/// @params[in] chunk a chunk of data to work with
/// @params[in] ne normal equations to update
void ImagingEquationAdapter::calcEquations(const IConstDataAccessor &chunk,
             scimath::INormalEquations &ne) const
{
  boost::shared_ptr<FakeSingleStepIterator> it = 
            itsIterAdapter.dynamicCast<FakeSingleStepIterator>();
  if (!it) {
      ASKAPTHROW(AskapError, "Bad cast inside ImagingEquationAdapter::calcEquations, most likely this means "
                 "there is a logical error");
  }
  it->assignConstDataAccessor(chunk);  
  calcEquations(ne);
  it->detachAccessor();      
}       
 