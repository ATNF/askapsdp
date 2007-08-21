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
             compIt->reset(new UnpolarizedGaussianSource(cur,fluxi,ra,dec,bmaj,
                            bmin,bpa));
          } else {
             // this is a point source
             compIt->reset(new UnpolarizedPointSource(cur,fluxi,ra,dec));
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
  
  // in the future we should move the iteration to the higher level and
  // deal here with the accessor only. It will reduce the number of repeated
  // iterations required         
  for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {

      const casa::Vector<double>& freq=itsIdi->frequency();
      CONRADDEBUGASSERT(freq.nelements()!=0);
      const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = itsIdi->uvw();
                 
// Set up arrays to hold the output values
// Two values (complex) per row, channel, pol
      const casa::uInt nData=itsIdi->nRow()*freq.nelements()*2;
      CONRADDEBUGASSERT(nData!=0);
      casa::Vector<casa::Double> residual(nData);
      // initialize residuals with the observed visibilities
                
      uint offset=0;
      for (casa::uInt row=0;row<itsIdi->nRow(); ++row,offset+=2*freq.nelements()) {
           casa::Vector<casa::Double> residSlice = 
                           residual(casa::Slice(offset,2*freq.nelements()));
           casa::Vector<casa::Complex> visSlice = 
                           itsIdi->visibility().xyPlane(0).row(row);
           casa::Vector<casa::Complex>::const_iterator visIt = visSlice.begin();
                                
           for (casa::Vector<casa::Double>::iterator residIt = residSlice.begin();
                      residIt!=residSlice.end() && visIt!=visSlice.end(); 
                      ++residIt,++visIt) {
                // calculate residuals
                *residIt = real(*visIt);
                ++residIt;
                *(residIt) = imag(*visIt);
           }
      }
                
      DesignMatrix designmatrix(parameters());
      for (std::vector<IParameterizedComponentPtr>::const_iterator compIt = 
                compList.begin(); compIt!=compList.end();++compIt) {
           CONRADDEBUGASSERT(*compIt); 
           const size_t nParameters = (*compIt)->nParameters();
           // Define AutoDiff's for the output visibilities.
           vector<casa::AutoDiff<double> > visDerivBuffer(2*freq.nelements(),
                         casa::AutoDiff<double>(0.,nParameters));
           casa::Array<casa::Double> derivatives(casa::IPosition(2,nData,nParameters));
      
           offset=0;
           // current component
           const IParameterizedComponent& curComp = *(*compIt);
           for (casa::uInt row=0;row<itsIdi->nRow();
                                 ++row,offset+=2*freq.nelements()) {
                curComp.calculate(uvw[row],freq,casa::Stokes::I,visDerivBuffer);
                // copy derivatives from buffer for each parameter
                for (casa::uInt par=0; par<nParameters; ++par) {
                     casa::Vector<casa::Double> derivSlice = 
                           derivatives(casa::IPosition(2,offset,par), 
                              casa::IPosition(2,offset+2*freq.nelements()-1,par));
                     vector<casa::AutoDiff<double> >::const_iterator bufIt =
                           visDerivBuffer.begin();
                     for (casa::Vector<casa::Double>::iterator sliceIt = 
                          derivSlice.begin(); sliceIt!=derivSlice.end() && 
                          bufIt!=visDerivBuffer.end(); ++sliceIt,++bufIt) {
                          
                          // copy derivatives for each spectral channel (both real and 
                          // imaginary parts)
                          *sliceIt = bufIt->derivative(par);
                     }                              
                }
                // add contribution to residuals
                casa::Vector<casa::Double> residSlice = 
                           residual(casa::Slice(offset,2*freq.nelements()));
                vector<casa::AutoDiff<double> >::const_iterator bufIt =
                           visDerivBuffer.begin();
                for (casa::Vector<casa::Double>::iterator residIt = residSlice.begin();
                     residIt!=residSlice.end() && bufIt!=visDerivBuffer.end();
                     ++residIt,++bufIt) {
                     *(residIt) -= bufIt->value();
                }

           }
           // Now we can add the design matrix, residual, and weights
           for (casa::uInt par=0; par<nParameters; ++par) {
                designmatrix.addDerivative((*compIt)->parameterName(par),
                       derivatives(casa::IPosition(2,0,par),
                                   casa::IPosition(2,nData-1,par)));
           }
      }
      casa::Vector<double> weights(nData,1.);
      designmatrix.addResidual(residual, weights);
      ne.add(designmatrix);
  }
}

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
