/// @file
/// @brief Specification of a predict step (simulation).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCONTROL_MWPREDICTSPEC_H
#define CONRAD_MWCONTROL_MWPREDICTSPEC_H

//# Includes
#include <mwcontrol/MWSingleSpec.h>

namespace conrad { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Specification of a predict step (simulation).

  /// This is a so-called \e leaf class in the MWSpec composite pattern (see
  /// Design Patterns, Gamma et al, 1995).
  ///
  /// It implements a predict step specification which is read from a
  /// LOFAR .parset file. The base class holds all data members.
  /// This class implements the required virtual functions.

  class MWPredictSpec : public MWSingleSpec
  {
  public:
    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWPredictSpec(const std::string& name, 
		  const LOFAR::ACC::APS::ParameterSet& parSet,
		  const MWSpec* parent)
      : MWSingleSpec(name, parSet, parent) {}

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents in human readable form into the output stream.
    /// Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;
  };

}} /// end namespaces

#endif
