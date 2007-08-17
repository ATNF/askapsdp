#include <fitting/Params.h>
#include <dataaccess/SharedIter.h>
#include <measurementequation/ComponentEquation.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>


#include <measurementequation/UnpolarizedPointSource.h>
#include <measurementequation/UnpolarizedGaussianSource.h>


#include <stdexcept>

using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
  namespace synthesis
  {

    ComponentEquation::ComponentEquation(const conrad::scimath::Params& ip,
          IDataSharedIter& idi) :  conrad::scimath::Equation(ip),
          itsIdi(idi) 
    {
      init();
    };

    ComponentEquation::ComponentEquation(IDataSharedIter& idi) :  
      conrad::scimath::Equation(), itsIdi(idi) 
    {
      itsParams=defaultParameters().clone();
      init();
    };

    ComponentEquation::ComponentEquation(const ComponentEquation& other)
    {
      operator=(other);
    }

    ComponentEquation& ComponentEquation::operator=(const ComponentEquation& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
        itsIdi=other.itsIdi;
      }
    }
    
    ComponentEquation::~ComponentEquation() 
    {
    }

    void ComponentEquation::init()
    {
    }

conrad::scimath::Params ComponentEquation::defaultParameters()
{
// The default parameters serve as a holder for the patterns to match the actual
// parameters. Shell pattern matching rules apply.
      conrad::scimath::Params ip;
      ip.add("flux.i");
      ip.add("direction.ra");
      ip.add("direction.dec");
      ip.add("shape.bmaj");
      ip.add("shape.bmin");
      ip.add("shape.bpa");
      return ip;
}

/// @brief fill the cache of the components
/// @details This method convertes the parameters into a vector of 
/// components. It is called on the first access to itsComponents
void ComponentEquation::fillComponentCache(
            std::vector<IParameterizedComponentPtr> &in) const
{
  std::vector<std::string> completions(parameters().completions("flux.i"));
  in.resize(completions.size());
  if(!completions.size()) {
     return;
  }
  
  // This loop is over all strings that complete the flux.i.* pattern
  // correctly. An exception will be throw if the parameters are not
  // consistent
  std::vector<IParameterizedComponentPtr>::iterator compIt=in.begin();
  for (std::vector<std::string>::const_iterator it=completions.begin();
        it!=completions.end();++it,++compIt)  {
          const std::string &cur = *it;
          
          const double ra=parameters().scalarValue("direction.ra"+cur);
          const double dec=parameters().scalarValue("direction.dec"+cur);
          const double fluxi=parameters().scalarValue("flux.i"+cur);
          const double bmaj=parameters().scalarValue("shape.bmaj"+cur);
          const double bmin=parameters().scalarValue("shape.bmin"+cur);
          const double bpa=parameters().scalarValue("shape.bpa"+cur);
          
          if((bmaj>0.0)&&(bmin>0.0)) {
             // this is a gaussian
             compIt->reset(new UnpolarizedGaussianSource(fluxi,ra,dec,bmaj,
                            bmin,bpa));
          } else {
             // this is a point source
             compIt->reset(new UnpolarizedPointSource(fluxi,ra,dec));
          }
        }
  
}                  
    
void ComponentEquation::predict()
{
    const std::vector<IParameterizedComponentPtr> &compList = 
           itsComponents.value(*this,&ComponentEquation::fillComponentCache);
      
    // in the future we should move the iteration to the higher level and
    // deal hear with the accessor only. It will reduce the number of repeated
    // iterations required         
    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {
         const casa::Vector<casa::Double>& freq=itsIdi->frequency();
         const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = itsIdi->uvw();
         casa::Cube<casa::Complex> &rwVis = itsIdi->rwVisibility();
         std::vector<double> vis(2*freq.nelements());
         
         // loop over components
         for (std::vector<IParameterizedComponentPtr>::const_iterator compIt = 
              compList.begin(); compIt!=compList.end();++compIt) {
              CONRADDEBUGASSERT(*compIt); 
              // current component
              const IParameterizedComponent& curComp = *(*compIt);
              for (casa::uInt row=0;row<itsIdi->nRow();++row) {
                   curComp.calculate(uvw[row],freq,casa::Stokes::I,vis);
            
                   for (casa::uInt i=0;i<freq.nelements();++i) {
                        rwVis(row,i,0) += casa::Complex(vis[2*i], vis[2*i+1]);
                   }
              }
         }
    }
};

