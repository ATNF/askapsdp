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

#ifndef IMAGING_EQUATION_ADAPTER_H
#define IMAGING_EQUATION_ADAPTER_H

#include <fitting/Equation.h>
#include <fitting/Params.h>
#include <fitting/INormalEquations.h>
#include <measurementequation/IMeasurementEquation.h>
#include <gridding/IVisGridder.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataAccessor.h>

namespace askap {

namespace synthesis {

/// @brief An adapter to make imaging equation a derivative from IMeasurementEquation
/// @details The current imaging code works with iterators, rather than accessors.
/// Although ImagingMultiChunkEquation allows to take this iterator dependency
/// in stages, it is still a lot of work to convert calcEquations and predict
/// methods of a typical imaging measurement equation to be able to derive it 
/// from this class. This adapter allows to translate calls to the virtual
/// methods of IMeasurementEquation to the appropriate call of the 
/// iterator-dependent measurement equation. I hope this adapter is temporary.
/// @ingroup measurementequation
struct ImagingEquationAdapter : virtual public IMeasurementEquation,
                      virtual public askap::scimath::Equation
{
   /// @brief constructor
   /// @details This constructor initializes a fake iterator. Actual measurement
   /// equation is set up via a call to assign method (templated)
   ImagingEquationAdapter();
   
   /// @brief assign actual measurement equation to an adapter
   /// @details This templated method constructs the actual measurement equation
   /// of an appropriate type and sets up itsActualEquation. itsIterAdapter is
   /// passed as an iterator. This version accepts just the parameters.
   /// @params[in] par input parameters
   template<typename ME>
   void assign(const scimath::Params &par) 
   { itsActualEquation.reset(new ME(par, itsIterAdapter)); }
   
   /// @brief assign actual measurement equation to an adapter
   /// @details This templated method constructs the actual measurement equation
   /// of an appropriate type and sets up itsActualEquation. itsIterAdapter is
   /// passed as an iterator. This version accepts parameters and a gridder.
   /// @params[in] par input parameters
   /// @params[in] gridder input gridder (passed as shared pointer)
   template<typename ME>
   void assign(const scimath::Params &par, const IVisGridder::ShPtr &gridder) 
   { itsActualEquation.reset(new ME(par, itsIterAdapter, gridder)); }
   
   /// @brief access to parameters
   /// @details This call is translated to itsActualEquation. We need to override
   /// this method of scimath::Equation, as otherwise an empty parameter class
   /// initialized in the default constructor would always be returned.
   /// @return const reference to paramters of this equation
   virtual const scimath::Params& parameters() const;
   
   /// @brief set parameters
   /// @details This call is translated to itsActualEquation.
   /// @params[in] par new parameters
   virtual void setParameters(const scimath::Params &par);
   
   /// @brief predict visibilities
   /// @details This call is translated to itsActualEquation.
   virtual void predict() const;
   
   /// @brief calculate normal equations
   /// @details This call is translated to itsActualEquation.
   /// @params[in] ne normal equations to be updated
   virtual void calcEquations(scimath::INormalEquations &ne) const;
   
   /// @brief clone this "composite" equation
   /// @details The operations performed by this method are more complex 
   /// than just copy constructor, because we store shared pointers to 
   /// the iterator adapter and underlying measurement equation. They both
   /// have to be cloned properly.
   /// @return a shared pointer with the cloned version
   scimath::Equation::ShPtr clone() const;        
   
   /// @brief accessor-based version of predict
   /// @details This version of predict is implemented via iterator-based
   /// version of itsIterAdapter.
   /// @params[in] chunk a chunk to be filled with predicted data
   virtual void predict(IDataAccessor &chunk) const;
   
   /// @brief accessor-based version of calcEquations
   /// @details This version of calcEquations is implemented via iterator-based
   /// version of itsIterAdapter.
   /// @params[in] chunk a chunk of data to work with
   /// @params[in] ne normal equations to update
   virtual void calcEquations(const IConstDataAccessor &chunk,
             scimath::INormalEquations &ne) const;
   
private:
   /// @brief iterator adapter
   IDataSharedIter itsIterAdapter;
   /// @brief actual measurement equation 
   scimath::Equation::ShPtr itsActualEquation;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef IMAGING_EQUATION_ADAPTER_H
