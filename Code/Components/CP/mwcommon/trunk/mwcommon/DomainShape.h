/// @file
/// @brief Define the shape of a domain.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_DOMAINSHAPE_H
#define CONRAD_MWCOMMON_DOMAINSHAPE_H

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <iosfwd>

namespace conrad { namespace cp {

  /// @ingroup mwcommon
  /// @brief Define the shape of a domain.

  /// This class defines the shape of a domain.
  /// Currently this can only be done for time and frequency.
  ///
  /// This object can be used by ObsDomain to iterate over its observation
  /// domain in chunk of this domain shape.

  class DomainShape
  {
  public:
    /// Set default shape to all frequencies and times.
    DomainShape();

    /// Set from frequency in Hz and time in sec.
    DomainShape (double freqSize, double timeSize);

    /// Get the shape.
    /// @{
    double getFreqSize() const 
      { return itsFreqSize; }
    double getTimeSize() const 
      { return itsTimeSize; }
    /// @}

    /// Convert to/from blob.
    /// @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream&,
					   const DomainShape&);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream&,
					   DomainShape&);
    /// @}

    /// Print.
    friend std::ostream& operator<< (std::ostream&,
				     const DomainShape&);

  private:
    double itsFreqSize;
    double itsTimeSize;
  };

}} /// end namespaces

#endif