void ComponentEquation::calcEquations(conrad::scimath::NormalEquations& ne)
{
  const std::vector<IParameterizedComponentPtr> &compList = 
           itsComponents.value(*this,&ComponentEquation::fillComponentCache);
   
  std::vector<std::string> completions(parameters().completions("flux.i"));
      
  for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {

       const casa::Vector<double>& freq=itsIdi->frequency();
       const double time=itsIdi->time();

       const uint nParameters=6;

// Define AutoDiff's for the output visibilities.
       casa::Vector<casa::AutoDiff<double> > av(2*freq.nelements(),
                         casa::AutoDiff<double>(0.,nParameters));
        
// Set up arrays to hold the output values
// Two values (complex) per row, channel, pol
        uint nData=itsIdi->nRow()*freq.nelements()*2;
        casa::Vector<casa::Double> raDeriv(nData);
        casa::Vector<casa::Double> decDeriv(nData);
        casa::Vector<casa::Double> fluxiDeriv(nData);
        casa::Vector<casa::Double> bmajDeriv(nData);
        casa::Vector<casa::Double> bminDeriv(nData);
        casa::Vector<casa::Double> bpaDeriv(nData);
        casa::Vector<casa::Double> residual(nData);
        casa::Vector<double> weights(nData);

        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          DesignMatrix designmatrix(parameters());

          uint offset=0;

          string raName("direction.ra"+(*it));
          string decName("direction.dec"+(*it));
          string fluxName("flux.i"+(*it));
          string bmajName("shape.bmaj"+(*it));
          string bminName("shape.bmin"+(*it));
          string bpaName("shape.bpa"+(*it));

// Define AutoDiff's for the three unknown parameters
          casa::AutoDiff<double> ara(parameters().scalarValue(raName), nParameters, 0);
          casa::AutoDiff<double> adec(parameters().scalarValue(decName), nParameters, 1);
          casa::AutoDiff<double> afluxi(parameters().scalarValue(fluxName), nParameters, 2);
          casa::AutoDiff<double> abmaj(parameters().scalarValue(bmajName), nParameters, 3);
          casa::AutoDiff<double> abmin(parameters().scalarValue(bminName), nParameters, 4);
          casa::AutoDiff<double> abpa(parameters().scalarValue(bpaName), nParameters, 5);

          for (uint row=0;row<itsIdi->nRow();row++)
          {

            if((abmaj>0.0)&&(abmin>0.0))
            {
              this->calcRegularGauss<casa::AutoDiff<double> >(ara, adec,
                afluxi, abmaj, abmin, abpa, freq,
                itsIdi->uvw()(row)(0),
                itsIdi->uvw()(row)(1),
                itsIdi->uvw()(row)(3),
                av);
            }
            else
            {
              this->calcRegularPoint<casa::AutoDiff<double> >(ara, adec,
                afluxi, freq,
                itsIdi->uvw()(row)(0),
                itsIdi->uvw()(row)(1),
                itsIdi->uvw()(row)(3),
                av);
            }

            for (uint i=0;i<freq.nelements();i++)
            {
              residual(2*i+offset)=real(itsIdi->visibility()(row,i,0))-av(2*i).value();
              residual(2*i+1+offset)=imag(itsIdi->visibility()(row,i,0))-av(2*i+1).value();
              raDeriv(2*i+offset)=av[2*i].derivative(0);
              raDeriv(2*i+1+offset)=av(2*i+1).derivative(0);
              decDeriv(2*i+offset)=av[2*i].derivative(1);
              decDeriv(2*i+1+offset)=av(2*i+1).derivative(1);
              fluxiDeriv(2*i+offset)=av[2*i].derivative(2);
              fluxiDeriv(2*i+1+offset)=av(2*i+1).derivative(2);
              bmajDeriv(2*i+offset)=av[2*i].derivative(3);
              bmajDeriv(2*i+1+offset)=av(2*i+1).derivative(3);
              bminDeriv(2*i+offset)=av[2*i].derivative(4);
              bminDeriv(2*i+1+offset)=av(2*i+1).derivative(4);
              bpaDeriv(2*i+offset)=av[2*i].derivative(5);
              bpaDeriv(2*i+1+offset)=av(2*i+1).derivative(5);
              weights(2*i+offset)=1.0;
              weights(2*i+1+offset)=1.0;
            }

            offset+=2*freq.nelements();
          }
// Now we can add the design matrix, residual, and weights
          designmatrix.addDerivative(raName, raDeriv);
          designmatrix.addDerivative(decName, decDeriv);
          designmatrix.addDerivative(fluxName, fluxiDeriv);
          designmatrix.addDerivative(bmajName, bmajDeriv);
          designmatrix.addDerivative(bminName, bminDeriv);
          designmatrix.addDerivative(bpaName, bpaDeriv);
          designmatrix.addResidual(residual, weights);

          ne.add(designmatrix);
        }
      }
    };

// This can be done easily by hand (and we should do for production) but I'm leaving
// it in this form for the moment to show how the differentiation is done using
// casa::AutoDiff
    template<class T>
      void ComponentEquation::calcRegularPoint(const T& ra, const T& dec, const T& flux,
      const casa::Vector<double>& freq,
      const double u, const double v, const double w,
      casa::Vector<T>& vis)
    {
      T n =  casa::sqrt(T(1.0) - (ra*ra+dec*dec));
      T delay = casa::C::_2pi * (ra * u + dec * v + n * w)/casa::C::c;
      T scale = 1.0/casa::C::c;
      for (uint i=0;i<freq.nelements();i++)
      {
        T phase = delay * freq(i);
        vis(2*i)   = flux * cos(phase);
        vis(2*i+1) = flux * sin(phase);
      }
    }

    template<class T>
      void ComponentEquation::calcRegularGauss(const T& ra, const T& dec, const T& flux,
      const T& bmaj, const T& bmin, const T& bpa,
      const casa::Vector<double>& freq,
      const double u, const double v, const double w,
      casa::Vector<T>& vis)
    {
      T n =  casa::sqrt(T(1.0) - (ra*ra+dec*dec));
      T delay = casa::C::_2pi * (ra * u + dec * v + n * w)/casa::C::c;
// exp(-a*x^2) transforms to exp(-pi^2*u^2/a)
// a=4log(2)/FWHM^2 so scaling = pi^2*FWHM/(4log(2))
      T scale = std::pow(casa::C::pi,2)/(4*log(2));
      T up=( cos(bpa)*u + sin(bpa)*v)/casa::C::c;
      T vp=(-sin(bpa)*u + cos(bpa)*v)/casa::C::c;
      T r=(bmaj*bmaj*up*up+bmin*bmin*vp*vp)*scale;
      for (uint i=0;i<freq.nelements();i++)
      {
        T phase = delay * freq(i);
        T decorr = exp( - r * freq(i) * freq(i));
        vis(2*i)   = flux * decorr * cos(phase);
        vis(2*i+1) = flux * decorr * sin(phase);
      }
    }

  }

}
