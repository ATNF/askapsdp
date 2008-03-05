/// @file
/// @brief Step to process the MW correct command using BBSKernel.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_MWCORRECTSTEPBBS_H
#define ASKAP_MWCONTROL_MWCORRECTSTEPBBS_H

#include <mwcommon/MWSimpleStep.h>
#include <mwcontrol/MWStepBBSProp.h>
#include <vector>
#include <string>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Step to process the MW correct command using BBSKernel.

  /// This class defines a step that corrects the data in the VDS for
  /// the given parameters.
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWCorrectStepBBS: public MWCorrectStep
  {
  public:
    MWCorrectStepBBS();

    virtual ~MWCorrectStepBBS();

    /// Clone the step object.
    virtual MWCorrectStepBBS* clone() const;

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
