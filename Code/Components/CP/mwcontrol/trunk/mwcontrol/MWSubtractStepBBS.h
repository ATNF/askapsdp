/// @file
/// @brief Step to process the MW subtract command using BBSKernel.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSUBTRACTSTEPBBS_H
#define CONRAD_MWCONTROL_MWSUBTRACTSTEPBBS_H

#include <mwcommon/MWSimpleStep.h>
#include <mwcontrol/MWStepBBSProp.h>
#include <vector>
#include <string>

namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Step to process the MW subtract command using BBSKernel.

  /// This class defines a step that subtracts a model from the data
  /// and writes the result into the VDS
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWSubtractStepBBS: public MWSubtractStep
  {
  public:
    MWSubtractStepBBS();

    virtual ~MWSubtractStepBBS();

    /// Clone the step object.
    virtual MWSubtractStepBBS* clone() const;

    /// Create a new object of this type.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Give the (unique) class name of the MWStep.
    virtual std::string className() const;

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWStepVisitor&) const;

    /// Get access to the properties.
    /// @{
    const MWStepBBSProp& getProp() const
      { return itsProp; }
    MWStepBBSProp& getProp()
      { return itsProp; }
    /// @}

    /// Convert to/from blob.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    /// @}

  private:
    MWStepBBSProp itsProp;
  };

}} /// end namespaces

#endif
