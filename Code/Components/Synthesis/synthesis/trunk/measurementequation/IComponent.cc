/// @file
///
/// @brief Abstract component
/// @details
/// IComponent is a base class for components working with ComponentEquation
/// examples of components include, e.g. Gaussian or point sources.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// 

#include <measurementequation/IComponent.h>

/// virtual destructor to keep the compiler happy
conrad::synthesis::IComponent::~IComponent()  {} 
