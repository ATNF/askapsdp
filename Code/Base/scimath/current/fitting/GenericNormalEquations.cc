/// @file
/// @brief Normal equations without any approximation
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In this approach, the matrix
/// is approximated by a sum of diagonal and shift invariant matrices. This
/// class represents the generic case, where no approximation to the normal
/// matrix is done. Implementation of this class is largely taken from the
/// old NormalEquation (revisions up to 4637) written by Tim Cornwell.
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
#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <askap/AskapError.h>
#include <utils/DeepCopyUtils.h>

#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

// std includes
#include <utility>
#include <set>
#include <stdexcept>

// casa includes
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>



using namespace askap;
using namespace askap::scimath;
using namespace LOFAR;

using casa::product;
using casa::transpose;

/// @brief a default constructor
/// @details It creates an empty normal equations class
GenericNormalEquations::GenericNormalEquations() {}
  
/// @brief constructor from a design matrix
/// @details This version of the constructor is equivalent to an
/// empty constructor plus a call to add method with the given
/// design matrix
/// @param[in] dm Design matrix to use
GenericNormalEquations::GenericNormalEquations(const DesignMatrix& dm)
{
  add(dm);
}

/// @brief copy constructor
/// @details It is required because this class has non-trivial types (std containers
/// of casa containers)
/// @param[in] src other class
GenericNormalEquations::GenericNormalEquations(const GenericNormalEquations &src) :
        INormalEquations(src)
{
  deepCopyOfSTDMap(src.itsDataVector, itsDataVector);  
  for (std::map<string, MapOfMatrices>::const_iterator ci = src.itsNormalMatrix.begin();
       ci!=src.itsNormalMatrix.end(); ++ci) {
       deepCopyOfSTDMap(ci->second, itsNormalMatrix[ci->first]);
  }
}

/// @brief assignment operator
/// @details It is required because this class has non-trivial types (std containers
/// of casa containers)
/// @param[in] src other class
/// @return reference to this object
GenericNormalEquations& GenericNormalEquations::operator=(const GenericNormalEquations &src)
{
  if (&src != this) {
      itsDataVector.clear();
      deepCopyOfSTDMap(src.itsDataVector, itsDataVector);  
      itsNormalMatrix.clear();
      for (std::map<string, MapOfMatrices>::const_iterator ci = src.itsNormalMatrix.begin();
           ci!=src.itsNormalMatrix.end(); ++ci) {           
           deepCopyOfSTDMap(ci->second, itsNormalMatrix[ci->first]);
      }      
  }
  return *this;
}

      
/// @brief reset the normal equation object
/// @details After a call to this method the object has the same pristine
/// state as immediately after creation with the default constructor
void GenericNormalEquations::reset()
{
  itsDataVector.clear();
  itsNormalMatrix.clear();
}
          
/// @brief Clone this into a shared pointer
/// @details "Virtual constructor" - creates a copy of this object. Derived
/// classes must override this method to instantiate the object of a proper 
/// type.
/// @return shared pointer on INormalEquation class
GenericNormalEquations::ShPtr GenericNormalEquations::clone() const
{
  return ShPtr(new GenericNormalEquations(*this));
}

/// @brief Merge these normal equations with another
/// @details Combining two normal equations depends on the actual class type
/// (different work is required for a full matrix and for an approximation).
/// This method must be overriden in the derived classes for correct 
/// implementation. 
/// This means that we just add
/// @param[in] src an object to get the normal equations from
void GenericNormalEquations::merge(const INormalEquations& src) 
{
   try {
      const GenericNormalEquations &gne = 
                dynamic_cast<const GenericNormalEquations&>(src);
      
      // loop over all parameters, add them one by one.
      // We could have passed iterator directly to mergeParameter and it
      // would work faster (no extra search accross the map). But current
      // code is more readable.
      for (MapOfVectors::const_iterator ci = gne.itsDataVector.begin(); 
           ci != gne.itsDataVector.end(); ++ci) {
           mergeParameter(ci->first, gne);
      }          
   }
   catch (const AskapError &) {
      throw;
   }
   catch (const std::bad_cast &) {
      throw AskapError("Attempt to use GenericNormalEquation::merge with "
                        "incompatible type of the normal equation class");
   }
}

