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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// own includes
#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <conrad/ConradError.h>

#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

// std includes
#include <utility>
#include <set>
#include <stdexcept>

// casa includes
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>



using namespace conrad;
using namespace conrad::scimath;
using namespace LOFAR;


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
      
/// @brief reset the normal equation object
/// @detail After a call to this method the object has the same pristine
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
   catch (const ConradError &) {
      throw;
   }
   catch (const std::bad_cast &) {
      throw ConradError("Attempt to use GenericNormalEquation::merge with "
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
   CONRADDEBUGASSERT(srcItRow != src.itsNormalMatrix.end());
   
   const MapOfVectors::const_iterator srcItData = src.itsDataVector.find(par);
   CONRADDEBUGASSERT(srcItData != src.itsDataVector.end());
   
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
      CONRADDEBUGASSERT(nmRowIt->second.find(par) != nmRowIt->second.end());
       
      // first, process normal matrix 
      for (MapOfMatrices::iterator nmColIt = nmRowIt->second.begin();
                          nmColIt != nmRowIt->second.end(); ++nmColIt) {
            
           // search for an appropriate parameter in the input matrix 
           MapOfMatrices::const_iterator inNMIt = inNM.find(nmColIt->first);           
           // work with cross-terms only if the input matrix have them
           if (inNMIt != inNM.end()) {
               CONRADCHECK(inNMIt->second.shape() == nmColIt->second.shape(),
                        "shape mismatch for normal matrix, parameters ("<<
                        nmRowIt->first<<" , "<<nmColIt->first<<")");
                nmColIt->second += inNMIt->second; // add up a matrix         
           }            
      }
      
      // now process the data vector
      MapOfVectors::const_iterator dvIt = itsDataVector.find(par);
      CONRADDEBUGASSERT(dvIt != itsDataVector.end());
      CONRADCHECK(inDV.shape() == dvIt->second.shape(),
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
     for (std::map<std::string, MapOfMatrices>::const_iterator nameIt = 
          itsNormalMatrix.begin(); nameIt != itsNormalMatrix.end(); ++nameIt) {
            
          // search for an appropriate parameter in the source 
          MapOfMatrices::const_iterator inNMIt = inNM.find(nameIt->first);           
          if (inNMIt != inNM.end()) {
              // insert terms only if the input matrix have them
              nmRowIt->second.insert(*inNMIt); // assign a matrix         
          } else {
              // insert zero matrix, as the parameter referred by nameIt and
              // the new parameter are independent and, therefore, have zero 
              // cross-terms.
              const casa::uInt thisParDimension = 
                               parameterDimension(nameIt->second); 
              nmRowIt->second.insert(std::make_pair(nameIt->first,
                  casa::Matrix<double>(newParDimension, thisParDimension,0.)));
          }            
     }
       
     // process data vector
     CONRADDEBUGASSERT(itsDataVector.find(par) == itsDataVector.end());
     itsDataVector.insert(std::make_pair(par, inDV));
  }                       
}           

/// @brief extract dimension of a parameter from the given row
/// @details This helper method analyses the matrices stored in the supplied
/// map (effectively a row of a sparse matrix) and extracts the dimension of
/// the parameter this row corresponds to. If compiled with CONRAD_DEBUG, 
/// this method does an additional consistency check that all elements of
/// the sparse matrix give the same dimension (number of rows is the same for
/// all elements).
/// @param[in] nmRow a row of the sparse normal matrix to work with
/// @return dimension of the corresponding parameter
casa::uInt GenericNormalEquations::parameterDimension(const MapOfMatrices &nmRow)
{
  const MapOfMatrices::const_iterator it = nmRow.begin();
  CONRADDEBUGASSERT(it != nmRow.end());
  const casa::uInt dim = it->second.nrow();
#ifdef CONRAD_DEBUG
  for (MapOfMatrices::const_iterator cur = it; cur != nmRow.end(); ++cur) {
       CONRADASSERT(cur->second.nrow() == dim);
  }     
#endif // CONRAD_DEBUG
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
  
  
  std::set<string>::iterator iterRow;
  std::set<string>::iterator iterCol;
      

// This looks hairy but it's all just linear algebra!
      for (iterRow=names.begin();iterRow!=names.end();iterRow++)
      {
        bool first=(itsDataVector[*iterRow].size()==0);
        for (uint iDataSet=0;iDataSet<nDataSet;iDataSet++)
        {
// Need to special case for CASA product limitation
          if(dm.derivative(*iterRow)[iDataSet].ncolumn()==1)
          {
            const casa::Vector<casa::Double>& aV(dm.derivative(*iterRow)[iDataSet].column(0));
            if(first)
            {
              // we need to initialize the Vector by either resizing or
              // assigning a vector of size 1 as below
              itsDataVector[*iterRow] = casa::Vector<double>(1,
                            sum(((aV)*(dm.residual()[iDataSet]))));
              first=false;
            }
            else
            {
              // operator+= will add constant to every element of the vector,
              // we have just one element here
              itsDataVector[*iterRow]+=sum(((aV)*(dm.residual()[iDataSet])));
            }
          }
          else
          {
            if(first)
            {
              itsDataVector[*iterRow]=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
              first=false;
            }
            else
            {
              itsDataVector[*iterRow]+=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
            }
          }
        }
      }
// Outside loops are over parameter names
      for (iterCol=names.begin();iterCol!=names.end();iterCol++)
      {
        const uint nACol=dm.derivative(*iterCol).size();
// Inside loops are over lists of derivatives
        for (uint iACol=0;(iACol<nACol);iACol++)
        {
          for (iterRow=names.begin();iterRow!=names.end();iterRow++)
          {
            bool first=(itsNormalMatrix[*iterRow][*iterCol].nrow()==0);
            const uint nARow=dm.derivative(*iterRow).size();
            for (uint iARow=0;(iARow<nARow);iARow++)
            {
              if((dm.derivative(*iterRow)[iARow].ncolumn()==1)&&(dm.derivative(*iterCol)[iACol].ncolumn()==1))
              {
                const casa::Vector<casa::Double>& aRowV(dm.derivative(*iterRow)[iARow].column(0));
                const casa::Vector<casa::Double>& aColV(dm.derivative(*iterCol)[iACol].column(0));
                if(first)
                {
                  itsNormalMatrix[*iterRow][*iterCol].resize(1,1);
                  itsNormalMatrix[*iterRow][*iterCol].set(sum(((aRowV)*(aColV))));
                  first=false;
                }
                else
                {
                  itsNormalMatrix[*iterRow][*iterCol]+=sum(((aRowV)*(aColV)));
                }
              }
              else
              {
                if(first)
                {
                  itsNormalMatrix[*iterRow][*iterCol]=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                  first=false;
                }
                else
                {
                  itsNormalMatrix[*iterRow][*iterCol]+=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                }
              }
            }
          }
        }
      }
  
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
  CONRADASSERT(cIt1 != itsNormalMatrix.end());
  std::map<string, casa::Matrix<double> >::const_iterator cIt2 = 
                                   cIt1->second.find(par2);
  CONRADASSERT(cIt2 != cIt1->second.end());
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
  CONRADASSERT(cIt != itsDataVector.end());
  return cIt->second;                                  
}                          
  
/// @brief write the object to a blob stream
/// @param[in] os the output stream
LOFAR::BlobOStream& GenericNormalEquations::operator<<(LOFAR::BlobOStream& os) const
{ 
  const std::string type("unknown");
  os<<type<<itsNormalMatrix<<itsDataVector;
  return os;
  
}

/// @brief read the object from a blob stream
/// @param[in] is the input stream
/// @note Not sure whether the parameter should be made const or not 
LOFAR::BlobIStream& GenericNormalEquations::operator>>(LOFAR::BlobIStream& is)
{ 
  std::string type("GenericNormalEquations");
  is>>type;
  CONRADCHECK(type == "GenericNormalEquations", 
              "Attempting to read from a blob stream an object of the wrong "
              "type: expect GenericNormalEquations, found "<<type);
  is>>itsNormalMatrix>>itsDataVector;
  return is;
               
}

  