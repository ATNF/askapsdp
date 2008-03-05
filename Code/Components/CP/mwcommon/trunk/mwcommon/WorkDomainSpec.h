/// @file
/// @brief Define the specifications of the work domain.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_WORKDOMAINSPEC_H
#define ASKAP_MWCOMMON_WORKDOMAINSPEC_H

#include <mwcommon/DomainShape.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <vector>
#include <string>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Define the specifications of the work domain.

  /// This class defines the properties of a work domain. It contains:
  /// <ul>
  ///  <li> The size in time and freq.
  ///  <li> The integration to be done in time and/or freq.
  ///  <li> The input data column.
  ///  <li> The antenna numbers or names to be selected.
  ///  <li> If autocorrelations between antennas are to be selected.
  ///  <li> The polarisation correlations to be selected.
  /// </ul>
  /// A work domain defines the amount of data a worker can hold in memory.
  ///
  /// The control will iterate over the entire data set in chunk of the
  /// work domain size. For each chunk it will perform the steps as defined
  /// by an MWMultiStep object on the data in the work domain or a subset
  /// of them.

  class WorkDomainSpec
  {
  public:
    /// Default constructor (for containers).
    WorkDomainSpec()
      : itsInColumn("DATA"),
	itsAutoCorr(false)
    {}

    /// Set/get work domain shape.
    /// @{
    void setShape (const DomainShape& shape)
      { itsShape = shape; }
    const DomainShape& getShape() const
      { return itsShape; }
    /// @}

    /// Set/get integration interval in frequency or time.
    /// @{
    void setFreqIntegration (double hz)
      { itsFreqInt = hz; }
    void setTimeIntegration (double sec)
      { itsTimeInt = sec; }
    double getFreqIntegration() const
      { return itsFreqInt; }
    double getTImeIntegration() const
      { return itsTimeInt; }
    /// @}

    /// Set/get the input data column to use.
    /// @{
    void setInColumn (const std::string& inColumn)
      { itsInColumn = inColumn; }
    const std::string& getInColumn() const
      { return itsInColumn; }
    /// @}

    /// Set/get the antennas to use (0-based numbers).
    /// @{
    void setAntennas (const std::vector<int>& antNrs);
    const std::vector<int>& getAntennas() const
      { return itsAntNrs; }
    /// @}

    /// Set/get antennas by name patterns.
    /// Each name can be a filename-like pattern.
    /// @{
    void setAntennaNames (const std::vector<std::string>& antNamePatterns);
    const std::vector<std::string>& getAntennaNames() const
      { return itsAntNames; }
    /// @}

    /// Set/get the autocorrelations flag.
    /// @{
    void setAutoCorr (bool autoCorr)
      { itsAutoCorr = autoCorr; }
    bool getAutoCorr() const
      { return itsAutoCorr; }
    /// @}

    /// Set/get the correlations to use.
    /// @{
    void setCorr (const std::vector<bool>& corr);
    const std::vector<bool>& getCorr() const
      { return itsCorr; }
    /// @}

    /// Write or read the object into/from a blob stream.
    /// @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream&,
					   const WorkDomainSpec&);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream&,
					   WorkDomainSpec&);
    /// @}

  private:
    std::string              itsInColumn;
    std::vector<int>         itsAntNrs;
    std::vector<std::string> itsAntNames;
    bool                     itsAutoCorr;
    std::vector<bool>        itsCorr;
    DomainShape              itsShape;
    double                   itsFreqInt;
    double                   itsTimeInt;
  };

}} /// end namespaces

#endif
