/// @file
/// @brief Normal equations with an approximation for imaging
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In the latter approach, the 
/// matrix is approximated by a sum of diagonal and shift invariant matrices. 
/// This class represents the approximated case, and is used with imaging 
/// algorithms.
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <fitting/DesignMatrix.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

#include <askap/AskapError.h>

#include <utils/DeepCopyUtils.h>

#include <stdexcept>
#include <string>
#include <map>
#include <cmath>
#include <vector>

using namespace LOFAR;

using std::abs;
using std::map;
using std::string;
using std::vector;

namespace askap
{
  namespace scimath
  {


    ImagingNormalEquations::ImagingNormalEquations() {};
    
    ImagingNormalEquations::ImagingNormalEquations(const Params& ip) : itsParams(ip.clone())
    {
      vector<string> names=ip.freeNames();
      vector<string>::iterator iterRow;
      vector<string>::iterator iterCol;
      for (iterRow=names.begin();iterRow!=names.end();++iterRow)
      {
        itsDataVector[*iterRow]=casa::Vector<double>(0);
        itsShape[*iterRow]=casa::IPosition();
        itsReference[*iterRow]=casa::IPosition();
        itsNormalMatrixSlice[*iterRow]=casa::Vector<double>(0);
        itsNormalMatrixDiagonal[*iterRow]=casa::Vector<double>(0);
      }
    }


    /// @brief copy constructor
    /// @details Data members of this class are non-trivial types including
    /// std containers of casa containers. The letter are copied by reference by default. We,
    /// therefore, need this copy constructor to achieve proper copying.
    /// @param[in] src input measurement equations to copy from
    ImagingNormalEquations::ImagingNormalEquations(const ImagingNormalEquations &src) :
         itsParams(src.itsParams), itsShape(src.itsShape), itsReference(src.itsReference)
    {
      deepCopyOfSTDMap(src.itsNormalMatrixSlice, itsNormalMatrixSlice);
      deepCopyOfSTDMap(src.itsNormalMatrixDiagonal, itsNormalMatrixDiagonal);
      deepCopyOfSTDMap(src.itsDataVector, itsDataVector);      
    }


    ImagingNormalEquations::~ImagingNormalEquations()
    {
      reset();
    }
  
  /// @brief Merge these normal equations with another
  /// @details Combining two normal equations depends on the actual class type
  /// (different work is required for a full matrix and for an approximation).
  /// This method must be overriden in the derived classes for correct 
  /// implementation. 
  /// This means that we just add
  /// @param[in] src an object to get the normal equations from
  void ImagingNormalEquations::merge(const INormalEquations& src)
  {
    try {
      const ImagingNormalEquations &other = 
                         dynamic_cast<const ImagingNormalEquations&>(src);    
      if (!other.itsParams) {
          // do nothing, src is empty
          return;
      }
      if (!itsParams) {
          // this object is empty, just do an assignment
          operator=(other);
          return;
      }
      ASKAPDEBUGASSERT(itsParams && other.itsParams);
      itsParams->merge(*other.itsParams);
      const vector<string> names=itsParams->freeNames();
      //vector<string>::const_iterator iterRow;
      vector<string>::const_iterator iterCol;

      for (iterCol=names.begin();iterCol!=names.end();++iterCol)
      {
        if (other.itsDataVector.find(*iterCol) == other.itsDataVector.end()) {
            continue;
        }
        ASKAPDEBUGASSERT(other.itsDataVector.find(*iterCol) != other.itsDataVector.end()); 
        if(itsDataVector[*iterCol].size()!=other.itsDataVector.find(*iterCol)->second.size())
        {
          itsDataVector[*iterCol].assign(other.itsDataVector.find(*iterCol)->second);
        }
        else
        {
          itsDataVector[*iterCol]+=other.itsDataVector.find(*iterCol)->second;
        }
            
        itsShape[*iterCol].resize(0);
        ASKAPDEBUGASSERT(other.itsShape.find(*iterCol) != other.itsShape.end());
        itsShape[*iterCol]=other.itsShape.find(*iterCol)->second;
        
        itsReference[*iterCol].resize(0);
        ASKAPDEBUGASSERT(other.itsReference.find(*iterCol) != other.itsReference.end());
        itsReference[*iterCol] = other.itsReference.find(*iterCol)->second;
        
        ASKAPDEBUGASSERT(other.itsNormalMatrixSlice.find(*iterCol) != other.itsNormalMatrixSlice.end());
        if(itsNormalMatrixSlice[*iterCol].shape()!=other.itsNormalMatrixSlice.find(*iterCol)->second.shape())
        {
          itsNormalMatrixSlice[*iterCol].assign(other.itsNormalMatrixSlice.find(*iterCol)->second);
        }
        else
        {
          itsNormalMatrixSlice[*iterCol]+=other.itsNormalMatrixSlice.find(*iterCol)->second;
        }
 
        ASKAPDEBUGASSERT(other.itsNormalMatrixDiagonal.find(*iterCol) != other.itsNormalMatrixDiagonal.end());
        if(itsNormalMatrixDiagonal[*iterCol].shape()!=other.itsNormalMatrixDiagonal.find(*iterCol)->second.shape())
        {
          itsNormalMatrixDiagonal[*iterCol].assign(other.itsNormalMatrixDiagonal.find(*iterCol)->second);
        }
        else
        {
          itsNormalMatrixDiagonal[*iterCol]+=other.itsNormalMatrixDiagonal.find(*iterCol)->second;
        }       
      }     
    }
    catch (const std::bad_cast &bc) {
        ASKAPTHROW(AskapError, "An attempt to merge NormalEquations with an "
                    "equation of incompatible type");
    }
  }

