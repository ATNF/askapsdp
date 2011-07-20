/// @file
/// @brief polarisation cross-products of visibilities 
/// @details This is a helper class intended to ship around cross-products of
/// the components of visibility vector (model and measured). It is used in
/// preaveraged calibration and in the normal equations method which builds
/// normal equations using ComplexDiffMatrix and these cross-products
/// (i.e. not via DesignMatrix as for the calibration without preaveraging).
/// Such helper class is handy to have, otherwise the interface bloats up 
/// considerably. In addition, we can enforce symmetries (i.e. conj(Vi)*Vj =
/// conj(conj(Vj)*Vi)) and avoid calculation (and keeping) of all Npol^2 products.
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

#ifndef POL_X_PRODUCTS_H
#define POL_X_PRODUCTS_H

// casa includes
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>

namespace askap {

namespace scimath {

/// @brief polarisation cross-products of visibilities 
/// @details This is a helper class intended to ship around cross-products of
/// the components of visibility vector (model and measured). It is used in
/// preaveraged calibration and in the normal equations method which builds
/// normal equations using ComplexDiffMatrix and these cross-products
/// (i.e. not via DesignMatrix as for the calibration without preaveraging).
/// Such helper class is handy to have, otherwise the interface bloats up 
/// considerably. In addition, we can enforce symmetries (i.e. conj(Vi)*Vj =
/// conj(conj(Vj)*Vi)) and don't calculate keep all Npol^2 products.
/// @note This class uses reference semantics (i.e. handy to ship the data)
/// @ingroup fitting
class PolXProducts {
public:
   /// @brief basic constructor, uninitialised arrays
   /// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
   /// @note The arrays are left uninitialised after this constructor, their size have to be changed 
   /// before they can be used
   explicit PolXProducts(const casa::uInt npol);
   
   /// @brief constructor initialising arrays
   /// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
   /// @param[in] shape shape of the arrays without polarisation dimension which is always added last
   /// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
   /// @note This version of the constructor does initialise the arrays to the requested size and by default
   /// fills them with zeros.
   PolXProducts(const casa::uInt npol, const casa::IPosition &shape, const bool doZero = true);
   
   /// @brief obtain the slice at given position
   /// @details This method makes a slice of the underlying arrays along the polarisation axis 
   /// at the given position for other dimensions. Note, reference semantics implied.
   /// @param[in] pos position vector for all axes except the last one (polarisation). The vector size
   /// should be the dimension of arrays minus 1.
   /// @return the one dimensional slice at the given position
   PolXProducts slice(const casa::IPosition &pos);
   
   /// @brief obtain the slice at the given position
   /// @details This method makes a slice of the underlying arrays along the polarisation axis 
   /// at the given position for other dimensions. Note, unlike slice, this method makes a copy, so
   /// it needs a read-only access to the original buffer. 
   /// @param[in] pos position vector for all axes except the last one (polarisation). The vector size
   /// should be the dimension of arrays minus 1.
   /// @return the one dimensional slice at the given position
   PolXProducts roSlice(const casa::IPosition &pos) const;   
   
   /// @brief obtain the slice at the given position
   /// @details This is a specialisation of the method which makes a slice of the underlying arrays along 
   /// the polarisation axis at the given position for other dimensions. It is valid for 3-dimensional buffers
   /// (x,y + polarisation dimension). Note, reference semantics implied. 
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @return the one dimensional slice at the given position
   inline PolXProducts slice(const casa::uInt x, const casa::uInt y) 
     { return slice(casa::IPosition(2, casa::Int(x), casa::Int(y)));}
   
   /// @brief obtain the slice at the given position
   /// @details This specialisation works with 3-dimensional buffers (x,y + polarisation dimension) and takes
   /// the coordinates of the slice position explicitly. 
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @return the one dimensional slice at the given position
   inline PolXProducts roSlice(const casa::uInt x, const casa::uInt y) const
     { return roSlice(casa::IPosition(2, casa::Int(x), casa::Int(y)));}
   
   
   /// @brief resize the arrays storing products
   /// @details After a call to this method the class is put to the same state as after the call
   /// to the constructor with array initialisation.
   /// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
   /// @param[in] shape shape of the arrays without polarisation dimension which is always added last
   /// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
   void resize(const casa::uInt npol, const casa::IPosition &shape, const bool doZero = true);
   
   /// @brief resize without changing the number of polarisations
   /// @details This method is equivalent to the previous one, but the dimensionality of the visibility
   /// vector is not changed.
   /// @param[in] shape shape of the arrays without polarisation dimension which is always added last
   /// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
   void resize(const casa::IPosition &shape, const bool doZero = true);
   
   /// @brief reset buffers to zero
   /// @details This method resets accumulation without changing the dimensions
   void reset();
   
   // data access
   
   /// @brief obtain the value for model visibility cross-products
   /// @details This version of the method is intended to be used if the underlying arrays are
   /// 3-dimensional (i.e. cubes). The polarisation dimension index is obtained from the pair
   /// of given polarisations.
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @return the value of cross-product
   casa::Complex getModelProduct(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, const casa::uInt pol2) const;
    
   /// @brief obtain the value for model visibility cross-products
   /// @details This version of the method deals with the slice which only has polarisation dimension.
   /// The polarisation dimension index is obtained from the pair
   /// of given polarisations.
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @return the value of cross-product
   casa::Complex getModelProduct(const casa::uInt pol1, const casa::uInt pol2) const;

   /// @brief obtain the value for cross-products between model and measured visibilities
   /// @details This version of the method is intended to be used if the underlying arrays are
   /// 3-dimensional (i.e. cubes). The polarisation dimension index is obtained from the pair
   /// of given polarisations.
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @return the value of cross-product
   casa::Complex getModelMeasProduct(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, const casa::uInt pol2) const;
    
