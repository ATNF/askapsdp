/// @file VdsPartDesc.h
/// @brief Description of a visibility data set or part thereof.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_VDSPARTDESC_H
#define CONRAD_MWCOMMON_VDSPARTDESC_H

//# Includes
#include <APS/ParameterSet.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace conrad { namespace cp {

  /// Description of a visibility data set or part thereof.

  /// This class holds the description of a visibility data set (VDS) part.
  /// It defines the name of the part and on which file system it is located.
  /// Using the ClusterDesc object it can be derived on which node this
  /// VDS part can be processed best. This is done by the WorkersDesc
  /// class.
  ///
  /// The description of the VDS also contains info about the time,
  /// frequency, and baseline domain of the visibility data.
  ///
  /// Currently the information is made persistent in a LOFAR .parset file.
  /// In the future it needs to use the Centrol Processor Resource Manager.

  class VdsPartDesc
  {
  public:
    /// Construct an empty object.
    VdsPartDesc()
      : itsStartTime(0), itsEndTime(0)
      {}

    /// Construct from the given parameterset.
    explicit VdsPartDesc (const LOFAR::ACC::APS::ParameterSet&);

    /// Set VDS name and file system.
    void setName (const std::string& name, const std::string& fileSys);

    /// Set the start and end time.
    void setTimes (double startTime, double endTime);

    /// Add a band.
    void addBand (int nchan, double startFreq, double endFreq);

    /// Set the baselines.
    void setBaselines (const std::vector<int>& ant1,
		       const std::vector<int>& ant2);

    /// Write it in parset format.
    void write (std::ostream& os, const std::string& prefix) const;

    /// Get the values.
    /// @{
    const std::string& getName() const
      { return itsName; }
    const std::string& getFileSys() const
      { return itsFileSys; }
    double getStartTime() const
      { return itsStartTime; }
    double getEndTime() const
      { return itsEndTime; }
    int getNBand() const
      { return itsNChan.size(); }
    const std::vector<int>& getNChan() const
      { return itsNChan; }
    const std::vector<double>& getStartFreqs() const
      { return itsStartFreqs; }
    const std::vector<double>& getEndFreqs() const
      { return itsEndFreqs; }
    const std::vector<int>& getAnt1() const
      { return itsAnt1; }
    const std::vector<int>& getAnt2() const
      { return itsAnt2; }
    /// @}

  private:
    std::string itsName;       //# full name of the VDS
    std::string itsFileSys;    //# name of file system the VDS resides on
    double      itsStartTime;
    double      itsEndTime;
    std::vector<int>         itsNChan;        //# nr of channels per band
    std::vector<double>      itsStartFreqs;   //# start freq of each band
    std::vector<double>      itsEndFreqs;     //# end freq of each band
    std::vector<int>         itsAnt1;         //# 1st antenna of each baseline
    std::vector<int>         itsAnt2;         //# 2nd antenna of each baseline
  };
    
}} /// end namespaces

#endif
