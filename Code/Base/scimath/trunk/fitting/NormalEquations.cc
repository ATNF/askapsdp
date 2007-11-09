/// @file
///
/// Holds the normal equations for a set of linear equations
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

#include <fitting/DesignMatrix.h>
#include <fitting/NormalEquations.h>
#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

#include <conrad/ConradError.h>

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

namespace conrad
{
  namespace scimath
  {

    NormalEquations::NormalEquations() {};
    
    NormalEquations::NormalEquations(const Params& ip) : itsParams(ip.clone())
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
        for (iterCol=names.begin();iterCol!=names.end();++iterCol)
        {
          itsNormalMatrix[*iterRow][*iterCol]=casa::Matrix<double>(0,0);
        }
      }
    }

    NormalEquations::NormalEquations(const NormalEquations& other)
    {
      operator=(other);
    }

    NormalEquations& NormalEquations::operator=(const NormalEquations& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
        itsNormalMatrix=other.itsNormalMatrix;
        itsNormalMatrixSlice=other.itsNormalMatrixSlice;
        itsNormalMatrixDiagonal=other.itsNormalMatrixDiagonal;
        itsDataVector=other.itsDataVector;
        itsShape=other.itsShape;
        itsReference=other.itsReference;
      }
      return *this;
    }

    NormalEquations::NormalEquations(const Params &ip, const DesignMatrix& dm)
    {
      //itsParams=dm.parameters().clone();
      itsParams=ip.clone();
      std::set<string> names=dm.parameterNames();
      std::set<string>::iterator iterRow;
      std::set<string>::iterator iterCol;
      const uint nDataSet=dm.residual().size();

// This looks hairy but it's all just linear algebra!
      for (iterRow=names.begin();iterRow!=names.end();iterRow++)
      {
        bool first=true;
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
            bool first=true;
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

    void NormalEquations::add(const DesignMatrix& dm)
    {
      //itsParams->merge(dm.parameters());
      std::set<string> names=dm.parameterNames();
      std::set<string>::iterator iterRow;
      std::set<string>::iterator iterCol;
      const uint nDataSet=dm.residual().size();

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
      /*
      for (std::map<std::string, casa::Vector<double> >::const_iterator tci=
           itsDataVector.begin();tci!=itsDataVector.end(); ++tci)
              std::cout<<tci->first<<" "<<tci->second.nelements()<<std::endl;
      */
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

    NormalEquations::~NormalEquations()
    {
      reset();
    }
    void NormalEquations::merge(const NormalEquations& other)
    {
      itsParams->merge(*other.itsParams);
      vector<string> names=itsParams->freeNames();
      vector<string>::iterator iterRow;
      vector<string>::iterator iterCol;

      for (iterCol=names.begin();iterCol!=names.end();iterCol++)
      {
        if(itsDataVector[*iterCol].size()!=other.itsDataVector.find(*iterCol)->second.size())
        {
          itsDataVector[*iterCol]=other.itsDataVector.find(*iterCol)->second;
        }
        else
        {
          itsDataVector[*iterCol]+=other.itsDataVector.find(*iterCol)->second;
        }
        itsShape[*iterCol].resize(0);
        itsShape[*iterCol]=other.itsShape.find(*iterCol)->second;
        if(itsNormalMatrixSlice[*iterCol].shape()!=other.itsNormalMatrixSlice.find(*iterCol)->second.shape())
        {
          itsNormalMatrixSlice[*iterCol]=other.itsNormalMatrixSlice.find(*iterCol)->second;
        }
        else
        {
          itsNormalMatrixSlice[*iterCol]+=other.itsNormalMatrixSlice.find(*iterCol)->second;
        }
        if(itsNormalMatrixDiagonal[*iterCol].shape()!=other.itsNormalMatrixDiagonal.find(*iterCol)->second.shape())
        {
          itsNormalMatrixDiagonal[*iterCol]=other.itsNormalMatrixDiagonal.find(*iterCol)->second;
        }
        else
        {
          itsNormalMatrixDiagonal[*iterCol]+=other.itsNormalMatrixDiagonal.find(*iterCol)->second;
        }
        for (iterRow=names.begin();iterRow!=names.end();iterRow++)
        {
          if(itsNormalMatrix[*iterCol][*iterRow].shape()!=other.itsNormalMatrix.find(*iterCol)->second.find(*iterRow)->second.shape())
          {
            itsNormalMatrix[*iterCol][*iterRow]=other.itsNormalMatrix.find(*iterCol)->second.find(*iterRow)->second;
          }
          else
          {
            itsNormalMatrix[*iterCol][*iterRow]+=other.itsNormalMatrix.find(*iterCol)->second.find(*iterRow)->second;
          }
        }
      }
    }

    const std::map<string, casa::Vector<double> >& NormalEquations::normalMatrixDiagonal() const
    {
      return itsNormalMatrixDiagonal;
    }

    const std::map<string, casa::Vector<double> >& NormalEquations::normalMatrixSlice() const
    {
      return itsNormalMatrixSlice;
    }
    
/// @brief normal equations for given parameters
/// @details This method is added instead of the one returning the
/// whole map of maps as a step towards hiding the actual matrix 
/// implementation
/// @param[in] par1 the name of the first parameter
/// @param[in] par2 the name of the second parameter
const casa::Matrix<double>& NormalEquations::normalMatrix(const std::string &par1, 
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

/// Return data vector
    const std::map<string, casa::Vector<double> >& NormalEquations::dataVector() const
    {
      return itsDataVector;
    }

/// Return shape
    const std::map<string, casa::IPosition >& NormalEquations::shape() const
    {
      return itsShape;
    }

/// Return reference
    const std::map<string, casa::IPosition >& NormalEquations::reference() const
    {
      return itsReference;
    }

    void NormalEquations::reset()
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
        for (iterCol=itsNormalMatrix[iterRow->first].begin();iterCol!=itsNormalMatrix[iterRow->first].end();iterCol++)
        {
          itsNormalMatrix[iterRow->first][iterCol->first].resize();
          itsNormalMatrix[iterRow->first][iterCol->first]=casa::Matrix<double>(0,0);
        }
      }
    }

    void NormalEquations::add(const string& name, const casa::Matrix<double>& normalmatrix,
      const casa::Vector<double>& datavector, const casa::IPosition& shape)
    {

      if(datavector.size()!=itsDataVector[name].size())
      {
        itsDataVector[name]=datavector;
      }
      else
      {
        itsDataVector[name]+=datavector;
      }
      if(normalmatrix.shape()!=itsNormalMatrix[name][name].shape())
      {
        itsNormalMatrix[name][name]=normalmatrix;
      }
      else
      {
        itsNormalMatrix[name][name]+=normalmatrix;
      }
      itsShape[name].resize(0);
      itsShape[name]=shape;
    }

    void NormalEquations::add(const string& name, const casa::Matrix<double>& normalmatrix,
      const casa::Vector<double>& datavector)
    {
      casa::IPosition shape(1, datavector.nelements());
      add(name, normalmatrix, datavector, shape);
    }

    void NormalEquations::addSlice(const string& name,
      const casa::Vector<double>& normalmatrixslice,
      const casa::Vector<double>& normalmatrixdiagonal,
      const casa::Vector<double>& datavector,
      const casa::IPosition& shape,
      const casa::IPosition& reference)
    {

      if(datavector.size()!=itsDataVector[name].size())
      {
        itsDataVector[name]=datavector;
      }
      else
      {
        itsDataVector[name]+=datavector;
      }
      if(normalmatrixslice.shape()!=itsNormalMatrixSlice[name].shape())
      {
        itsNormalMatrixSlice[name]=normalmatrixslice;
      }
      else
      {
        itsNormalMatrixSlice[name]+=normalmatrixslice;
      }
      if(normalmatrixdiagonal.shape()!=itsNormalMatrixDiagonal[name].shape())
      {
        itsNormalMatrixDiagonal[name]=normalmatrixdiagonal;
      }
      else
      {
        itsNormalMatrixDiagonal[name]+=normalmatrixdiagonal;
      }
      itsShape[name].resize(0);
      itsShape[name]=shape;
      itsReference[name].resize(0);
      itsReference[name]=reference;
    }

    void NormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrixdiagonal,
      const casa::Vector<double>& datavector, const casa::IPosition& shape)
    {

      if(datavector.size()!=itsDataVector[name].size())
      {
        itsDataVector[name]=datavector;
      }
      else
      {
        itsDataVector[name]+=datavector;
      }
      if(normalmatrixdiagonal.shape()!=itsNormalMatrixDiagonal[name].shape())
      {
        itsNormalMatrixDiagonal[name]=normalmatrixdiagonal;
      }
      else
      {
        itsNormalMatrixDiagonal[name]+=normalmatrixdiagonal;
      }
      itsShape[name].resize(0);
      itsShape[name]=shape;
    }

    void NormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrix,
      const casa::Vector<double>& datavector)
    {
      casa::IPosition shape(1, datavector.nelements());
      addDiagonal(name, normalmatrix, datavector, shape);
    }

    const Params& NormalEquations::parameters() const
    {
      return *itsParams;
    }

    NormalEquations::ShPtr NormalEquations::clone() const
    {
      return NormalEquations::ShPtr(new NormalEquations(*this));
    }

