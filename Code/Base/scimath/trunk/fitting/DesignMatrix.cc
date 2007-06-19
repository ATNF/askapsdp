#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using std::string;
using std::vector;
using std::map;

namespace conrad
{
  namespace scimath
  {

    DesignMatrix::DesignMatrix(const Params& ip) : itsParams(ip.clone())
    {
      itsAMatrix.clear();
      itsBVector.clear();
      itsWeight.clear();
    }

    DesignMatrix::DesignMatrix(const DesignMatrix& other)
    {
      operator=(other);
    }

    DesignMatrix& DesignMatrix::operator=(const DesignMatrix& other)
    {
      if(this!=&other)
      {
        itsAMatrix.clear();
        itsBVector.clear();
        itsWeight.clear();
        itsParams=other.itsParams;
        std::map<string, DMAMatrix>::iterator AIt, OtherAIt;
        for (AIt=other.itsAMatrix.begin();AIt!=other.itsAMatrix.end();AIt++)
        {
          DMAMatrix::iterator AMIt;
          for (uint i=0;i<AIt->first.size();i++)
          {
            itsAMatrix[AIt->first][i]=other.itsAMatrix[AIt->first][i].copy();
          }
        }

        for (uint i=0;i<other.itsBVector.size();i++)
        {
          itsBVector[i]=other.itsBVector[i].copy();
        }
        for (uint i=0;i<other.itsWeight.size();i++)
        {
          itsWeight[i]=other.itsWeight[i].copy();
        }
      }
    }

    DesignMatrix::~DesignMatrix()
    {
      reset();
    }

    void DesignMatrix::merge(const DesignMatrix& other)
    {
      itsParams->merge(*other.itsParams);

      vector<string> names(other.names());
      vector<string>::const_iterator nameIt;
      for (nameIt=names.begin();nameIt!=names.end();nameIt++)
      {
        DMAMatrix AM(other.derivative(*nameIt));
        DMAMatrix::iterator AMIt;
        for (AMIt=AM.begin();AMIt!=AM.end();AMIt++)
        {
          addDerivative(*nameIt, AMIt->copy());
        }
      }

      for (uint i=0;i<other.itsBVector.size();i++)
      {
        itsBVector.push_back(other.itsBVector[i].copy());
      }
      for (uint i=0;i<other.itsWeight.size();i++)
      {
        itsWeight.push_back(other.itsWeight[i].copy());
      }

    }

    void DesignMatrix::addDerivative(const string& name, const casa::Matrix<casa::Double>& deriv)
    {
      if(!itsParams->has(name))
      {
        throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
      }
      itsAMatrix[name].push_back(deriv.copy());
    }

    void DesignMatrix::addResidual(const casa::Vector<casa::Double>& residual, const casa::Vector<double>& weight)
    {
      itsBVector.push_back(residual.copy());
      itsWeight.push_back(weight.copy());
    }

    vector<string> DesignMatrix::names() const
    {
      return itsParams->freeNames();
    }

    const Params& DesignMatrix::parameters() const
    {
      return *itsParams;
    }

    Params& DesignMatrix::parameters()
    {
      return *itsParams;
    }

    DMAMatrix DesignMatrix::derivative(const string& name) const
    {
      if(!itsParams->has(name))
      {
        throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
      }
      if(itsAMatrix.count(name)==0)
      {
        throw(std::invalid_argument("Parameter "+name+" does not exist in the assigned values"));
      }
      return itsAMatrix[name];
    }

    DMBVector DesignMatrix::residual() const
    {
      return itsBVector;
    }

    DMWeight DesignMatrix::weight() const
    {
      return itsWeight;
    }

    void DesignMatrix::reset()
    {
      itsAMatrix.clear();
      itsBVector.clear();
      itsWeight.clear();
    }

    double DesignMatrix::fit() const
    {
      double sumwt=0.0;
      double sum=0.0;
      DMBVector::iterator bIt;
      DMWeight::iterator wIt;
      for (bIt=itsBVector.begin(),wIt=itsWeight.begin();
        (bIt!=itsBVector.end())&&(wIt!=itsWeight.end());bIt++, wIt++)
      {
        sumwt+=casa::sum(*wIt);
        sum+=casa::sum((*wIt)*((*bIt)*(*bIt)));
      }
      if(sumwt>0.0)
      {
        return sqrt(sum/sumwt);
      }
      else
      {
        throw(std::invalid_argument("Sum of weights is zero"));
      }
    }

    uint DesignMatrix::nData() const
    {
      uint nData=0;
      DMBVector::iterator bIt;
      DMWeight::iterator wIt;
      for (bIt=itsBVector.begin();bIt!=itsBVector.end();bIt++)
      {
        nData+=bIt->size();
      }
      return nData;
    }

    uint DesignMatrix::nParameters() const
    {
      uint nParameters=0;
      std::map<string, DMAMatrix>::iterator AIt;
      for (AIt=itsAMatrix.begin();AIt!=itsAMatrix.end();AIt++)
      {
        DMAMatrix::iterator it;
        for (it=AIt->second.begin();it!=AIt->second.end();it++)
        {
          nParameters+=it->ncolumn();
        }
      }
      return nParameters;
    }

    DesignMatrix::ShPtr DesignMatrix::clone()
    {
      return DesignMatrix::ShPtr(new DesignMatrix(*this));
    }

  }
}
