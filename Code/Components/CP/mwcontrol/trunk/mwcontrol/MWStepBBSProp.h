/// @file
/// @brief The properties for a BBSKernel step.
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

#ifndef ASKAP_MWCONTROL_MWBBSSTEPPROP_H
#define ASKAP_MWCONTROL_MWBBSSTEPPROP_H

#include <mwcommon/DomainShape.h>
#include <vector>
#include <string>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief The properties for a BBSKernel step.

  /// This class defines the base information for any step in the BBSKernel
  /// or similar software. Note that this information is on top of the
  /// information specified in the WorkDomainSpec, thus gives the opportunity
  /// to do some extra selection, etc. for an individual step.
  ///
  /// It defines the following info:
  /// <ul>
  ///  <li> The baselines by means of 2 vectors giving antenna name patterns
  ///       for antenna 1 and 2 of the baselines. The vectors have equal size.
  ///       The baselines are formed for all combinations of each pair.
  ///       E.g. patterns [01],[45] and [23],[678] form baselines
  ///        02, 03, 12, and 23, and 46, 47, 48, 56, 57, and 58.
  ///  <li> The integration size (possibly further integration than already
  ///       done in the WorkDomainSpec).
  ///  <li> If antenna autocorrelations are to be used.
  ///  <li> The polarisation correlations to be used.
  ///  <li> The names of the main sources to be used in the model.
  ///  <li> The names of some extra sources to be used in the model.
  ///  <li> The parts of the instrument model to be used. The BBS instrument
  ///       model contains some predefined parts (DIRGAIN, BANDPASS, ...).
  ///  <li> The output column to be used in the VDS when writing data.
  ///       If blank, no data are written. It makes it possible to write the
  ///       data at the last step only.
  /// </ul>

  class MWStepBBSProp
  {
  public:
    MWStepBBSProp();

    ~MWStepBBSProp();

    /// Set all variables.
    void set (const std::vector<std::string>& station1,
	      const std::vector<std::string>& station2,
	      const DomainShape&              integration,
	      const std::vector<std::string>& corrType,
	      const std::string&              corrSelection,
	      const std::vector<std::string>& sources,
	      const std::vector<std::string>& extraSources,
	      const std::vector<std::string>& instrumentModel,
	      const std::string&              outputData);

    /// Get the variables.
    /// @{
    const std::vector<std::string>& getStation1() const
      { return itsStation1; }
    const std::vector<std::string>& getStation2() const
      { return itsStation2; }
    const DomainShape&              getIntegration() const
      { return itsIntegration; }
    const std::vector<std::string>& getCorrType() const
      { return itsCorrType; }
    const std::string&              getCorrSelection() const
      { return itsCorrSelection; }
    const std::vector<std::string>& getSources() const
      { return itsSources; }
    const std::vector<std::string>& getExtraSources() const
      { return itsExtraSources; }
    const std::vector<std::string>& getInstrumentModel() const
      { return itsInstrumentModel; }
    const std::string&              getOutputData() const
      { return itsOutputData; }
    /// @}

    /// Convert to/from blob.
    /// @{
    void toBlob (LOFAR::BlobOStream&) const;
    void fromBlob (LOFAR::BlobIStream&);
    /// @}

  private:
    std::vector<std::string> itsStation1;
    std::vector<std::string> itsStation2;
    DomainShape              itsIntegration;
    std::vector<std::string> itsCorrType;
    std::string              itsCorrSelection;
    std::vector<std::string> itsSources;
    std::vector<std::string> itsExtraSources;
    std::vector<std::string> itsInstrumentModel;
    std::string              itsOutputData;
  };

}} /// end namespaces

#endif
