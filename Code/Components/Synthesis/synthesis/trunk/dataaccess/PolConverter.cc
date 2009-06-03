/// @file
/// @brief Converter of polarisation frames
/// @details This is the class which handles polarisation frame conversion and contains some
/// helper methods related to it (i.e. converting strings into Stokes enums). It may eventually
/// replace or become derived from IPolSelector, which is not used at the moment.
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
///

#include <dataaccess/PolConverter.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief constructor of the converter between two frames
/// @details
/// @param[in] polFrameIn input polarisation frame defined as a vector of Stokes enums
/// @param[in] polFrameOut output polarisation frame defined as a vector of Stokes enums
PolConverter::PolConverter(const casa::Vector<casa::Stokes::StokesTypes> &polFrameIn,
               const casa::Vector<casa::Stokes::StokesTypes> &polFrameOut) : itsVoid(false),
               itsTransform(polFrameOut.nelements(),polFrameIn.nelements(),casa::Complex(0.,0.)),
               itsPolFrameIn(polFrameIn), itsPolFrameOut(polFrameOut)
{
  if (equal(polFrameIn, polFrameOut)) {
      itsVoid = true;
  } else {
    fillMatrix(polFrameIn, polFrameOut);
  }
}
  
/// @brief default constructor - no conversion
/// @details Constructed via this method the object passes all visibilities intact
PolConverter::PolConverter() : itsVoid(true)
{
}

/// @brief compare two vectors of Stokes enums
/// @param[in] first first polarisation frame
/// @param[in] second second polarisation frame
/// @return true if two given frames are the same, false if not.
bool PolConverter::equal(const casa::Vector<casa::Stokes::StokesTypes> &first,
                    const casa::Vector<casa::Stokes::StokesTypes> &second)
{
  if (first.nelements() != second.nelements()) {
      return false;
  }
  for (casa::uInt pol = 0; pol<first.nelements(); ++pol) {
       if (first[pol]!=second[pol]) {
           return false;
       }
  }
  return true;
}
  
/// @brief main method doing conversion
/// @details Convert the given visibility vector between two polarisation frames supplied
/// in the constructor.
/// @param[in] vis visibility vector
/// @return converted visibility vector 
/// @note vis should have the same size (<=4) as both polFrames passed in the constructor, 
/// the output vector will have the same size.
casa::Vector<casa::Complex> PolConverter::operator()(casa::Vector<casa::Complex> vis) const
{
  if (itsVoid) {
      return vis;
  }
  ASKAPDEBUGASSERT(vis.nelements() == itsTransform.ncolumn());
  casa::Vector<casa::Complex> res(itsTransform.nrow(),0.);
  
  for (casa::uInt row = 0; row<res.nelements(); ++row) {
       for (casa::uInt col = 0; col<vis.nelements(); ++col) {
            res[row] += itsTransform(row,col)*vis[col];
       }
  }
  
  return res;
}

/// @brief build transformation matrix
/// @details This is the core of the algorithm, this method builds the transformation matrix
/// given the two frames .
/// @param[in] polFrameIn input polarisation frame defined as a vector of Stokes enums
/// @param[in] polFrameOut output polarisation frame defined as a vector of Stokes enums
void PolConverter::fillMatrix(const casa::Vector<casa::Stokes::StokesTypes> &polFrameIn,
                  const casa::Vector<casa::Stokes::StokesTypes> &polFrameOut)
{
  ASKAPDEBUGASSERT(itsTransform.nrow() == polFrameOut.nelements());
  ASKAPDEBUGASSERT(itsTransform.ncolumn() == polFrameIn.nelements());
   
}

