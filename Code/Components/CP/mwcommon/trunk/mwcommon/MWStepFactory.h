/// @file
/// @brief Factory pattern to make the correct MWStep object
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWSTEPFACTORY_H
#define CONRAD_MWCOMMON_MWSTEPFACTORY_H

#include <mwcommon/MWStep.h>
#include <map>

namespace conrad { namespace cp {

  /// Factory pattern to make the correct MWStep object

  /// This class contains a map of names to \a create functions
  /// of derived MWStep objects. It is used to reconstruct the correct
  /// MWStep object when reading it back from a blob.
  ///
  /// The map is static, so there is only one instance in a program.
  /// Usually the functions will be registered at the beginning of a program.

  class MWStepFactory
  {
  public:
    /// Define the signature of the function to create an MWStep object.
    typedef MWStep::ShPtr Creator ();

    /// Add a creator function.
    static void push_back (const std::string& name, Creator*);
    
    /// Create the derived MWStep object with the given name.
    /// An exception is thrown if the name is not in the map.
    static MWStep::ShPtr create (const std::string& name);

  private:
    static std::map<std::string, Creator*> itsMap;
  };


}} /// end namespaces

#endif
