/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef NO_X_POL_GAIN_H
#define NO_X_POL_GAIN_H

// own includes
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/Params.h>
#include <dataaccess/IConstDataAccessor.h>
#include <askap/AskapError.h>
#include <measurementequation/ParameterizedMEComponent.h>


// std includes
#include <string>
#include <utility>

namespace askap {

namespace synthesis {

/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
/// @ingroup measurementequation
struct NoXPolGain : public ParameterizedMEComponent {
   
   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit NoXPolGain(const scimath::Params &par) : 
                              ParameterizedMEComponent(par) {}
   
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
protected:
   /// @brief obtain a name of the parameter
   /// @details This method returns the parameter name for a gain of the
   /// given antenna and polarisation. In the future, we may add time and/or
   /// feed number as well.
   /// @param[in] ant antenna number (0-based)
   /// @param[in] pol index of the polarisation product
   static std::string paramName(casa::uInt ant, casa::uInt pol);
   
   /// @brief obtain polarisation indices
   /// @details We really need a better way of handling orders of polarisation
   /// products. I hope this method is temporary. It translates polarisation
   /// plane number in the visibility cube to two polarisation indices (i.e. 0 or 1).
   /// @param[in] pol polarisation plane number in the visibility cube 
   /// @param[in] nPol total number of polarisation planes
   /// @return a pair with polarisation indices
   static std::pair<casa::uInt, casa::uInt> polIndices(casa::uInt pol, casa::uInt nPol);
};

/// @brief main method returning Mueller matrix and derivatives
/// @details This method has to be overloaded (in the template sense) for
/// all classes representing various calibration effects. CalibrationME
/// template will call it when necessary. It returns 
/// @param[in] chunk accessor to work with
/// @param[in] row row of the chunk to work with
/// @return ComplexDiffMatrix filled with Mueller matrix corresponding to
/// this effect
inline scimath::ComplexDiffMatrix NoXPolGain::get(const IConstDataAccessor &chunk, 
                                      casa::uInt row) const
{
   const casa::uInt nPol = chunk.nPol();
   ASKAPDEBUGASSERT(nPol);   
   
   const casa::uInt ant1 = chunk.antenna1()[row];
   const casa::uInt ant2 = chunk.antenna2()[row];
   
   ASKAPASSERT(ant1!=ant2); // not yet implemented
   
   scimath::ComplexDiffMatrix calFactor(nPol, nPol, 0.);

   for (casa::uInt pol=0; pol<nPol; ++pol) {
             
        const std::pair<casa::uInt, casa::uInt> pInd = polIndices(pol, nPol);
        // gains for antenna 1, polarisation pInd.first
        const std::string g1name = paramName(ant1,pInd.first);
        const casa::Complex g1 = parameters().complexValue(g1name);
            
        // gains for antenna 2, polarisation polInd.second
        const std::string g2name = paramName(ant2,pInd.second);
        const casa::Complex g2 = parameters().complexValue(g2name);
            
        calFactor(pol,pol) = scimath::ComplexDiff(g1name,g1)*
                 conj(scimath::ComplexDiff(g2name,g2));            
   }
   return calFactor;
}

} // namespace synthesis

} // namespace askap



#endif // #ifndef NO_X_POL_GAIN_H
