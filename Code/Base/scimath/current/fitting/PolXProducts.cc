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

// own includes
#include <fitting/PolXProducts.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::scimath;

/// @brief basic constructor, uninitialised arrays
/// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
/// @note The arrays are left uninitialised after this constructor, their size have to be changed 
/// before they can be used
PolXProducts::PolXProducts(const casa::uInt npol) : itsNPol(npol) {}
   
/// @brief constructor initialising arrays
/// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
/// @param[in] shape shape of the arrays without polarisation dimension which is always added last
/// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
/// @note This version of the constructor does initialise the arrays to the requested size and by default
/// fills them with zeros.
PolXProducts::PolXProducts(const casa::uInt npol, const casa::IPosition &shape, const bool doZero) : itsNPol(npol),
     itsModelProducts(shape.concatenate(casa::IPosition(1,int(npol*(npol+1)/2)))),
     itsModelMeasProducts(shape.concatenate(casa::IPosition(1,int(npol*(npol+1)/2)))) 
{
  if (doZero) {
      itsModelProducts.set(0.);
      itsModelMeasProducts.set(0.);
  }
}
   
/// @brief obtain the slice at given position
/// @details This method makes a slice of the underlying arrays along the polarisation axis 
/// at the given position for other dimensions. Note, reference semantics implied.
/// @param[in] pos position vector for all axes except the last one (polarisation). The vector size
/// should be the dimension of arrays minus 1.
/// @return the one dimensional slice at the given position
PolXProducts PolXProducts::slice(const casa::IPosition &pos) 
{
  ASKAPDEBUGASSERT(nPol()>0);
  ASKAPDEBUGASSERT(pos.nelements() + 1 == itsModelProducts.shape().nelements());
  ASKAPDEBUGASSERT(itsModelMeasProducts.shape() == itsModelProducts.shape());

  PolXProducts result(nPol());
  // setup Slicer  
  const casa::IPosition endPos = pos.concatenate(casa::IPosition(1,int(nPol()*(nPol()+1)/2-1)));
  casa::IPosition startPos(endPos);
  startPos(pos.nelements()) = 0;
  const casa::Slicer slc(startPos, endPos, casa::Slicer::endIsLast);
  // take the slices
  result.itsModelProducts = itsModelProducts(slc).nonDegenerate();
  result.itsModelMeasProducts = itsModelMeasProducts(slc).nonDegenerate();
  return result;
}
   
/// @brief resize the arrays storing products
/// @details After a call to this method the class is put to the same state as after the call
/// to the constructor with array initialisation.
/// @param[in] npol number of polarisations (i.e. dimension of visibility vector)
/// @param[in] shape shape of the arrays without polarisation dimension which is always added last
/// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
void PolXProducts::resize(const casa::uInt npol, const casa::IPosition &shape, const bool doZero) 
{
  itsNPol = npol;
  resize(shape,doZero);
}
   
/// @brief resize without changing the number of polarisations
/// @details This method is equivalent to the previous one, but the dimensionality of the visibility
/// vector is not changed.
/// @param[in] shape shape of the arrays without polarisation dimension which is always added last
/// @param[in] doZero if true (default), the buffer arrays are filled with zeros. 
void PolXProducts::resize(const casa::IPosition &shape, const bool doZero)
{
  const casa::IPosition targetShape = shape.concatenate(casa::IPosition(1,int(itsNPol*(itsNPol+1)/2)));
  itsModelProducts.resize(targetShape);
  itsModelMeasProducts.resize(targetShape); 
  if (doZero) {
      reset();
  }  
}

/// @brief reset buffers to zero
/// @details This method resets accumulation without changing the dimensions
void PolXProducts::reset() {
  itsModelProducts.set(0.);
  itsModelMeasProducts.set(0.);
}

   
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
casa::Complex PolXProducts::getModelProduct(const casa::uInt x, const casa::uInt y, 
                         const casa::uInt pol1, const casa::uInt pol2) const
{
  ASKAPDEBUGASSERT(itsModelProducts.shape().nelements() == 3);

  // products are indexed with the first polarisation index being the largest. If the pol1<pol2 pair
  // is requested we need to conjugate
  if (pol1 >= pol2) { 
      const int index = int(polToIndex(pol1,pol2));
      return itsModelProducts(casa::IPosition(3,int(x),int(y),index));
  }
  const int index = int(polToIndex(pol2,pol1));
  return conj(itsModelProducts(casa::IPosition(3,int(x),int(y),index)));  
}

    
/// @brief obtain the value for model visibility cross-products
/// @details This version of the method deals with the slice which only has polarisation dimension.
/// The polarisation dimension index is obtained from the pair
/// of given polarisations.
/// @param[in] pol1 first polarisation coordinate of the pair forming the product
/// @param[in] pol2 second polarisation coordinate of the pair forming the product
/// @return the value of cross-product
casa::Complex PolXProducts::getModelProduct(const casa::uInt pol1, const casa::uInt pol2) const
{
  ASKAPDEBUGASSERT(itsModelProducts.shape().nelements() == 1);
  // products are indexed with the first polarisation index being the largest. If the pol1<pol2 pair
  // is requested we need to conjugate
  if (pol1 >= pol2) { 
      const int index = int(polToIndex(pol1,pol2));
      return itsModelProducts(casa::IPosition(1,index));
  }
  const int index = int(polToIndex(pol2,pol1));
  return conj(itsModelProducts(casa::IPosition(1,index)));
}

