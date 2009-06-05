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
    for (casa::uInt pol=0; pol<polFrameIn.nelements(); ++pol) {
         ASKAPCHECK(isValid(polFrameIn[pol]), "Conversion is unsupported for polarisation product "<<
                    int(polFrameIn[pol])<<" ("<<casa::Stokes::type(polFrameIn[pol])<<")");
    }
    for (casa::uInt pol=0; pol<polFrameOut.nelements(); ++pol) {
         ASKAPCHECK(isValid(polFrameOut[pol]), "Conversion is unsupported for polarisation product "<<
                    int(polFrameOut[pol])<<" ("<<casa::Stokes::type(polFrameOut[pol])<<")");
    }
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
  // See Hamaker, Bregman and Sault, 1996, A&ASS, 117, 137 for matrix formalism of
  // the polarisation conversion 

  // todo, check whether we can do the same in a more elegant and general way.
  // Do we need reverse conversion as well?
  
  casa::Matrix<casa::Complex> T(4,4,0.);
  if (isStokes(polFrameOut)) {
      if (isLinear(polFrameIn)) {
          // linear to stokes   
          T(0,0)=1.; T(0,3)=1.; 
          T(1,0)=1.; T(1,3)=-1.;
          T(2,1)=1.; T(2,2)=1.;
          T(3,1)=casa::Complex(0.,-1.); T(3,2)=casa::Complex(0.,1.);
      } else if (isCircular(polFrameIn)) {
          // circular to stokes
          T(0,0)=1.; T(0,3)=1.; 
          T(1,1)=casa::Complex(0.,-1.); T(3,2)=casa::Complex(0.,1.);
          T(2,0)=1.; T(1,3)=-1.;
          T(3,1)=1.; T(2,2)=1.;
      } else if (isStokes(polFrameIn)) {
          T.diagonal() = 1.;
      } else {
          ASKAPTHROW(AskapError, "Conversion of input polarisation frames into stokes parameters is not supported");
      }
  } else if ((isLinear(polFrameIn) && isLinear(polFrameOut)) || 
             (isCircular(polFrameIn) && isCircular(polFrameOut)))  {
             T.diagonal() = 1.;
  } else {
      ASKAPTHROW(AskapError, "Unsupported combination of input and output polarisation frames");
  }
    
  // have to copy, because the transformation may not preserve dimensionality
  for (casa::uInt row = 0; row<itsTransform.nrow(); ++row) {
       const casa::uInt rowIndex = getIndex(polFrameOut[row]);
       ASKAPDEBUGASSERT(rowIndex<4);
       for (casa::uInt col = 0; col<itsTransform.ncolumn(); ++col) {
            const casa::uInt colIndex = getIndex(polFrameIn[col]);
            ASKAPDEBUGASSERT(colIndex<4);
            itsTransform(row,col) = T(rowIndex,colIndex);
       }
  }
}
  
/// @brief test if frame matches a given stokes enum
/// @param[in] polFrame polarisation frame defined as a vector of Stokes enums
/// @param[in] stokes a single stokes enum defining the frame (should be the first in the set)
/// @return true, if the given vector and one stokes enum belong to the same frame 
bool PolConverter::sameFrame(const casa::Vector<casa::Stokes::StokesTypes> &polFrame,
                        casa::Stokes::StokesTypes stokes)
{
  ASKAPASSERT(polFrame.nelements()!=0);
  for (casa::uInt pol = 0; pol<polFrame.nelements(); ++pol) {
       const int index = (int(polFrame[pol]) - int(stokes));
       if ((index<0) || (index>=4)) {
           return false;
       }
  }
  return true;
}
  
/// @brief return index of a particular polarisation
/// @details To be able to fill matrices efficiently we want to convert, say IQUV into 0,1,2,3. 
/// This method does it for all supported types of polarisation products
/// @param[in] stokes a single stokes enum of the polarisation product to convert
/// @return unsigned index
casa::uInt PolConverter::getIndex(casa::Stokes::StokesTypes stokes)
{
  casa::Vector<casa::Stokes::StokesTypes> buf(1,stokes);
  if (isCircular(buf)) {
     return casa::uInt(stokes)-casa::uInt(casa::Stokes::RR);
  } else if (isLinear(buf)) {
     return casa::uInt(stokes)-casa::uInt(casa::Stokes::XX);
  } else if (isStokes(buf)) {
     return casa::uInt(stokes)-casa::uInt(casa::Stokes::I);
  }
  ASKAPTHROW(AskapError, "Unsupported type of polarisation product in PolConverter::getIndex "<<
             int(stokes));  
}

/// @brief check whether stokes parameter correspond to cross-correlation
/// @details casacore allows to code single-dish polarisation and there are some reserved codes
/// as well. As we're doing lots of indexing, it is good to check that given parameter is
/// valid before doing any further work.
/// @note Technically, this and a few other helper methods should be part of casa::Stokes
/// @param[in] pol polarisation type
/// @return true, if it is a normal cross-correlation or I,Q,U or V.
bool PolConverter::isValid(casa::Stokes::StokesTypes pol)
{
  // casacore's order is checked by unit test
  if ((int(pol) >= int(casa::Stokes::I)) || (int(pol) <= int(casa::Stokes::V))) {
      return true;
  }
  if ((int(pol) >= int(casa::Stokes::RR)) || (int(pol) <= int(casa::Stokes::LL))) {
      return true;
  }
  if ((int(pol) >= int(casa::Stokes::XX)) || (int(pol) <= int(casa::Stokes::YY))) {
      return true;
  }
  if ((int(pol) >= int(casa::Stokes::RX)) || (int(pol) <= int(casa::Stokes::YL))) {
      return true;
  }
  return false;
}


