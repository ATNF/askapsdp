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

#ifndef POL_CONVERTER_H
#define POL_CONVERTER_H

// std includes
#include <string>
#include <vector>

// casa includes
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/Stokes.h>

namespace askap {

namespace scimath {

/// @brief Converter of polarisation frames
/// @details This is the class which handles polarisation frame conversion and contains some
/// helper methods related to it (i.e. converting strings into Stokes enums). It may eventually
/// replace or become derived from IPolSelector, which is not used at the moment.
/// @note At this stage this class is incompatible with the converters used to create data iterator.
/// It is not clear at the moment whether this class should be modified to be used as such converter 
/// too.
/// @ingroup utils
struct PolConverter {

  /// @brief constructor of the converter between two frames
  /// @details
  /// @param[in] polFrameIn input polarisation frame defined as a vector of Stokes enums
  /// @param[in] polFrameOut output polarisation frame defined as a vector of Stokes enums
  /// @param[in] checkUnspecifiedProducts if true (default), the code checks that all 
  ///            polarisation products missing in the input frame are multiplied by 0 (and 
  ///            therefore don't affect the result), see itsCheckUnspecifiedProducts for more info 
  PolConverter(const casa::Vector<casa::Stokes::StokesTypes> &polFrameIn,
               const casa::Vector<casa::Stokes::StokesTypes> &polFrameOut,
               bool checkUnspecifiedProducts = true);
  
  /// @brief default constructor - no conversion
  /// @details Constructed via this method the object passes all visibilities intact
  PolConverter();
  
  /// @brief main method doing conversion
  /// @details Convert the given visibility vector between two polarisation frames supplied
  /// in the constructor.
  /// @param[in] vis visibility vector
  /// @return converted visibility vector 
  /// @note vis should have the same size (<=4) as both polFrames passed in the constructor, 
  /// the output vector will have the same size.
  casa::Vector<casa::Complex> operator()(casa::Vector<casa::Complex> vis) const;

  /// @brief check whether this conversion is void
  /// @return true if conversion is void, false otherwise
  inline bool isVoid() const throw() {return itsVoid;}

  /// @brief test if frame is linear
  /// @param[in] polFrame polarisation frame defined as a vector of Stokes enums
  /// @return true, if the frame is linear
  static inline bool isLinear(const casa::Vector<casa::Stokes::StokesTypes> &polFrame) 
     { return sameFrame(polFrame, casa::Stokes::XX);}
  
  /// @brief test if frame is circular
  /// @param[in] polFrame polarisation frame defined as a vector of Stokes enums
  /// @return true, if the frame is circular
  static inline bool isCircular(const casa::Vector<casa::Stokes::StokesTypes> &polFrame)
     { return sameFrame(polFrame, casa::Stokes::RR);}
  
  /// @brief test if frame is stokes
  /// @param[in] polFrame polarisation frame defined as a vector of Stokes enums
  /// @return true, if the frame is circular
  static inline bool isStokes(const casa::Vector<casa::Stokes::StokesTypes> &polFrame)
       { return sameFrame(polFrame, casa::Stokes::I);}
  
  /// @brief convert string representation into a vector of Stokes enums
  /// @details It is convenient to define polarisation frames like "xx,xy,yx,yy" or "iquv". 
  /// This method does it and return a vector of Stokes enums. Comma symbol is ignored. i.e.
  /// "iquv" and "i,q,u,v" are equivalent.
  /// @param[in] frame a string representation of the frame
  /// @return vector with Stokes enums 
  static casa::Vector<casa::Stokes::StokesTypes> fromString(const std::string &frame);
  
  /// @brief convert string representation into a vector of Stokes enums
  /// @details This version of the method accept string representations in a vector and doesn't
  /// parse the concatenated string.
  /// @param[in] products vector of strings representation of the frame
  /// @return vector with Stokes enums
  static casa::Vector<casa::Stokes::StokesTypes> fromString(const std::vector<std::string> &products);
  
  /// @brief convert a vector of Stokes enums into a vector of strings
  /// @details This method does a reverse job to fromString. It converts a vector of stokes enums 
  /// into a vector of strings (with one to one correspondence between elements)
  /// @param[in] frame vector of stokes enums
  /// @return vector with string represenation
  static std::vector<std::string> toString(const casa::Vector<casa::Stokes::StokesTypes> &frame);
  
  /// @brief compare two vectors of Stokes enums
  /// @param[in] first first polarisation frame
  /// @param[in] second second polarisation frame
  /// @return true if two given frames are the same, false if not.
  static bool equal(const casa::Vector<casa::Stokes::StokesTypes> &first,
                    const casa::Vector<casa::Stokes::StokesTypes> &second);
  