/// @brief Add one parameter from another normal equations class
/// @details This helper method is used in merging two normal equations.
/// It processes just one parameter.
/// @note This helper method works with instances of this class only (as
/// only then it knows how the actual normal matrix is handled). One could
/// have a general code which would work for every possible normal equation,
/// but in some cases it would be very inefficient. Therefore, the decision
/// has been made to throw an exception if incompatible operation is requested
/// and add the code to handle this situation later, if it appears to be 
/// necessary.
/// @param[in] par name of the parameter to copy
/// @param[in] src an object to get the normal equations from
void GenericNormalEquations::mergeParameter(const std::string &par, 
                              const GenericNormalEquations& src)
{
   
   // srcItRow is an iterator over rows in source matrix;
   // by analogy, destItCol is an iterator over columns in the destination matrix
   const std::map<std::string, MapOfMatrices>::const_iterator srcItRow = 
                         src.itsNormalMatrix.find(par);
   ASKAPDEBUGASSERT(srcItRow != src.itsNormalMatrix.end());
   
   const MapOfVectors::const_iterator srcItData = src.itsDataVector.find(par);
   ASKAPDEBUGASSERT(srcItData != src.itsDataVector.end());
   
   addParameter(par, srcItRow->second, srcItData->second);                        
}

/// @brief Add/update one parameter using given matrix and data vector
/// @details This helper method is the main workhorse use in merging two
/// normal equations, adding an independent parameter or a design matrix.
/// The normal matrix to be integrated with this class is given in the form
/// of map of matrices (effectively a sparse matrix). Each element of the map
/// corresponds to a cross- or parallel term in the normal equations. Data
/// vector is given simply as a casa::Vector, rather than the map of vectors,
/// because only one parameter is concerned here. If a parameter with the given
/// name doesn't exist, the method adds it to both normal matrix and data vector,
/// populating correctly all required cross-terms with 0-matrices of an 
/// appropriate shape.
/// @param[in] par name of the parameter to work with
/// @param[in] inNM input normal matrix
/// @param[in] inDV input data vector 
void GenericNormalEquations::addParameter(const std::string &par, 
           const MapOfMatrices &inNM, const casa::Vector<double>& inDV)
{
  // nmRowIt is an iterator over rows (the outer map) of the normal matrix
  // stored in this class 
  std::map<std::string, MapOfMatrices>::iterator nmRowIt = 
                         itsNormalMatrix.find(par);
  if (nmRowIt != itsNormalMatrix.end()) {
      // this parameter is already present in the normal matrix held by this class
      ASKAPDEBUGASSERT(nmRowIt->second.find(par) != nmRowIt->second.end());
       
      // first, process normal matrix 
      for (MapOfMatrices::iterator nmColIt = nmRowIt->second.begin();
                          nmColIt != nmRowIt->second.end(); ++nmColIt) {
            
           // search for an appropriate parameter in the input matrix 
           MapOfMatrices::const_iterator inNMIt = inNM.find(nmColIt->first);           
           // work with cross-terms only if the input matrix have them
           if (inNMIt != inNM.end()) {
               ASKAPCHECK(inNMIt->second.shape() == nmColIt->second.shape(),
                        "shape mismatch for normal matrix, parameters ("<<
                        nmRowIt->first<<" , "<<nmColIt->first<<")");
                nmColIt->second += inNMIt->second; // add up a matrix         
           }            
      }
      
      // now process the data vector
      MapOfVectors::const_iterator dvIt = itsDataVector.find(par);
      ASKAPDEBUGASSERT(dvIt != itsDataVector.end());
      ASKAPCHECK(inDV.shape() == dvIt->second.shape(),
               "shape mismatch for data vector, parameter: "<<dvIt->first);
      // we have to instantiate explicitly a casa::Vector object because
      // otherwise, for some reason, the compiler can't figure out the type 
      // properly at the += operator. Exploit reference semantics - no copying!
      casa::Vector<double> destVec = dvIt->second;
      destVec += inDV; // add up a vector  
  } else {
     // this is a brand new parameter
     nmRowIt = itsNormalMatrix.insert(std::make_pair(par,MapOfMatrices())).first;
     // iterator, which points to this parameter in inNM map.
     const casa::uInt newParDimension = parameterDimension(inNM); 
       
     // process normal matrix - add cross terms for all parameters, names are
     // gathered from rows (it uses the fact the normal matrix is always square)
     for (std::map<std::string, MapOfMatrices>::iterator nmOldRowIt = 
          itsNormalMatrix.begin(); nmOldRowIt != itsNormalMatrix.end(); 
          ++nmOldRowIt) {
            
          // search for an appropriate parameter in the source 
          MapOfMatrices::const_iterator inNMIt = inNM.find(nmOldRowIt->first);           
          if (inNMIt != inNM.end()) {
              // insert terms only if the input matrix have them
              nmRowIt->second.insert(*inNMIt); // assign a matrix
              if (par != nmOldRowIt->first) {
                  // fill in a symmetric term
                  nmOldRowIt->second.insert(std::make_pair(par,
                                transpose(inNMIt->second)));
              }         
          } else {
              // insert zero matrix, as the parameter referred by nameIt and
              // the new parameter are independent and, therefore, have zero 
              // cross-terms.
              const casa::uInt thisParDimension = 
                               parameterDimension(nmOldRowIt->second); 
              nmRowIt->second.insert(std::make_pair(nmOldRowIt->first,
                  casa::Matrix<double>(newParDimension, thisParDimension,0.)));
              if (par != nmOldRowIt->first) {
                  // fill in a symmetric term
                  nmOldRowIt->second.insert(std::make_pair(par,
                      casa::Matrix<double>(thisParDimension, newParDimension,0.)));    
              }
          }            
     }
     
       
     // process data vector
     ASKAPDEBUGASSERT(itsDataVector.find(par) == itsDataVector.end());
     itsDataVector.insert(std::make_pair(par, inDV));
  }                       
}           

