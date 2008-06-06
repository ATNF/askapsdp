/// @file
/// @brief A step consisting of several other steps.
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

#ifndef ASKAP_MWCOMMON_MWMULTISTEP_H
#define ASKAP_MWCOMMON_MWMULTISTEP_H

#include <mwcommon/MWStep.h>
#include <list>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief A step consisting of several other steps.

  /// This class makes it possible to form a list of MWStep objects.
  /// Note that the class itself is an MWStep, so the list can be nested.
  /// The \a visit function will call \a visit of each step in the list.
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWMultiStep: public MWStep
  {
  public:
    virtual ~MWMultiStep();

    /// Clone the step object.
    virtual MWMultiStep* clone() const;

    /// Create a new object of this type.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Add a step object (in fact, a clone is added).
    void push_back (const MWStep&);

    /// Give the (unique) class name of the MWStep.
    virtual std::string className() const;

    /// Visit the object, which visits each step.
    virtual void visit (MWStepVisitor&) const;

    /// Convert to/from blob.
    /// Note that reading back from a blob uses MWStepFactory to
    /// create the correct objects.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    /// @}

    /// Define functions and so to iterate in the STL way.
    /// @{
    typedef std::list<MWStep::ShPtr>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsSteps.begin(); }
    const_iterator end() const
      { return itsSteps.end(); }
    /// @}

  private:
    std::list<MWStep::ShPtr> itsSteps;
  };

}} /// end namespaces

#endif
