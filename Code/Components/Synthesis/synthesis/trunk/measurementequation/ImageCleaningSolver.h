/// @file
/// @brief A structural class for solvers doing cleaning
/// @details There are specific parameters for solvers doing cleaning via LatticeCleaner.
/// It seems that at this stage most of these specialized parameters are handled by
/// the scimath::Solveable class (is it a fat interface?), however a fractional threshold
/// required by just multiscale solver and MSMF solver is defined here.
///
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

#ifndef IMAGE_CLEANING_SOLVER_H
#define IMAGE_CLEANING_SOLVER_H

#include <measurementequation/ImageSolver.h>

namespace askap {

namespace synthesis {

/// @brief A structural class for solvers doing cleaning
/// @details There are specific parameters for solvers doing cleaning via LatticeCleaner.
/// It seems that at this stage most of these specialized parameters are handled by
/// the scimath::Solveable class (is it a fat interface?), however a fractional threshold
/// required by just multiscale solver and MSMF solver is defined here.
/// @ingroup measurementequation
class ImageCleaningSolver : public ImageSolver {
public:
   /// @brief constructor from parameters
   /// @details Free parameters named image* will be interpreted as images and
   /// solutions formed by the solveNormalEquation method
   /// @param[in] ip input parameters
   ImageCleaningSolver(const askap::scimath::Params &ip);
   
   /// @brief access to a fractional threshold
   /// @return current fractional threshold
   double fractionalThreshold() const;
   
   /// @brief set a new fractional threshold
   /// @param[in] fThreshold new fractional threshold
   /// @note Assign 0. to switch this option off.
   void setFractionalThreshold(double fThreshold);
   
   /// @brief access to a masking threshold
   /// @return current masking threshold
   double maskingThreshold() const;
   
   /// @brief set a new masking threshold
   /// @param[in] mThreshold new masking threshold
   /// @note Assign -1. or any negative number to revert to a default behavior of the
   /// S/N based cleaning. The masking threshold value, which used to be hardcoded in
   /// the casacore when signal-based cleaning was the only available option, equals to 0.9.
   void setMaskingThreshold(double mThreshold);
   
private:
   /// @brief Fractional cleaning threshold   
   /// @details it means it is defined with respect to the peak flux (i.e. 0.2 from the peak flux).
   /// Assign zero (default) if you don't want any fractional threshold applied.
   double itsFractionalThreshold;
   
   /// @brief threshold for cleaning mask
   /// @details This value is passed in the setMask call to the LatticeCleaner. Negative 
   /// value (default) means that no thresholding is used to the mask and the mask array
   /// is treated as a weight providing a capability to do S/N-based cleaning. This is the
   /// default approach. If a positive value is assigned, it is treated as a threshold.
   /// If the mask value is lower than this threshold, the corresponding pixel is not cleaned.
   /// This is a classical signal-based cleaning.
   double itsMaskingThreshold;
};

} // namespace synthesis

} // namespace askap


#endif // #ifndef IMAGE_CLEANING_SOLVER_H
