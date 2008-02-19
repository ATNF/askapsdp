/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef NO_X_POL_GAIN_H
#define NO_X_POL_GAIN_H

// own includes
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/Params.h>
#include <dataaccess/IConstDataAccessor.h>
#include <conrad/ConradError.h>
#include <measurementequation/ParameterizedMEComponent.h>


// std includes
#include <string>

namespace conrad {

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
   CONRADDEBUGASSERT(chunk.nPol());   
   const casa::uInt nPol = chunk.nPol()<=2 ? chunk.nPol() : 2;
   
   /*
   if (chunk.nPol()>2) {
       CONRADTHROW(ConradError, "Cross pols are not supported at the moment, you have nPol="<<
                   chunk.nPol());
   }
   */
   
   const casa::uInt ant1 = chunk.antenna1()[row];
   const casa::uInt ant2 = chunk.antenna2()[row];
   
   CONRADASSERT(ant1!=ant2); // not yet implemented
   
   scimath::ComplexDiffMatrix calFactor(chunk.nPol(), chunk.nPol(), 0.);

   for (casa::uInt pol=0; pol<nPol; ++pol) {
             
        // gains for antenna 1, polarisation pol
        const std::string g1name = paramName(ant1,pol);
        const casa::Complex g1 = parameters().complexValue(g1name);
            
        // gains for antenna 2, polarisation pol
        const std::string g2name = paramName(ant2,pol);
        const casa::Complex g2 = parameters().complexValue(g2name);
            
        calFactor(pol,pol) = scimath::ComplexDiff(g1name,g1)*
                 conj(scimath::ComplexDiff(g2name,g2));            
   }
   return calFactor;
}

} // namespace synthesis

} // namespace conrad



#endif // #ifndef NO_X_POL_GAIN_H