/// @brief obtain the value for cross-products between model and measured visibilities
/// @details This version of the method is intended to be used if the underlying arrays are
/// 3-dimensional (i.e. cubes). The polarisation dimension index is obtained from the pair
/// of given polarisations.
/// @param[in] x first coordinate
/// @param[in] y second coordinate
/// @param[in] pol1 first polarisation coordinate of the pair forming the product
/// @param[in] pol2 second polarisation coordinate of the pair forming the product
/// @return the value of cross-product
casa::Complex PolXProducts::getModelMeasProduct(const casa::uInt x, const casa::uInt y, 
                                                const casa::uInt pol1, const casa::uInt pol2) const
{
  ASKAPDEBUGASSERT(itsModelMeasProducts.shape().nelements() == 3);

  // products are indexed with the first polarisation index being the largest. If the pol1<pol2 pair
  // is requested we need to conjugate
  if (pol1 >= pol2) { 
      const int index = int(polToIndex(pol1,pol2));
      return itsModelMeasProducts(casa::IPosition(3,int(x),int(y),index));
  }
  const int index = int(polToIndex(pol2,pol1));
  return conj(itsModelMeasProducts(casa::IPosition(3,int(x),int(y),index)));  
}
                                                
    
/// @brief obtain the value for cross-products between model and measured visibilities
/// @details This version of the method deals with the slice which only has polarisation dimension.
/// The polarisation dimension index is obtained from the pair
/// of given polarisations.
/// @param[in] pol1 first polarisation coordinate of the pair forming the product
/// @param[in] pol2 second polarisation coordinate of the pair forming the product
/// @return the value of cross-product
casa::Complex PolXProducts::getModelMeasProduct(const casa::uInt pol1, const casa::uInt pol2) const
{
  ASKAPDEBUGASSERT(itsModelMeasProducts.shape().nelements() == 1);
  // products are indexed with the first polarisation index being the largest. If the pol1<pol2 pair
  // is requested we need to conjugate
  if (pol1 >= pol2) { 
      const int index = int(polToIndex(pol1,pol2));
      return itsModelMeasProducts(casa::IPosition(1,index));
  }
  const int index = int(polToIndex(pol2,pol1));
  return conj(itsModelMeasProducts(casa::IPosition(1,index)));  
}

   
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
void PolXProducts::add(const casa::uInt x, const casa::uInt y, const casa::uInt pol1, const casa::uInt pol2, 
            const casa::Complex modelProduct, const casa::Complex modelMeasProduct)
{
  ASKAPDEBUGASSERT(itsModelProducts.shape().nelements() == 3);
  ASKAPDEBUGASSERT(itsModelMeasProducts.shape().nelements() == 3);
  // we can enforce pol1 >= pol2 here making the user responsible for correct conjugation of the cross terms
  // this is just the easiest option because technically we don't need generality
  ASKAPDEBUGASSERT(pol1 >= pol2);
  const int index = int(polToIndex(pol1,pol2));
  const casa::IPosition pos(3,int(x),int(y),index);
  itsModelProducts(pos) += modelProduct;
  itsModelMeasProducts(pos) += modelMeasProduct;
}
   
/// @brief polarisation index for a given pair of polarisations
/// @details We need to keep track of cross-polarisation products. These cross-products are
/// kept alongside with the parallel-hand products in the same cube. This method translates
/// a pair of polarisation products (each given by a number ranging from 0 to nPol) into a
/// single index, which can be used to extract the appropriate statistics out of the cubes
/// itsModelProducts and itsModelMeasProducts
/// @param[in] pol1 polarisation of the first visibility
/// @param[in] pol2 polarisation of the second visibility
/// @return an index into plane of itsModelProducts and itsModelMeasProducts
casa::uInt PolXProducts::polToIndex(casa::uInt pol1, casa::uInt pol2) const
{
  const casa::uInt npol = nPol();
  ASKAPDEBUGASSERT((pol1<npol) && (pol2<npol));
  if (pol1 == pol2) {
      return pol1;
  }
  // the code below is generic, but it is handy to enforce that pol1>=pol2
  // here, because otherwise this condition has to be taken into account in other 
  // parts of the code (i.e. when we decide whether to conjugate or not)
  ASKAPCHECK(pol1 >= pol2, "Expect pol1>=pol2 you have pol1="<<pol1<<" pol2="<<pol2);
  //
  const casa::uInt minPol = casa::min(pol1,pol2);
  const casa::uInt maxPol = casa::max(pol1,pol2);
  // order: parallel hand, (1,0), (2,0), (2,1), (3,0),...
  const casa::uInt index = npol + minPol + (maxPol - 1) * maxPol / 2;
  ASKAPDEBUGASSERT(index < npol * (npol+1) / 2);
  return index;
}

/// @brief polarisations corresponding to a given index
/// @details We need to keep track of cross-polarisation products. These cross-products are
/// kept alongside with the parallel-hand products in the same cube. This method is 
/// a reverse to polToIndex and translates an index back to two polarisation products
std::pair<casa::uInt,casa::uInt> PolXProducts::indexToPol(casa::uInt index) const
{
  const casa::uInt npol = nPol();
  if (index < npol) {
      // parallel-hand products come first
      return std::pair<casa::uInt, casa::uInt>(index,index);
  }
  index -= npol;
  for (casa::uInt polMax = 1, sum = 0; polMax<npol; ++polMax) {
       if (index < sum + polMax) {
           return std::pair<casa::uInt, casa::uInt>(polMax, index - sum);
       }
       sum += polMax;
  }
  ASKAPTHROW(AskapError, "Index "<<index<<" exceeds maximum possible for nPol="<<npol);
}
            