    const std::map<string, casa::Vector<double> >& ImagingNormalEquations::normalMatrixDiagonal() const
    {
      return itsNormalMatrixDiagonal;
    }

    const std::map<string, casa::Vector<double> >& ImagingNormalEquations::normalMatrixSlice() const
    {
      return itsNormalMatrixSlice;
    }
 
/// @brief normal equations for given parameters
/// @details In the current framework, parameters are essentially 
/// vectors, not scalars. Each element of such vector is treated
/// independently (but only vector as a whole can be fixed). As a 
/// result any element of the normal matrix is another matrix for
/// all non-scalar parameters. For scalar parameters each such
/// matrix has a shape of [1,1].
/// @param[in] par1 the name of the first parameter
/// @param[in] par2 the name of the second parameter
/// @return one element of the sparse normal matrix (a dense matrix)
const casa::Matrix<double>& ImagingNormalEquations::normalMatrix(const std::string &par1, 
                        const std::string &par2) const
{
   ASKAPTHROW(AskapError, 
               "ImagingNormalEquations::normalMatrix has not yet been implemented");
   const casa::Matrix<double> *buf = 0;
   return *buf; // the code shouldn't get this far.                             
}

/// @brief data vector for a given parameter
/// @details In the current framework, parameters are essentially 
/// vectors, not scalars. Each element of such vector is treated
/// independently (but only vector as a whole can be fixed). As a 
/// result any element of the normal matrix as well as an element of the
/// data vector are, in general, matrices, not scalar. For the scalar 
/// parameter each element of data vector is a vector of unit length.
/// @param[in] par the name of the parameter of interest
/// @return one element of the sparse data vector (a dense vector)
const casa::Vector<double>& ImagingNormalEquations::dataVector(const std::string &par) const
{
   std::map<string, casa::Vector<double> >::const_iterator cIt = 
                                     itsDataVector.find(par);
   ASKAPASSERT(cIt != itsDataVector.end());                                  
   return cIt->second;
}

const std::map<std::string, casa::Vector<double> >& ImagingNormalEquations::dataVector() const
{
  return itsDataVector;
}

/// Return shape
    const std::map<string, casa::IPosition >& ImagingNormalEquations::shape() const
    {
      return itsShape;
    }

/// Return reference
    const std::map<string, casa::IPosition >& ImagingNormalEquations::reference() const
    {
      return itsReference;
    }

    void ImagingNormalEquations::reset()
    {
      map<string, casa::Vector<double> >::iterator iterRow;
      map<string, casa::Matrix<double> >::iterator iterCol;
      for (iterRow=itsDataVector.begin();iterRow!=itsDataVector.end();iterRow++)
      {
        itsDataVector[iterRow->first].resize();
        itsDataVector[iterRow->first]=casa::Vector<double>(0);
        itsShape[iterRow->first].resize(0);
        itsShape[iterRow->first]=casa::IPosition();
        itsReference[iterRow->first].resize(0);
        itsReference[iterRow->first]=casa::IPosition();
        itsNormalMatrixSlice[iterRow->first].resize();
        itsNormalMatrixSlice[iterRow->first]=casa::Vector<double>(0);
        itsNormalMatrixDiagonal[iterRow->first].resize();
        itsNormalMatrixDiagonal[iterRow->first]=casa::Vector<double>(0);
      }
    }

 

    void ImagingNormalEquations::addSlice(const string& name,
      const casa::Vector<double>& normalmatrixslice,
      const casa::Vector<double>& normalmatrixdiagonal,
      const casa::Vector<double>& datavector,
      const casa::IPosition& shape,
      const casa::IPosition& reference)
    {

      if(datavector.size()!=itsDataVector[name].size())
      {
        ASKAPDEBUGASSERT(itsDataVector[name].size() == 0);
        itsDataVector[name]=datavector;
      }
      else
      {
        itsDataVector[name]+=datavector;
      }
      if(normalmatrixdiagonal.shape()!=itsNormalMatrixDiagonal[name].shape())
      {
        ASKAPDEBUGASSERT(itsNormalMatrixDiagonal[name].size() == 0);
        itsNormalMatrixDiagonal[name]=normalmatrixdiagonal;
      }
      else
      {
        itsNormalMatrixDiagonal[name]+=normalmatrixdiagonal;
      }
      if(normalmatrixslice.shape()!=itsNormalMatrixSlice[name].shape()) 
      { 
         itsNormalMatrixSlice[name]=normalmatrixslice; 
      } 
      else 
      { 
         itsNormalMatrixSlice[name]+=normalmatrixslice; 
      }
      itsShape[name].resize(0);
      itsShape[name]=shape;
      itsReference[name].resize(0);
      itsReference[name]=reference;
    }
    
