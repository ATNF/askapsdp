/// @file
/// 
/// @brief Composite calibration component  (a sum of two or three others).
/// @details This template acts as a composite effect with the resulting
/// Mueller matrix equal to the sum of input Mueller matrices.
/// @note I currently forsee two ways of dealing with the composite effects,
/// especially sums. First, if the effect is solvable it has to be included
/// in the effect chain and used with CalibrationME. This template is
/// intended for this case. The second way is to have a separate composite
/// equation replacing CalibrationME, which adds some effect to the data.
/// It is more appropriate for simulator, which can add some non-solvable 
/// modifications of the data (e.g. noise). The main benefit of this 
/// second approach is an ability to construct the equations more dynamically.
/// The main drawback is inability to solve for parameters using just the 
/// functionality of wrapped classes.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef SUM_H
#define SUM_H

#include <measurementequation/MEComponent.h>
#include <dataaccess/IConstDataAccessor.h>

namespace askap {

namespace synthesis {

/// @brief Composite calibration component  (a sum of two or three others).
/// @details This template acts as a composite effect with the resulting
/// Mueller matrix equal to the sum of input Mueller matrices.
/// @note I currently forsee two ways of dealing with the composite effects,
/// especially sums. First, if the effect is solvable it has to be included
/// in the effect chain and used with CalibrationME. This template is
/// intended for this case. The second way is to have a separate composite
/// equation replacing CalibrationME, which adds some effect to the data.
/// It is more appropriate for simulator, which can add some non-solvable 
/// modifications of the data (e.g. noise). The main benefit of this 
/// second approach is an ability to construct the equations more dynamically.
/// The main drawback is inability to solve for parameters using just the 
/// functionality of wrapped classes.
/// @ingroup measurementequation
template<typename Effect1,typename  Effect2,typename  Effect3 = MEComponent>
struct Sum : public MEComponent {

   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit Sum(const scimath::Params &par) :  
            itsEffect1(par), itsEffect2(par), itsEffect3(par) {}

   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary.
   /// @param[in] chunk accessor to work with
   /// @param[in] row row of the chunk to work with
   /// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
   /// this effect
   inline scimath::ComplexDiffMatrix get(const IConstDataAccessor &chunk, 
                                casa::uInt row) const
   { using namespace scimath; return itsEffect1.get(chunk,row)+
          itsEffect2.get(chunk,row)+itsEffect3.get(chunk,row); }

   
private:
   /// @brief buffer for the first effect
   Effect1 itsEffect1;
   /// @brief buffer for the second effect
   Effect2 itsEffect2;
   /// @brief buffer for the third effect
   Effect3 itsEffect3;
};


/// @brief specialization for two items only
template<typename Effect1, typename Effect2>
struct Sum<Effect1, Effect2, MEComponent> : public MEComponent {

   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit Sum(const scimath::Params &par) :  
            itsEffect1(par), itsEffect2(par) {}

   /// @brief main method returning Mueller matrix and derivatives
   /// @details This method has to be overloaded (in the template sense) for
   /// all classes representing various calibration effects. CalibrationME
   /// template will call it when necessary.
   /// @param[in] chunk accessor to work with
   /// @param[in] row row of the chunk to work with
   /// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
   /// this effect
   inline scimath::ComplexDiffMatrix get(const IConstDataAccessor &chunk, 
                                casa::uInt row) const
   { using namespace scimath; return itsEffect1.get(chunk,row) + itsEffect2.get(chunk,row); }

   
private:
   /// @buffer first effect
   Effect1 itsEffect1;
   /// @buffer second effect
   Effect2 itsEffect2;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef SUM_H
