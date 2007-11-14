/// @file
///
/// Class to handle design equations for the fitting classes
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

#include <conrad/ConradError.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using std::string;
using std::vector;
using std::map;
using namespace conrad;

namespace conrad
{
  namespace scimath
  {

    DesignMatrix::DesignMatrix() : itsParameterNamesInvalid(true) {}
    
/*
    DesignMatrix::DesignMatrix(const Params& ip) : itsParams(ip.clone()),
                                 itsParameterNamesInvalid(true)
    {
      itsAMatrix.clear();
      itsBVector.clear();
      itsWeight.clear();
    }
    */

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
        //itsParams=other.itsParams;
        itsParameterNamesInvalid = true;
        // We need to do a deep copy to ensure that future changes don't propagate here
        for (std::map<string, DMAMatrix>::const_iterator AIt=other.itsAMatrix.begin();AIt!=other.itsAMatrix.end();AIt++)
        {
          const DMAMatrix& otherDMAMatrix(AIt->second);
          DMAMatrix& thisDMAMatrix(itsAMatrix[AIt->first]);
          for (DMAMatrix::const_iterator it=otherDMAMatrix.begin();it!=otherDMAMatrix.end();it++)
          {
            thisDMAMatrix.push_back(it->copy());
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
      return *this;
    }

    DesignMatrix::~DesignMatrix()
    {
      reset();
    }

    void DesignMatrix::merge(const DesignMatrix& other)
    {
      //itsParams->merge(*other.itsParams);
      
      for (std::map<std::string,DMAMatrix>::const_iterator cit = 
           other.itsAMatrix.begin();cit!=other.itsAMatrix.end();++cit) {
           const DMAMatrix& AM = cit->second;
           for (DMAMatrix::const_iterator AMIt = AM.begin(); AMIt!=AM.end(); ++AMIt) {
                addDerivative(cit->first,AMIt->copy());
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
      itsParameterNamesInvalid = true;
      //CONRADCHECK(itsParams->has(name), "Parameter "+name+" does not exist in the declared parameters");
      itsAMatrix[name].push_back(deriv.copy());
    }

    void DesignMatrix::addResidual(const casa::Vector<casa::Double>& residual, const casa::Vector<double>& weight)
    {
      CONRADDEBUGASSERT(residual.nelements() == weight.nelements());
      itsBVector.push_back(residual.copy());
      itsWeight.push_back(weight.copy());
    }

    /// @brief obtain all parameter names
    /// @details This method builds on-demand and returns a set with parameter
    /// names this design matrix knows about. 
    /// @return a const reference to std::set
    const std::set<std::string> DesignMatrix::parameterNames() const
    {
      if (itsParameterNamesInvalid) {
          itsParameterNames.clear();
          for (std::map<std::string, DMAMatrix>::const_iterator ci = itsAMatrix.begin();
               ci != itsAMatrix.end(); ++ci) {
               itsParameterNames.insert(itsParameterNames.end(),ci->first);
          }
          itsParameterNamesInvalid = false;
      }
      return itsParameterNames;
    }

    const DMAMatrix& DesignMatrix::derivative(const string& name) const
    {
      //CONRADCHECK(itsParams->has(name), "Parameter "+name+" does not exist in the declared parameters");
      CONRADCHECK(itsAMatrix.count(name)>0, "Parameter "+name+" does not exist in the assigned values");
      return itsAMatrix[name];
    }

    const DMBVector& DesignMatrix::residual() const
    {
      return itsBVector;
    }

    const DMWeight& DesignMatrix::weight() const
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
      DMBVector::const_iterator bIt = itsBVector.begin();
      DMWeight::const_iterator wIt = itsWeight.begin(); 
      for (; (bIt!=itsBVector.end())&&(wIt!=itsWeight.end());++bIt, ++wIt)
      {
        sumwt+=casa::sum(*wIt);
        sum+=casa::sum((*wIt)*((*bIt)*(*bIt)));
      }
      CONRADCHECK(sumwt>0.0, "Sum of weights is zero");
      return sqrt(sum/sumwt);
    }

    casa::uInt DesignMatrix::nData() const
    {
      casa::uInt nData=0;
      for (DMBVector::const_iterator bIt=itsBVector.begin();bIt!=itsBVector.end();++bIt)
      {
        nData+=bIt->size();
      }
      return nData;
    }

    int DesignMatrix::nParameters() const
    {
      int nParameters=0;
      for (std::map<string, DMAMatrix>::const_iterator AIt=itsAMatrix.begin();AIt!=itsAMatrix.end();++AIt)
      {
        for (DMAMatrix::const_iterator it=AIt->second.begin();it!=AIt->second.end();++it)
        {
          nParameters+=it->ncolumn();
        }
      }
      return nParameters;
    }

    DesignMatrix::ShPtr DesignMatrix::clone() const 
    {
      return DesignMatrix::ShPtr(new DesignMatrix(*this));
    }

  }
}
