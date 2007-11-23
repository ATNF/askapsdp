#include <fitting/Params.h>
#include <dataaccess/SharedIter.h>
#include <measurementequation/ComponentEquation.h>
#include <fitting/INormalEquations.h>
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

using conrad::scimath::INormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
  namespace synthesis
  {

    ComponentEquation::ComponentEquation(const conrad::scimath::Params& ip,
          const IDataSharedIter& idi) :   MultiChunkEquation(idi),  
           conrad::scimath::GenericEquation(ip), itsAllComponentsUnpolarised(false)
    {
      init();
    };

    ComponentEquation::ComponentEquation(const IDataSharedIter& idi) :
           MultiChunkEquation(idi),  
           itsAllComponentsUnpolarised(false) 
    {
      setParameters(defaultParameters());
      init();
    };
    
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
  
  // we will need to change this variable to false in the loop below, when
  // at least one polarised component is implemented.
  itsAllComponentsUnpolarised = true;
  
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

/// @brief a helper method to populate a visibility cube
/// @details This is method computes visibilities for the one given
/// component and adds them to the cube provided. This is the most
/// generic method, which iterates over polarisations. An overloaded
/// version of the method do the same for unpolarised components
/// (i.e. it doesn't bother to add zeros)
///
/// @param[in] comp component to generate the visibilities for
/// @param[in] uvw baseline spacings, one triplet for each data row.
/// @param[in] freq a vector of frequencies (one for each spectral
///            channel) 
/// @param[in] rwVis a non-const reference to the visibility cube to alter
void ComponentEquation::addModelToCube(const IParameterizedComponent& comp,
       const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw,
       const casa::Vector<casa::Double>& freq,
       casa::Cube<casa::Complex> &rwVis)
{
  CONRADDEBUGASSERT(rwVis.nrow() == uvw.nelements());
  CONRADDEBUGASSERT(rwVis.ncolumn() == freq.nelements());

  // in the future we need to ensure that polarisation 
  // products appear in this order and as Stokes parameters
  const casa::Stokes::StokesTypes polVect[4] =
       { casa::Stokes::I, casa::Stokes::Q, casa::Stokes::U, casa::Stokes::V };
                                
  CONRADDEBUGASSERT(rwVis.nplane()<=4);
  
  // flattened buffer for visibilities 
  std::vector<double> vis(2*freq.nelements()); 
 
  for (casa::uInt row=0;row<rwVis.nrow();++row) {
       for (casa::uInt pol=0; pol<rwVis.nplane(); ++pol) {
           comp.calculate(uvw[row],freq,polVect[pol],vis);
                   
           /// next command adds model visibilities to the
           /// appropriate slice of the visibility cube. Conversions
           /// between complex and two doubles are handled automatically
           addVector(vis,rwVis.xyPlane(pol).row(row));
       }          
  }
}               

/// @brief a helper method to populate a visibility cube
/// @details This is method computes visibilities for the one given
/// component and adds them to the cube provided. This is a second
/// version of the method. It is intended for unpolarised components
/// (i.e. it doesn't bother to add zeros)
///
/// @param[in] comp component to generate the visibilities for
/// @param[in] uvw baseline spacings, one triplet for each data row.
/// @param[in] freq a vector of frequencies (one for each spectral
///            channel) 
/// @param[in] rwVis a non-const reference to the visibility cube to alter
void ComponentEquation::addModelToCube(const IUnpolarizedComponent& comp,
       const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw,
       const casa::Vector<casa::Double>& freq,
       casa::Cube<casa::Complex> &rwVis)
{
  CONRADDEBUGASSERT(rwVis.nrow() == uvw.nelements());
  CONRADDEBUGASSERT(rwVis.ncolumn() == freq.nelements());
  CONRADDEBUGASSERT(rwVis.nplane() >= 1);
  
  // in the future, we have to ensure that the first polarisation product is
  // stokes I. 
 
  // flattened buffer for visibilities 
  std::vector<double> vis(2*freq.nelements()); 
 
  for (casa::uInt row=0;row<rwVis.nrow();++row) {
       comp.calculate(uvw[row],freq,vis);
       
       /// next command adds model visibilities to the
       /// appropriate slice of the visibility cube. Conversions
       /// between complex and two doubles are handled automatically
       addVector(vis,rwVis.xyPlane(0).row(row));
  }           
}

/// @brief Predict model visibilities for one accessor (chunk).
/// @details This version of the predict method works with
/// a single chunk of data only. It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). In the future, I expect that
/// predict() without parameters will be deprecated.
/// @param chunk a read-write accessor to work with
void ComponentEquation::predict(IDataAccessor &chunk) const
{
  const std::vector<IParameterizedComponentPtr> &compList = 
         itsComponents.value(*this,&ComponentEquation::fillComponentCache);
  
  const casa::Vector<casa::Double>& freq = chunk.frequency();
  const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = chunk.uvw();
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
         
  // reset all visibility cube to 0
  rwVis.set(0.);
         
  // loop over components
  for (std::vector<IParameterizedComponentPtr>::const_iterator compIt = 
       compList.begin(); compIt!=compList.end();++compIt) {
       
       CONRADDEBUGASSERT(*compIt); 
       // current component
       const IParameterizedComponent& curComp = *(*compIt);
       try {
            const IUnpolarizedComponent &unpolComp = 
              dynamic_cast<const IUnpolarizedComponent&>(curComp);
            addModelToCube(unpolComp,uvw,freq,rwVis);
       }
       catch (const std::bad_cast&) {
            addModelToCube(curComp,uvw,freq,rwVis);
       }
  }
}


/// @brief a helper method to update design matrix and residuals
/// @details This method iterates over a given number of polarisation 
/// products in the visibility cube. It updates the design matrix with
/// derivatives and subtracts values from the vector of residuals.
/// The latter is a flattened vector which should have a size of 
/// 2*nChan*nPol*nRow. Spectral channel is the most frequently varying
/// index, then follows the polarisation index, and the least frequently
/// varying index is the row. The number of channels and the number of
/// rows always corresponds to that of the visibility cube. The number of
/// polarisations can be less than the number of planes in the cube to
/// allow processing of incomplete data cubes (or unpolarised components). In contrast to 
/// 
/// @param[in] comp component to generate the visibilities for
/// @param[in] uvw baseline coorindates for each row
/// @param[in] freq a vector of frequencies (one frequency for each
///            spectral channel)
/// @param[in] dm design matrix to update (to add derivatives to)
/// @param[in] residual vector of residuals to update 
/// @param[in] nPol a number of polarisation products to process
void ComponentEquation::updateDesignMatrixAndResiduals(
                   const IParameterizedComponent& comp,
                   const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw,
                   const casa::Vector<casa::Double>& freq,
                   scimath::DesignMatrix &dm, casa::Vector<casa::Double> &residual,
                   casa::uInt nPol)
{
  const size_t nParameters = comp.nParameters();
  // number of data points  in the flattened vector
  const casa::uInt nData=nPol*uvw.nelements()*freq.nelements()*2;
  CONRADDEBUGASSERT(nData!=0);
  CONRADDEBUGASSERT(nPol<=4);
  CONRADDEBUGASSERT(residual.nelements() == nData);
      
  // Define AutoDiffs to buffer the output of a single call to the calculate 
  // method of the component.
  vector<casa::AutoDiff<double> > visDerivBuffer(2*freq.nelements(),
                          casa::AutoDiff<double>(0.,nParameters));
  casa::Array<casa::Double> derivatives(casa::IPosition(2,nData, nParameters));
           
  for (casa::uInt row=0,offset=0; row<uvw.nelements(); ++row) {
       
       const casa::RigidVector<casa::Double, 3> &thisRowUVW = uvw[row];
       
       // in the future we need to ensure that polarisation 
       // products appear in this order and as Stokes parameters
       const casa::Stokes::StokesTypes polVect[4] =
           { casa::Stokes::I, casa::Stokes::Q, casa::Stokes::U, casa::Stokes::V };
       
       for (casa::uInt pol=0; pol<nPol; ++pol,offset+=2*freq.nelements()) {
            comp.calculate(thisRowUVW,freq,polVect[pol],visDerivBuffer);
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
  }
  // Now we can add the design matrix, residual, and weights
  for (casa::uInt par=0; par<nParameters; ++par) {
       dm.addDerivative(comp.parameterName(par), 
              derivatives(casa::IPosition(2,0,par),
                          casa::IPosition(2,nData-1,par)));
  }                
}

/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This version of the method works on a single chunk of
/// data only (one iteration).It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). In the future, I expect that
/// the variant of the method without parameters will be deprecated.
/// @param[in] chunk a read-write accessor to work with
/// @param[in] ne Normal equations
void ComponentEquation::calcEquations(const IConstDataAccessor &chunk,
                   conrad::scimath::GenericNormalEquations& ne) const
{
  const std::vector<IParameterizedComponentPtr> &compList = 
         itsComponents.value(*this,&ComponentEquation::fillComponentCache);
  
  const casa::Vector<double>& freq=chunk.frequency();
  CONRADDEBUGASSERT(freq.nelements()!=0);
  const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = chunk.uvw();
  const casa::Cube<casa::Complex> &visCube = chunk.visibility();
                 
  // maximum number of polarisations to process, can be less than
  // the number of planes in the visibility cube, if all components are
  // unpolarised
  const casa::uInt nPol = itsAllComponentsUnpolarised ? 1 : chunk.nPol();
  CONRADDEBUGASSERT(nPol <= chunk.visibility().nplane());
      
  // Set up arrays to hold the output values
  // Two values (complex) per row, channel, pol
  const casa::uInt nData=chunk.nRow()*freq.nelements()*2*nPol;
  CONRADDEBUGASSERT(nData!=0);
  casa::Vector<casa::Double> residual(nData);
      
  // initialize residuals with the observed visibilities          
  for (casa::uInt row=0,offset=0; row < chunk.nRow(); ++row) {
       for (casa::uInt pol=0; pol<nPol; ++pol,offset+=2*freq.nelements()) {
            // the following command copies visibility slice to the appropriate 
            // residual slice converting complex to two doubles automatically
            // via templates 
            copyVector(visCube.xyPlane(pol).row(row),
                  residual(casa::Slice(offset,2*freq.nelements())));      
       } 
  }
                
  DesignMatrix designmatrix; // old parameters: parameters();
  for (std::vector<IParameterizedComponentPtr>::const_iterator compIt = 
            compList.begin(); compIt!=compList.end();++compIt) {
       CONRADDEBUGASSERT(*compIt); 
       updateDesignMatrixAndResiduals(*(*compIt),uvw,freq,designmatrix,
                           residual,nPol);
  }
  casa::Vector<double> weights(nData,1.);
  designmatrix.addResidual(residual, weights);
  ne.add(designmatrix);
}

/// @brief read-write access to parameters
/// @details This method is overridden to invalidate component cache.
/// @return a non-const reference to Param::ShPtr
scimath::Params::ShPtr& ComponentEquation::rwParameters() throw()
{ 
  itsComponents.invalidate();
  return scimath::Equation::rwParameters();
}

/// @brief Calculate the normal equations for the iterator
/// @details This version iterates through all chunks of data and
/// calls an abstract method declared in IMeasurementEquation for each 
/// individual accessor (each iteration of the iterator)
/// @note there is probably a problem with constness here. Hope this method is
/// only temporary here.
/// @param[in] ne Normal equations
void ComponentEquation::calcGenericEquations(conrad::scimath::GenericNormalEquations& ne)
{
  MultiChunkEquation::calcGenericEquations(ne);
}

} // namespace synthesis

} // namespace conrad