/// @brief extract dimension of a parameter from the given row
/// @details This helper method analyses the matrices stored in the supplied
/// map (effectively a row of a sparse matrix) and extracts the dimension of
/// the parameter this row corresponds to. If compiled with ASKAP_DEBUG, 
/// this method does an additional consistency check that all elements of
/// the sparse matrix give the same dimension (number of rows is the same for
/// all elements).
/// @param[in] nmRow a row of the sparse normal matrix to work with
/// @return dimension of the corresponding parameter
casa::uInt GenericNormalEquations::parameterDimension(const MapOfMatrices &nmRow)
{
  const MapOfMatrices::const_iterator it = nmRow.begin();
  ASKAPDEBUGASSERT(it != nmRow.end());
  const casa::uInt dim = it->second.nrow();
#ifdef ASKAP_DEBUG
  for (MapOfMatrices::const_iterator cur = it; cur != nmRow.end(); ++cur) {
       ASKAPASSERT(cur->second.nrow() == dim);
  }     
#endif // ASKAP_DEBUG
  return dim;
}  

  
/// @brief Add a design matrix to the normal equations
/// @details This method computes the contribution to the normal matrix 
/// using a given design matrix and adds it.
/// @param[in] dm Design matrix to use
void GenericNormalEquations::add(const DesignMatrix& dm)
{
  std::set<string> names=dm.parameterNames();
  const casa::uInt nDataSet=dm.residual().size();
  if (!nDataSet) {
      return; // nothing to process
  }
  
  // Loop over all parameters defined by the design matrix.
  // It may be better to write an iterator over parameters defined in
  // the design matrix instead of building a set or list. 
  for (std::set<string>::const_iterator iterRow = names.begin(); 
       iterRow != names.end(); ++iterRow) {
       const DMAMatrix &derivMatrices = dm.derivative(*iterRow);
       DMAMatrix::const_iterator derivMatricesIt = derivMatrices.begin();
       ASKAPDEBUGASSERT(derivMatricesIt != derivMatrices.end());
       DMBVector::const_iterator residualIt = dm.residual().begin();
       ASKAPDEBUGASSERT(residualIt != dm.residual().end());
       ASKAPDEBUGASSERT(derivMatricesIt->ncolumn());
       
       casa::Vector<double> dataVector; // data vector buffer for this row 
       
       // it looks unnecessary from the first glance to fill the map
       // of matrices for the whole row. However, the design matrix can
       // be defined for a subset of parameters used by this normal equation
       // class. Therefore, one must resize appropriate elements of 
       // itsNormalMatrix to have there zero matrix of appropriate shape.
       // it requires access to the size of the result anyway, therefore
       // it is not too bad to calculate all elements in the row before
       // merging them with itsNormalMatrix
       MapOfMatrices normalMatrix; // normal matrix buffer for this row
       
       // the first contribution
       dataVector = dvElement(*derivMatricesIt, *residualIt);
       
       for (std::set<string>::const_iterator iterCol = names.begin();
                iterCol != names.end(); ++iterCol) {
                
            normalMatrix.insert(std::make_pair(*iterCol, 
                 nmElement(*derivMatricesIt, extractDerivatives(dm,*iterCol,0))));         
       }
      
       // now add up all other data points
       ++derivMatricesIt;
       for(casa::uInt dataPoint = 1; derivMatricesIt != derivMatrices.end() ;
                               ++dataPoint,++derivMatricesIt) {
           dataVector += dvElement(*derivMatricesIt, *residualIt);
          
           for (MapOfMatrices::iterator iterCol = normalMatrix.begin();
                               iterCol != normalMatrix.end(); ++iterCol) {
                    
                iterCol->second += nmElement(*derivMatricesIt, 
                           extractDerivatives(dm,iterCol->first,dataPoint));
           }
       }
      addParameter(*iterRow, normalMatrix, dataVector); 
  }
    
}

