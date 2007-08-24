/// @file
///
/// ComponentEquation: Equation for dealing with discrete components such
/// as point sources and Gaussians.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef SYNCOMPONENTEQUATION_H_
#define SYNCOMPONENTEQUATION_H_

// own include
#include <fitting/Equation.h>
#include <fitting/Params.h>

#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/CachedAccessorField.tcc>

#include <measurementequation/IParameterizedComponent.h>

// casa includes
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace conrad
{
  namespace synthesis
  {

    /// @brief Visibility processing for components
    ///
    /// @details This class does predictions and calculates normal equations
    /// for discrete components such as point sources and Gaussians.
    /// Names are flux.{i,q,u,v}, direction.{ra,dec}, shape.{bmaj,bmin,bpa}
    /// etc.
    ///
    /// @ingroup measurementequation
    
    class ComponentEquation : public conrad::scimath::Equation
    {
      public:

        /// @brief Standard constructor using the parameters and the
        /// data iterator.
        /// @param ip Parameters
        /// @param idi data iterator
        ComponentEquation(const conrad::scimath::Params& ip,
          IDataSharedIter& idi);

        /// @brief Constructor using default parameters
        /// @param idi data iterator
        ComponentEquation(IDataSharedIter& idi);

        /// Copy constructor
        ComponentEquation(const ComponentEquation& other);

        /// Assignment operator
        ComponentEquation& operator=(const ComponentEquation& other);

        virtual ~ComponentEquation();
        
        /// Return the default parameters
        static conrad::scimath::Params defaultParameters();

        /// @brief Predict model visibility for the iterator. 
        virtual void predict();

/// @brief Calculate the normal equations
/// @param ne Normal equations
        virtual void calcEquations(conrad::scimath::NormalEquations& ne);

      private:
      /// Shared iterator for data access
        IDataSharedIter itsIdi;
        /// Initialize this object
        virtual void init();
    protected:
        /// a short cut to shared pointer on a parameterized component
        typedef boost::shared_ptr<IParameterizedComponent> IParameterizedComponentPtr;
    
        /// @brief fill the cache of the components
        /// @details This method convertes the parameters into a vector of 
        /// components. It is called on the first access to itsComponents
        void fillComponentCache(std::vector<IParameterizedComponentPtr> &in) const;      
    private:   
        /// @brief vector of components plugged into this component equation
        /// this has nothing to do with data accessor, we just reuse the class
        /// for a cached field
        CachedAccessorField<std::vector<IParameterizedComponentPtr> > itsComponents;     
    };

  }

}

#endif
