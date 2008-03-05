/// @file
///
/// @brief Abstract component
/// @details
/// IComponent is a base class for components working with ComponentEquation
/// examples of components include, e.g. Gaussian or point sources.
/// 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#include <measurementequation/IComponent.h>

/// virtual destructor to keep the compiler happy
askap::synthesis::IComponent::~IComponent()  {} 

/// @brief convert StokesTypes into an index 0..3
/// @details It is decided that all components have to be defined in
/// terms of IQUV stokes parameters. It is not prohibited that the 
/// constructors of actual components accept other stokes parameters like
/// XX, etc. However, in the latter case, these parameters should be converted
/// to IQUV at the time of the object construction. Most likely actual 
/// components will hold an array of fluxes for each stokes parameter. Therefore,
/// it is necessary to convert quickly from StokesTypes to the index.
/// This method gives a mapping of I to 0, Q to 1, U to 2 and V to 3. For
/// other values an exception is thrown.
/// @param[in] pol required polarization
/// @return an index (I: 0, Q: 1, U: 2 and V: 3)
size_t askap::synthesis::IComponent::stokesIndex(casa::Stokes::StokesTypes pol) 
                    throw(AskapError)
{
  ASKAPDEBUGASSERT(pol!=casa::Stokes::Undefined && pol<=casa::Stokes::V);
  return static_cast<size_t>(pol-casa::Stokes::I);
}
