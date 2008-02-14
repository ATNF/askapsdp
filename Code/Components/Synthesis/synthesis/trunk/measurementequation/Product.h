/// @file
/// 
/// @brief Composite calibration component  (a product of two others).
/// @details This template acts as a composite effect with the resulting
/// Mueller matrix equal to the matrix product of two input Mueller matrices.
/// @note There are plans to extend the interface to several multipliers
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef PRODUCT_H
#define PRODUCT_H

#include <measurementequation/MEComponent.h>
#include <dataaccess/IConstDataAccessor.h>

namespace conrad {

namespace synthesis {

/// @brief Composite calibration component  (a product of two others).
/// @details This template acts as a composite effect with the resulting
/// Mueller matrix equal to the matrix product of two input Mueller matrices.
/// @note There are plans to extend the interface to several multipliers
/// @ingroup measurementequation
template<typename Effect1,typename  Effect2,typename  Effect3 = MEComponent>
struct Product  : public MEComponent {

   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit Product(const scimath::Params &par) :  
            itsEffect1(par), itsEffect2(par), itsEffect3(par) {}

   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary. It returns 
   /// @param[in] chunk accessor to work with
   /// @param[in] row row of the chunk to work with
   /// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
   /// this effect
   inline scimath::ComplexDiffMatrix get(const IConstDataAccessor &chunk, 
                                casa::uInt row) const
   { using namespace scimath; return itsEffect1.get(chunk,row)*
          itsEffect2.get(chunk,row)*itsEffect3.get(chunk,row); }

   
private:
   /// @brief buffer for the first effect
   Effect1 itsEffect1;
   /// @brief buffer for the second effect
   Effect2 itsEffect2;
   /// @brief buffer for the third effect
   Effect3 itsEffect3;
};


/// @brief specialization for two multipliers only
template<typename Effect1, typename Effect2>
struct Product<Effect1, Effect2, MEComponent> : public MEComponent {

   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit Product(const scimath::Params &par) :  
            itsEffect1(par), itsEffect2(par){}

   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary. It returns 
   /// @param[in] chunk accessor to work with
   /// @param[in] row row of the chunk to work with
   /// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
   /// this effect
   inline scimath::ComplexDiffMatrix get(const IConstDataAccessor &chunk, 
                                casa::uInt row) const
   { using namespace scimath; return itsEffect1.get(chunk,row)*itsEffect2.get(chunk,row); }

   
private:
   /// @buffer first effect
   Effect1 itsEffect1;
   /// @buffer second effect
   Effect2 itsEffect2;
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef PRODUCT_H