    /// @brief Store slice of the normal matrix for a given parameter. 
    ///
    /// This means
    /// that the cross terms between parameters are excluded and only
    /// a slice of the normal matrix is retained.
    /// @param name Name of parameter
    /// @param normalmatrixslice Slice of normal matrix for this parameter
    /// @param normalmatrixdiagonal Diagonal of normal matrix for
    ///        this parameter
    /// @param datavector Data vector for this parameter
    /// @param reference Reference point for the slice
    void ImagingNormalEquations::addSlice(const string& name,
                    const casa::Vector<double>& normalmatrixslice,
                    const casa::Vector<double>& normalmatrixdiagonal,
                    const casa::Vector<double>& datavector,
                    const casa::IPosition& reference)
    {
      addSlice(name,normalmatrixslice,normalmatrixdiagonal,datavector,
               casa::IPosition(1, datavector.nelements()), reference);
    }

    void ImagingNormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrixdiagonal,
      const casa::Vector<double>& datavector, const casa::IPosition& shape)
    {

      if(datavector.size()!=itsDataVector[name].size())
      {
        ASKAPDEBUGASSERT(itsDataVector[name].size() == 0);
        itsDataVector[name]=datavector;
      }
      else
      {
        itsDataVector[name]+=datavector;
      }
      if(normalmatrixdiagonal.shape()!=itsNormalMatrixDiagonal[name].shape())
      {
        ASKAPDEBUGASSERT(itsNormalMatrixDiagonal[name].size() == 0);
        itsNormalMatrixDiagonal[name]=normalmatrixdiagonal;
      }
      else
      {
        itsNormalMatrixDiagonal[name]+=normalmatrixdiagonal;
      }
      itsShape[name].resize(0);
      itsShape[name]=shape;
    }

    void ImagingNormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrixdiagonal,
      const casa::Vector<double>& datavector)
    {
      casa::IPosition shape(1, datavector.nelements());
      addDiagonal(name, normalmatrixdiagonal, datavector, shape);
    }

    const Params& ImagingNormalEquations::parameters() const
    {
      return *itsParams;
    }

    INormalEquations::ShPtr ImagingNormalEquations::clone() const
    {
      return INormalEquations::ShPtr(new ImagingNormalEquations(*this));
    }

    /// @brief write the object to a blob stream
    /// @param[in] os the output stream
    void ImagingNormalEquations::writeToBlob(LOFAR::BlobOStream& os) const
    {
      os << *(itsParams) << itsNormalMatrixSlice 
        << itsNormalMatrixDiagonal << itsShape << itsReference << itsDataVector; 
    }
    
    /// @brief read the object from a blob stream
    /// @param[in] is the input stream
    /// @note Not sure whether the parameter should be made const or not 
    void ImagingNormalEquations::readFromBlob(LOFAR::BlobIStream& is) 
    {
      itsParams = Params::ShPtr(new Params());
      is >> *(itsParams)  >> itsNormalMatrixSlice 
         >> itsNormalMatrixDiagonal >> itsShape >> itsReference 
         >> itsDataVector;
    }
    
    /// @brief obtain all parameters dealt with by these normal equations
    /// @details Normal equations provide constraints for a number of 
    /// parameters (i.e. unknowns of these equations). This method returns
    /// a vector with the string names of all parameters mentioned in the
    /// normal equations represented by the given object.
    /// @return a vector listing the names of all parameters (unknowns of these equations)
    /// @note if ASKAP_DEBUG is set some extra checks on consistency of these 
    /// equations are done
    std::vector<std::string> ImagingNormalEquations::unknowns() const {
      std::vector<std::string> result; 
      result.reserve(itsNormalMatrixSlice.size());
      for (std::map<std::string, casa::Vector<double> >::const_iterator ci = itsNormalMatrixSlice.begin();
           ci!=itsNormalMatrixSlice.end(); ++ci) {
           result.push_back(ci->first);

// extra checks in debug mode
#ifdef ASKAP_DEBUG
           ASKAPCHECK(itsNormalMatrixDiagonal.find(ci->first) != itsNormalMatrixDiagonal.end(),
                      "Parameter "<<ci->first<<" is present in the matrix slice but is missing in the diagonal");
           ASKAPCHECK(itsShape.find(ci->first) != itsShape.end(),
                      "Parameter "<<ci->first<<" is present in the matrix slice but is missing in the shape map");
           ASKAPCHECK(itsReference.find(ci->first) != itsReference.end(),
                      "Parameter "<<ci->first<<" is present in the matrix slice but is missing in the reference map");
           ASKAPCHECK(itsDataVector.find(ci->first) != itsDataVector.end(),
                      "Parameter "<<ci->first<<" is present in the matrix slice but is missing in the data vector");                      
#endif // #ifdef ASKAP_DEBUG  
      }
      return result;
    } // unknowns method
    
  } // namespace scimath
} // namespace askap
