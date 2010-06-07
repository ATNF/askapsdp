/// @file
///
/// WienerPreconditioner: Precondition the normal equations
///                       by applying a Wiener filter
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
/// @author Urvashi Rau <rurvashi@aoc.nrao.edu>
///
#ifndef SYN_WEINER_PRECONDITIONER_H_
#define SYN_WEINER_PRECONDITIONER_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <fitting/Axes.h>

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

#include <measurementequation/IImagePreconditioner.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Precondition the normal equations via a Wiener filter
    ///
    /// @details It constructs a Wiener filter from the PSF 
    /// and applies it to the PSF and current Residual image
    /// @ingroup measurementequation
    class WienerPreconditioner : public IImagePreconditioner
    {
      public:

     /// @brief default constructor with zero noise power
     /// @note do we really need it?
     WienerPreconditioner();
     
     /// @brief constructor with explicitly defined noise power
     /// @param[in] noisepower parameter of the 
     /// @param[in] normalise if true, PSF is normalised during filter construction
     WienerPreconditioner(float noisepower, bool normalise);

     /// @brief constructor with explicitly defined robustness
     /// @details In this version, the noise power is calculated from
     /// the robustness parameter 
     /// @param[in] robustness robustness parameter (roughly matching Briggs' weighting)
     /// @note Normalisation of PSF is always used when noise power is defined via robustness
     WienerPreconditioner(float robustness);
        
      /// @brief Clone this object
      /// @return shared pointer to a cloned copy
      virtual IImagePreconditioner::ShPtr clone();
        
      /// @brief Apply preconditioning to Image Arrays
      /// @details This is the actual method, which does preconditioning.
      /// It is applied to the PSF as well as the current residual image.
      /// @param[in] psf array with PSF
      /// @param[in] dirty array with dirty image
      /// @return true if psf and dirty have been altered
      virtual bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) const;

      /// @brief static factory method to create preconditioner from a parset
      /// @details
      /// @param[in] parset subset of parset file (with preconditioner.Wiener. removed)
      /// @return shared pointer 
      static boost::shared_ptr<WienerPreconditioner> createPreconditioner(const LOFAR::ParameterSet &parset);

      private:
      /// @brief Parameter of the filter
      /// @details Depending on the mode, it can either be the noise power value directly or the 
      /// robustness parameter roughly matching Briggs' weighting.
      float itsParameter;

      /// @brief Use normalised PSF in filter construction?
      bool itsDoNormalise;

      /// @brief true, if parameter is robustness, false if it is the noise power
      bool itsUseRobustness;
   };

  }
}
#endif
