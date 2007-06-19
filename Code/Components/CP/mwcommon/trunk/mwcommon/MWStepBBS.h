/// @file MWStepBBS.h
/// @brief The base class for a BBSKernel step.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MWBBSSTEP_H
#define CONRAD_MWCOMMON_MWBBSSTEP_H

#include <mwcommon/MWStep.h>
#include <mwcommon/DomainShape.h>
#include <vector>
#include <string>

namespace conrad { namespace cp {

  /// The base class for a BBSKernel step.

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

  class MWStepBBS: public MWStep
  {
  public:
    virtual ~MWStepBBS();

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
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
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
