/// @file
/// @brief Abstract base class for steps to process MW commands.
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

#ifndef ASKAP_MWCOMMON_MWSTEP_H
#define ASKAP_MWCOMMON_MWSTEP_H

#include <mwcommon/MWStepVisitor.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <boost/shared_ptr.hpp>

namespace askap { namespace mwbase {

  /// @ingroup mwcommon
  /// @brief Abstract base class for steps to process MW commands.

  /// This class is the abstract base class for all possible steps that
  /// can be executed in the Master-Control framework.
  /// A step must be able to store and retrieve itself into/from a blob.
  ///
  /// The \a visit function uses the visitor pattern to get access to
  /// a concrete MWStep object, for example to execute the step.
  /// It means that a function needs to be added to the visitor classes
  /// for each newly derived MWStep class.
  ///
  /// The MWStepFactory class is a class containing a map of type name to
  /// a \a create function that can create an MWStep object of the required
  /// type. At the beginning of a program the required create functions have
  /// to be registered in the factory. Note that the user can choose which
  /// create function maps to a given name, which makes it possible to
  /// use different implementations of similar functionality.

  class MWStep
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MWStep> ShPtr;

    virtual ~MWStep();

    /// Clone the step object.
    virtual MWStep* clone() const = 0;

    /// Give the (unique) class name of the MWStep.
    virtual std::string className() const = 0;

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visit function.
    virtual void visit (MWStepVisitor&) const;

    /// Convert to/from blob.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const = 0;
    virtual void fromBlob (LOFAR::BlobIStream&) = 0;
    /// @}

    /// Convert to/from blob.
    /// @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
					   const MWStep& step)
      { step.toBlob(bs); return bs; }
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
					   MWStep& step)
      { step.fromBlob(bs); return bs; }
    /// @}
  };


}} /// end namespaces

#endif
