/// @file
/// 
/// @brief Calibration effect: antenna gains without cross-pol
/// @details This is a simple effect which can be used in conjunction
/// with the CalibrationME template (as its template argument)
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
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
#include <utils/PolConverter.h>

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
   /// @param[in] par shared pointer to parameters
   inline explicit NoXPolGain(const scimath::Params::ShPtr &par) : 
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
   /// @brief obtain a value of the parameter
   /// @details This helper method checks whether a particular parameter
   /// is defined in the internal Params object. If yes, the appropriate
   /// value is returned wrapped around in a ComplexDiff class. If not,
   /// a default value of (1.,0.) is returned. This method encapsulates
   /// the assignment of default values.
   /// @param[in] name parameter name
   /// @return value of the parameter wrapped in a complex diff object
   inline scimath::ComplexDiff getParameter(const std::string &name) const;


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


/// @brief obtain a value of the parameter
/// @details This helper method checks whether a particular parameter
/// is defined in the internal Params object. If yes, the appropriate
/// value is returned wrapped around in a ComplexDiff class. If not,
/// a default value of (1.,0.) is returned. This method encapsulates
/// the assignment of default values.
/// @param[in] name parameter name
/// @return value of the parameter wrapped in a complex diff object
inline scimath::ComplexDiff NoXPolGain::getParameter(const std::string &name) const
{
   ASKAPDEBUGASSERT(parameters());
   if (parameters()->has(name)) {
       // there is a parameter defined with the given name
       const casa::Complex gain = parameters()->complexValue(name);
       return scimath::ComplexDiff(name, gain);
   }
   // return the default
   return scimath::ComplexDiff(name,casa::Complex(1.,0.));
}


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
   ASKAPDEBUGASSERT(nPol != 0);   
   const casa::Vector<casa::Stokes::StokesTypes> stokes = chunk.stokes();   
   ASKAPDEBUGASSERT(stokes.nelements() == nPol);
   ASKAPDEBUGASSERT(!scimath::PolConverter::isStokes(stokes));
   
   const casa::uInt ant1 = chunk.antenna1()[row];
   const casa::uInt ant2 = chunk.antenna2()[row];
   
   scimath::ComplexDiffMatrix calFactor(nPol, nPol, 0.);

   for (casa::uInt pol=0; pol<nPol; ++pol) {
        
        const casa::uInt polIndex = scimath::PolConverter::getIndex(stokes[pol]);
        // polIndex is index in the polarisation frame, i.e.
        // XX is 0, XY is 1, YX is 2 and YY is 3
        // we need an index into matrix 
        const std::pair<casa::uInt, casa::uInt> pInd = polIndices(pol, nPol);
        // gains for antenna 1, polarisation X if XX or XY, or Y if YX or YY
        const std::string g1name = paramName(ant1, polIndex / 2);
            
        // gains for antenna 2, polarisation X if XX or YX, or Y if XY or YY
        const std::string g2name = paramName(ant2, polIndex % 2);
            
        calFactor(pol,pol) = getParameter(g1name)*conj(getParameter(g2name));            
   }
   return calFactor;
}

} // namespace synthesis

} // namespace askap



#endif // #ifndef NO_X_POL_GAIN_H
