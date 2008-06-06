/// @file
/// @brief Step to process the MW subtract command using BBSKernel.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_MWSUBTRACTSTEPBBS_H
#define ASKAP_MWCONTROL_MWSUBTRACTSTEPBBS_H

#include <mwcommon/MWSimpleStep.h>
#include <mwcontrol/MWStepBBSProp.h>
#include <vector>
#include <string>

namespace askap { namespace cp {

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