/// @brief Extract derivatives from design matrix
/// @details This method extracts an appropriate derivative matrix
/// from the given design matrix. Effectively, it implements
/// dm.derivative(par)[dataPoint] with some additional validity checks
/// @param[in] dm Design matrix to work with
/// @param[in] par parameter name of interest
/// @param[in] dataPoint a sequence number of the data point, for which 
/// the derivatives are returned
/// @return matrix of derivatives
const casa::Matrix<double>& 
     GenericNormalEquations::extractDerivatives(const DesignMatrix &dm,
             const std::string &par, casa::uInt dataPoint)
{
  const DMAMatrix &derivMatrices = dm.derivative(par);
  // there is no benefit here from introducing an iterator as
  // only one specific offset is always taken
  ASKAPDEBUGASSERT(dataPoint < derivMatrices.size());
  return derivMatrices[dataPoint];
}  
  
/// @brief Calculate an element of A^tA
/// @details Each element of a sparse normal matrix is also a matrix
/// in general. However, due to some limitations of CASA operators, a
/// separate treatment is required for degenerate cases. This method
/// calculates an element of the normal matrix (effectively an element of
/// a product of A transposed and A, where A is the whole design matrix)
/// @param[in] matrix1 the first element of a sparse normal matrix
/// @param[in] matrix2 the second element of a sparse normal matrix
/// @return a product of matrix1 transposed to matrix2
casa::Matrix<double> GenericNormalEquations::nmElement(const casa::Matrix<double> &matrix1,
               const casa::Matrix<double> &matrix2)
{
  ASKAPDEBUGASSERT(matrix1.ncolumn() && matrix2.ncolumn());
  ASKAPDEBUGASSERT(matrix1.nrow() == matrix2.nrow());
  if (matrix1.ncolumn() == 1 && matrix2.ncolumn() == 1) {
      const casa::Vector<double> &m1ColVec = matrix1.column(0);
      const casa::Vector<double> &m2ColVec = matrix2.column(0);
      return casa::Matrix<double>(1,1,sum(m1ColVec*m2ColVec));
  }
  
  // at least one of the matrices is non-degenerate
  return product(transpose(matrix1),matrix2);
}               
  
/// @brief Calculate an element of A^tB
/// @details Each element of a sparse normal matrix is also a matrix
/// in general. However, due to some limitations of CASA operators, a
/// separate treatment is required for degenerate cases. This method
/// calculates an element of the right-hand side of the normal equation
/// (effectively an element of a product of A transposed and the data
/// vector, where A is the whole design matrix)
/// @param[in] dm an element of the design matrix
/// @param[in] dv an element of the data vector
casa::Vector<double> GenericNormalEquations::dvElement(const casa::Matrix<double> &dm,
              const casa::Vector<double> &dv)
{
  ASKAPDEBUGASSERT(dm.ncolumn() && dv.nelements());
  ASKAPDEBUGASSERT(dm.nrow() == dv.nelements());
  if (dm.ncolumn() == 1) {
      const casa::Vector<double> &dmColVec = dm.column(0);
      return casa::Vector<double>(1, sum(dmColVec*dv));
  }
  // dm is non-degenerate
  return product(transpose(dm), dv);
}               
  