   /// @brief obtain the value for cross-products between model and measured visibilities
   /// @details This version of the method deals with the slice which only has polarisation dimension.
   /// The polarisation dimension index is obtained from the pair
   /// of given polarisations.
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @return the value of cross-product
   casa::Complex getModelMeasProduct(const casa::uInt pol1, const casa::uInt pol2) const;
   
   /// @brief add to the products buffer
   /// @details The real usage of the product buffers is to sum these products over the dataset. 
   /// This method encapsulates all index handling and adds up the given two complex numbers to the
   /// appropriate buffers. It is assumed that the buffers are 3-dimensional.
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @param[in] modelProduct a complex number to add to the modelProduct buffer   
   /// @param[in] modelMeasProduct a complex number to add to the modelMeasProduct buffer   
   void add(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, const casa::uInt pol2, 
            const casa::Complex modelProduct, const casa::Complex modelMeasProduct);
            
   /// @brief add to the model product buffer
   /// @details The real usage of the model product buffer. This method encapsulates
   /// index handling and adds up the given complex value to the buffer of model cross-products.
   /// This version of the method is intended for 3-dimensional buffers.
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @param[in] modelProduct a complex number to add to the modelProduct buffer   
   /// @note to avoid bugs with unnecessary addition we enforce here that pol1>=pol2
   void addModelProduct(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, 
            const casa::uInt pol2, const casa::Complex modelProduct);   

   /// @brief add to the model product buffer
   /// @details The real usage of the model product buffer. This method encapsulates
   /// index handling and adds up the given complex value to the buffer of model cross-products.
   /// This version of the method is intended for 1-dimensional buffers.
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @param[in] modelProduct a complex number to add to the modelProduct buffer   
   /// @note to avoid bugs with unnecessary addition we enforce here that pol1>=pol2
   void addModelProduct(const casa::uInt pol1, const casa::uInt pol2, const casa::Complex modelProduct);   
   
   /// @brief add to the model and measured product buffer
   /// @details The real usage of the model and measured product buffer. This method encapsulates
   /// index handling and adds up the given complex value to the buffer of model by measured cross-products.
   /// This version of the method is intended for 3-dimensional buffers.
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @param[in] modelMeasProduct a complex number to add to the modelMeasProduct buffer   
   /// @note For cross-products between model and measured data any combination of pol1 and
   /// pol2 is allowed (i.e. there is no restriction that pol1>=pol2)
   void addModelMeasProduct(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, 
            const casa::uInt pol2, const casa::Complex modelMeasProduct);   

   /// @brief add to the model and measured product buffer
   /// @details The real usage of the model and measured product buffer. This method encapsulates
   /// index handling and adds up the given complex value to the buffer of model by measured cross-products.
   /// This version of the method is intended for 1-dimensional buffers.
   /// @param[in] pol1 first polarisation coordinate of the pair forming the product
   /// @param[in] pol2 second polarisation coordinate of the pair forming the product
   /// @param[in] modelMeasProduct a complex number to add to the modelMeasProduct buffer   
   /// @note For cross-products between model and measured data any combination of pol1 and
   /// pol2 is allowed (i.e. there is no restriction that pol1>=pol2)
   void addModelMeasProduct(const casa::uInt pol1, const casa::uInt pol2, const casa::Complex modelMeasProduct);   
   
   
   /// @brief obtain number of polarisations
   /// @return the number of polarisations
   inline casa::uInt nPol() const { return itsNPol; }

protected:   
   /// @brief setup a slicer for a given position
   /// @details This is a helper method used in methods making a slice along polarisation dimension.
   /// Given the position, it forms a slicer object for buffer arrays.
   /// @param[in] pos position vector for all axes except the last one (polarisation). The vector size
   /// should be the dimension of arrays minus 1.
   /// @param[in] forMeasProduct if true the last dimension of the array is assumed to be npol squared
   /// @return an instance of the slicer object
   casa::Slicer getSlicer(const casa::IPosition &pos, bool forMeasProduct) const;      

   /// @brief polarisation index for a given pair of polarisations
   /// @details We need to keep track of cross-polarisation products. These cross-products are
   /// kept alongside with the parallel-hand products in the same cube. This method translates
   /// a pair of polarisation products (each given by a number ranging from 0 to nPol) into a
   /// single index, which can be used to extract the appropriate statistics out of the cubes
   /// itsModelProducts and itsModelMeasProducts
   /// @param[in] pol1 polarisation of the first visibility
   /// @param[in] pol2 polarisation of the second visibility
   /// @return an index into plane of itsModelProducts and itsModelMeasProducts
   casa::uInt polToIndex(casa::uInt pol1, casa::uInt pol2) const;

   /// @brief polarisations corresponding to a given index
   /// @details We need to keep track of cross-polarisation products. These cross-products are
   /// kept alongside with the parallel-hand products in the same cube. This method is 
   /// a reverse to polToIndex and translates an index back to two polarisation products
   std::pair<casa::uInt,casa::uInt> indexToPol(casa::uInt index) const; 
   
private:
   /// @brief number of polarisations (i.e. dimension of visibility vector)
   casa::uInt itsNPol;     
   
   /// @brief products of components of model visibility
   casa::Array<casa::Complex> itsModelProducts;
   
   /// @brief products of components of model visibility by measured visibility
   casa::Array<casa::Complex> itsModelMeasProducts;   
};

} // namespace scimath

} // namespace askap

#endif // #ifndef POL_X_PRODUCTS_H


