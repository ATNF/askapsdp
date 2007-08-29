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
#include <measurementequation/VectorOperations.h>


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
    // deal here with the accessor only. It will reduce the number of repeated
    // iterations required         
    for (itsIdi.init();itsIdi.hasMore();itsIdi.next()) {
         const casa::Vector<casa::Double>& freq=itsIdi->frequency();
         const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = itsIdi->uvw();
         casa::Cube<casa::Complex> &rwVis = itsIdi->rwVisibility();
         std::vector<double> vis(2*freq.nelements());
         
         // reset all visibility cube to 0
         rwVis.set(0.);
         
         // loop over components
         for (std::vector<IParameterizedComponentPtr>::const_iterator compIt = 
              compList.begin(); compIt!=compList.end();++compIt) {
              CONRADDEBUGASSERT(*compIt); 
              // current component
              const IParameterizedComponent& curComp = *(*compIt);
              for (casa::uInt row=0;row<itsIdi->nRow();++row) {
                   curComp.calculate(uvw[row],freq,casa::Stokes::I,vis);
                   
                   /// next command adds model visibilities to the
                   /// appropriate slice of the visibility cube. Conversions
                   /// between complex and two doubles are handled automatically
                   addVector(vis,rwVis.xyPlane(0).row(row));
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
                
      for (casa::uInt row=0,offset=0; row<itsIdi->nRow(); 
                                      ++row,offset+=2*freq.nelements()) {
           // the following command copies visibility slice to the appropriate 
           // residual slice converting complex to two doubles automatically
           // via templates 
           copyVector(itsIdi->visibility().xyPlane(0).row(row),
                      residual(casa::Slice(offset,2*freq.nelements())));       
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
           
           // current component
           const IParameterizedComponent& curComp = *(*compIt);
           for (casa::uInt row=0,offset=0; row<itsIdi->nRow();
                                 ++row,offset+=2*freq.nelements()) {
                curComp.calculate(uvw[row],freq,casa::Stokes::I,visDerivBuffer);
                // copy derivatives from buffer for each parameter
                for (casa::uInt par=0; par<nParameters; ++par) {
                     // copy derivatives for each channel from visDerivBuffer
                     // to the appropriate slice of the derivatives Array
                     // template takes care of the actual types
                     copyDerivativeVector(par,visDerivBuffer,  
                           derivatives(casa::IPosition(2,offset,par), 
                           casa::IPosition(2,offset+2*freq.nelements()-1,par)));                                 
                }
                // subtract contribution from the residuals
                // next command does: residual slice -= visDerivBuffer
                // taking care of all type conversions via templates
                subtractVector(visDerivBuffer,
                         residual(casa::Slice(offset,2*freq.nelements())));          
  
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

} // namespace synthesis

} // namespace conrad