// These are the items that we need to write to and read from a blob stream
// Params::ShPtr itsParams;
// std::map<string, std::map<string, casa::Matrix<double> > > itsNormalMatrix
// std::map<string, casa::Vector<double> > itsNormalMatrixSlice
// std::map<string, casa::Vector<double> > itsNormalMatrixDiagonal
// std::map<string, casa::IPosition> itsShape
// std::map<string, casa::IPosition> itsReference
// std::map<string, casa::Vector<double> > itsDataVector

    
    LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const NormalEquations& ne) 
    {
      os << *(ne.itsParams) << ne.itsNormalMatrix << ne.itsNormalMatrixSlice 
        << ne.itsNormalMatrixDiagonal << ne.itsShape << ne.itsReference << ne.itsDataVector; 
//        << ne.itsNormalMatrixDiagonal << ne.itsDataVector;
     return os;
    }

    LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, NormalEquations& ne)
    {
      ne.itsParams = Params::ShPtr(new Params());
      is >> *(ne.itsParams) >> ne.itsNormalMatrix >> ne.itsNormalMatrixSlice 
         >> ne.itsNormalMatrixDiagonal >> ne.itsShape >> ne.itsReference 
         >> ne.itsDataVector;
//        >> ne.itsNormalMatrixDiagonal >> ne.itsDataVector;
      return is;
    }
  }
}