  /// @brief number of polarisation planes in the input
  /// @details This is a helper method giving the expected dimensionality of the input vector
  /// @return expected number of polarisation planes for the input visibility vector
  inline casa::uInt nInputDim() const { return itsPolFrameIn.nelements();}
  
  /// @brief number of polarisation planes in the output
  /// @details This is a helper method giving the dimensionality of the output vector
  /// @return number of polarisation planes in the output visibility vector
  inline casa::uInt nOutputDim() const { return itsPolFrameOut.nelements();}
  
  /// @brief test if frame matches a given stokes enum
  /// @param[in] polFrame polarisation frame defined as a vector of Stokes enums
  /// @param[in] stokes a single stokes enum defining the frame (should be the first in the set)
  /// @return true, if the given vector and one stokes enum belong to the same frame 
  static bool sameFrame(const casa::Vector<casa::Stokes::StokesTypes> &polFrame,
                        casa::Stokes::StokesTypes stokes);  

  /// @brief return index of a particular polarisation
  /// @details To be able to fill matrices efficiently we want to convert, say IQUV into 0,1,2,3. 
  /// This method does it for all supported types of polarisation products
  /// @param[in] stokes a single stokes enum of the polarisation product to convert
  /// @return unsigned index
  static casa::uInt getIndex(casa::Stokes::StokesTypes stokes);
  
  /// @brief reverse method for getIndex
  /// @details convert index into stokes enum. Because the same index can correspond to a number
  /// of polarisation products (meaning of index is frame-dependent), a second parameter is
  /// required to unambiguate it. It can be any stokes enum of the frame, not necessarily the
  /// first one. 
  /// @param[in] index an index to convert
  /// @param[in] stokes any stokes enum from the working frame
  /// @note This method is actually used only to provide a sensible message in the exception. No
  /// other code depends on it.
  static casa::Stokes::StokesTypes stokesFromIndex(casa::uInt index, casa::Stokes::StokesTypes stokes);

  /// @brief check whether stokes parameter correspond to cross-correlation
  /// @details casacore allows to code single-dish polarisation and there are some reserved codes
  /// as well. As we're doing lots of indexing, it is good to check that given parameter is
  /// valid before doing any further work.
  /// @note Technically, this and a few other helper methods should be part of casa::Stokes
  /// @param[in] pol polarisation type
  /// @return true, if it is a normal cross-correlation or I,Q,U or V.
  static bool isValid(casa::Stokes::StokesTypes pol);
  
protected:
  /// @brief build transformation matrix
  /// @details This is the core of the algorithm, this method builds the transformation matrix
  /// given the two frames .
  /// @param[in] polFrameIn input polarisation frame defined as a vector of Stokes enums
  /// @param[in] polFrameOut output polarisation frame defined as a vector of Stokes enums
  void fillMatrix(const casa::Vector<casa::Stokes::StokesTypes> &polFrameIn,
                  const casa::Vector<casa::Stokes::StokesTypes> &polFrameOut);
  
  /// @brief fill matrix describing parallactic angle rotation
  /// @details 
  /// @param[in] pa1 parallactic angle on the first antenna
  /// @param[in] pa2 parallactic angle on the second antenna
  void fillPARotationMatrix(double pa1, double pa2);
  
private:
  /// @brief no operation flag
  /// @details True if itsPolFrameOut == itsPolFrameIn or if class has been 
  /// created with the default constructor
  bool itsVoid;
  
  /// @brief transformation matrix 
  /// @details to convert input polarisation frame to the target one
  casa::Matrix<casa::Complex> itsTransform;
  
  /// @brief matrix describing parallactic angle rotation
  casa::Matrix<casa::Double> itsPARotation;
  
  // the following methods may be removed in the future
  
  /// @brief polarisation frame assumed for input (stokes enums)
  casa::Vector<casa::Stokes::StokesTypes> itsPolFrameIn;                     
  
  /// @brief target polarisation frame (stokes enums)
  casa::Vector<casa::Stokes::StokesTypes> itsPolFrameOut;
  
  /// @brief true to check that all required polarisation products are present in input
  /// @details Polarisation transformation may not always preserve the dimension of the polarisation
  /// vector (i.e. one may want just stokes I from full stokes vector). If this flag is true, 
  /// the method would check that omitted polarisations are actually multiplied by 0, and throws an
  /// exception if it is not the case. This is appropriate for gridding. A more relaxed behavior 
  /// equivalent to setting all unknown polarisation products to 0 is appropriate for degridding.
  /// This is achieved if this parameter is set to false.
  const bool itsCheckUnspecifiedProducts;
};

} // namespace scimath

} // namespace askap

#endif // #ifndef POL_CONVERTER_H
