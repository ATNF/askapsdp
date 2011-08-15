/// @file
/// @brief An interface for accessing calibration solutions for reading.
/// @details This interface is used to access calibration parameters
/// read-only. A writable version of the interface is derived from this
/// class. Various implementations are possible, i.e. parset-based, 
/// table-based and working via database ice service.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>
/// Based on the original version of this interface by Ben Humphreys <ben.humphreys@csiro.au>

#ifndef I_CAL_SOLUTION_CONST_ACCESSOR_H
#define I_CAL_SOLUTION_CONST_ACCESSOR_H

// casa includes
#include <casa/aipstype.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/SquareMatrix.h>

// boost includes
#include "boost/shared_ptr.hpp"

// own includes
#include <calibaccess/JonesIndex.h>
#include <calibaccess/JonesJTerm.h>
#include <calibaccess/JonesDTerm.h>

namespace askap {
namespace accessors {

/// @brief An interface for accessing calibration solutions for reading.
/// @details This interface is used to access calibration parameters
/// read-only. A writable version of the interface is derived from this
/// class. Various implementations are possible, i.e. parset-based, 
/// table-based and working via database ice service.
/// @ingroup calibaccess
struct ICalSolutionConstAccessor {
   /// @brief virtual destructor to keep the compiler happy
   virtual ~ICalSolutionConstAccessor();
   
   // virtual methods which need to be overridden in concrete 
   // implementation classes
   
   /// @brief obtain gains (J-Jones)
   /// @details This method retrieves parallel-hand gains for both 
   /// polarisations (corresponding to XX and YY). If no gains are defined
   /// for a particular index, gains of 1. with invalid flags set are
   /// returned.
   /// @param[in] index ant/beam index 
   /// @return JonesJTerm object with gains and validity flags
   virtual JonesJTerm gain(const JonesIndex &index) const = 0;
   
   /// @brief obtain leakage (D-Jones)
   /// @details This method retrieves cross-hand elements of the 
   /// Jones matrix (polarisation leakages). There are two values
   /// (corresponding to XY and YX) returned (as members of JonesDTerm 
   /// class). If no leakages are defined for a particular index,
   /// zero leakages are returned with invalid flags set. 
   /// @param[in] index ant/beam index
   /// @return JonesDTerm object with leakages and validity flags
   virtual JonesDTerm leakage(const JonesIndex &index) const = 0;
   
   /// @brief obtain bandpass (frequency dependent J-Jones)
   /// @details This method retrieves parallel-hand spectral
   /// channel-dependent gain (also known as bandpass) for a
   /// given channel and antenna/beam. The actual implementation
   /// does not necessarily store these channel-dependent gains
   /// in an array. It could also implement interpolation or 
   /// sample a polynomial fit at the given channel (and 
   /// parameters of the polynomial could be in the database). If
   /// no bandpass is defined (at all or for this particular channel),
   /// gains of 1.0 are returned (with invalid flag is set).
   /// @param[in] index ant/beam index
   /// @param[in] chan spectral channel of interest
   /// @return JonesJTerm object with gains and validity flags
   virtual JonesJTerm bandpass(const JonesIndex &index, const casa::uInt chan) const = 0;
   
   // helper methods to simplify access to the calibration parameters
   
   /// @brief obtain full 2x2 Jones Matrix taking all effects into account
   /// @details This method returns resulting 2x2 matrix taking gain, leakage and
   /// bandpass effects (for a given channel) into account. Invalid gains (and bandpass
   /// values) are replaced by 1., invalid leakages are replaced by zeros. This method
   /// calls gain, bandpass and leakage virtual methods
   /// @param[in] index ant/beam index
   /// @param[in] chan spectral channel of interest
   /// @return 2x2 Jones matrix
   casa::SquareMatrix<casa::Complex, 2> jones(const JonesIndex &index, const casa::uInt chan) const;
      
   /// @brief obtain full 2x2 Jones Matrix taking all effects into account
   /// @details This version of the method accepts antenna and beam indices explicitly and
   /// does extra checks before calling the main method expressed via JonesIndex.
   /// @param[in] ant antenna index
   /// @param[in] beam beam index
   /// @param[in] chan spectral channel of interest
   /// @return 2x2 Jones matrix
   casa::SquareMatrix<casa::Complex, 2> jones(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) const;
   
   /// @brief obtain validity flag for the full 2x2 Jones Matrix
   /// @details This method combines all validity flags for parameters used to compose Jones
   /// matrix and returns true if all elements are valid and false if at least one constituent
   /// is not valid
   /// @param[in] index ant/beam index
   /// @param[in] chan spectral channel of interest
   /// @return true, if the matrix returned by jones(...) method called with the same parameters is
   /// valid, false otherwise
   bool jonesValid(const JonesIndex &index, const casa::uInt chan) const;
   
   /// @brief obtain validity flag for the full 2x2 Jones Matrix
   /// @details This version of the method accepts antenna and beam indices explicitly and
   /// does extra checks before calling the main method expressed via JonesIndex.
   /// @param[in] ant antenna index
   /// @param[in] beam beam index
   /// @param[in] chan spectral channel of interest
   /// @return true, if the matrix returned by jones(...) method called with the same parameters is
   /// valid, false otherwise
   bool jonesValid(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) const;

   /// @brief shared pointer definition
   typedef boost::shared_ptr<ICalSolutionConstAccessor> ShPtr;
};

} // namespace accessors
} // namespace askap

#endif // #ifndef I_CAL_SOLUTION_CONST_ACCESSOR_H
 