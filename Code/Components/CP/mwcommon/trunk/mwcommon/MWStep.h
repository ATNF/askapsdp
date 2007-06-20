/// @file
/// @brief Abstract base class for steps to process MW commands.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWSTEP_H
#define CONRAD_MWCOMMON_MWSTEP_H

#include <mwcommon/MWStepVisitor.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <boost/shared_ptr.hpp>

namespace conrad { namespace cp {

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

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWStepVisitor&) const = 0;

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