/// @brief add normal matrix for a given parameter
/// @details This means that the cross terms between parameters 
/// are excluded. However the terms inside a parameter are retained.
/// @param[in] name Name of the parameter
/// @param[in] normalmatrix Normal Matrix for this parameter
/// @param[in] datavector Data vector for this parameter
void GenericNormalEquations::add(const string& name, 
                               const casa::Matrix<double>& normalmatrix,
                               const casa::Vector<double>& datavector)
{
  MapOfMatrices tempSparseMatrix;
  tempSparseMatrix[name] = normalmatrix;
  
  addParameter(name, tempSparseMatrix, datavector);
}  
  
/// @brief normal equations for given parameters
/// @details In the current framework, parameters are essentially 
/// vectors, not scalars. Each element of such vector is treated
/// independently (but only vector as a whole can be fixed). As a 
/// result the element of the normal matrix is another matrix for
/// all non-scalar parameters. For scalar parameters each such
/// matrix has a shape of [1,1].
/// @param[in] par1 the name of the first parameter
/// @param[in] par2 the name of the second parameter
const casa::Matrix<double>& GenericNormalEquations::normalMatrix(const std::string &par1, 
                          const std::string &par2) const
{
  std::map<string,std::map<string, casa::Matrix<double> > >::const_iterator cIt1 = 
                                   itsNormalMatrix.find(par1);
  ASKAPCHECK(cIt1 != itsNormalMatrix.end(), "Missing first parameter "<<par1<<" is requested from the normal matrix");
  std::map<string, casa::Matrix<double> >::const_iterator cIt2 = 
                                   cIt1->second.find(par2);
  ASKAPCHECK(cIt2 != cIt1->second.end(), "Missing second parameter "<<par2<<" is requested from the normal matrix");
  return cIt2->second;                             
}

/// @brief data vector for a given parameter
/// @details In the current framework, parameters are essentially 
/// vectors, not scalars. Each element of such vector is treated
/// independently (but only vector as a whole can be fixed). As a 
/// result any element of the normal matrix as well as an element of the
/// data vector are, in general, matrices, not scalar. For the scalar 
/// parameter each element of data vector is a vector of unit length.
/// @param[in] par the name of the parameter of interest
const casa::Vector<double>& GenericNormalEquations::dataVector(const std::string &par) const
{
  std::map<string, casa::Vector<double> >::const_iterator cIt = 
                                           itsDataVector.find(par);
  ASKAPCHECK(cIt != itsDataVector.end(),"Parameter "<<par<<" is not found in the normal equations");
  return cIt->second;                                  
}                          
  
/// @brief write the object to a blob stream
/// @param[in] os the output stream
void GenericNormalEquations::writeToBlob(LOFAR::BlobOStream& os) const
{ 
  // increment version number on the next line and in the next method
  // if any new data members are added  
  os.putStart("GenericNormalEquations",1);
  os<<itsNormalMatrix<<itsDataVector;
  os.putEnd();
}

/// @brief read the object from a blob stream
/// @param[in] is the input stream
/// @note Not sure whether the parameter should be made const or not 
void GenericNormalEquations::readFromBlob(LOFAR::BlobIStream& is)
{ 
  const int version = is.getStart("GenericNormalEquations");
  ASKAPCHECK(version == 1, 
              "Attempting to read from a blob stream an object of the wrong "
              "version: expect version 1, found version "<<version);
  is>>itsNormalMatrix>>itsDataVector;
  is.getEnd();
}

/// @brief obtain all parameters dealt with by these normal equations
/// @details Normal equations provide constraints for a number of 
/// parameters (i.e. unknowns of these equations). This method returns
/// a vector with the string names of all parameters mentioned in the
/// normal equations represented by the given object.
/// @return a vector listing the names of all parameters (unknowns of these equations)
/// @note if ASKAP_DEBUG is set some extra checks on consistency of these 
/// equations are done
std::vector<std::string> GenericNormalEquations::unknowns() const
{
  std::vector<std::string> result;
  result.reserve(itsNormalMatrix.size());
  for (std::map<std::string, MapOfMatrices>::const_iterator ci=itsNormalMatrix.begin();
       ci!=itsNormalMatrix.end(); ++ci) {
       result.push_back(ci->first);
// extra consistency checks in the debug mode
#ifdef ASKAP_DEBUG
       ASKAPCHECK(itsDataVector.find(ci->first) != itsDataVector.end(), 
                  "The parameter "<<ci->first<<" is present in the mornal matrix but missing in the data vector");
#endif // #ifdef ASKAP_DEBUG
  }
  return result;
} // unknowns method

 
