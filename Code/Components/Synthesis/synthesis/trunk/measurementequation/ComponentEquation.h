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

#include <fitting/Equation.h>
#include <fitting/Params.h>

#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>

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
        
        /// @brief Templated function to do the calculation of value and derivatives.
        /// @param ra Right Ascension (rad)
        /// @param dec Declination (rad)
        /// @param bmaj Major axis (rad)
        /// @param bmin Minor axis (rad)
        /// @param bpa Position angle (rad)
        /// @param freq Observing frequency (Hz)
        /// @param u U coordinate (meters)
        /// @param v V coordinate (meters)
        /// @param w W coordinate (meters)
        /// @param vis Visibility
        template<class T>
          void calcRegularGauss(const T& ra, const T& dec, const T& flux,
          const T& bmaj, const T& bmin, const T& bpa,
          const casa::Vector<double>& freq,
          const double u, const double v, const double w,
          casa::Vector<T>& vis);
/// @brief Templated function to do the calculation of value and derivatives.
        /// @param ra Right Ascension (rad)
        /// @param dec Declination (rad)
        /// @param freq Observing frequency (Hz)
        /// @param u U coordinate (meters)
        /// @param v V coordinate (meters)
        /// @param w W coordinate (meters)
        /// @param vis Visibility
        template<class T>
          void calcRegularPoint(const T& ra, const T& dec, const T& flux,
          const casa::Vector<double>& freq,
          const double u, const double v, const double w,
          casa::Vector<T>& vis);
    };

  }

}
#endif
