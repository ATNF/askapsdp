/// @file
/// @brief Handle a LOFAR .parset file
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_PARAMETERHANDLER_H
#define CONRAD_MWCOMMON_PARAMETERHANDLER_H

#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

namespace conrad { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Handle a LOFAR .parset file

  /// This class handles the processing of a LOFAR .parset file
  /// It augments the LOFAR ParameterSet class with functions that can deal
  /// with undefined parameters. There is a set of functions that return
  /// a default value if undefined and a set of functions that leave the
  /// value untouched if undefined.

  class ParameterHandler
  {
  public:
    explicit ParameterHandler (const LOFAR::ACC::APS::ParameterSet&);

    //// Get a parameter value.
    //// An exception is thrown if it does not exist.
    //// @{
    std::string getString (const std::string& parm) const;
    double getDouble (const std::string& parm) const;
    unsigned getUint (const std::string& parm) const;
    bool getBool (const std::string& parm) const;
    std::vector<std::string> getStringVector (const std::string& parm) const;
    //// @}

    //// Get a parameter value.
    //// If it does not exist, the default value is used instead.
    //// @{
    std::string getString (const std::string& parm,
			   const std::string& defVal) const;
    double getDouble (const std::string& parm,
		      double defVal) const;
    unsigned getUint (const std::string& parm,
		      unsigned defVal) const;
    bool getBool (const std::string& parm,
		  bool defVal) const;
    std::vector<std::string> getStringVector
      (const std::string& parm, const std::vector<std::string>& defVal) const;
    //// @}

    //// Get a parameter value and fill \a value with it.
    //// If it does not exist, nothing is done.
    //// @{
    void fillString (const std::string& parm,
		     std::string& value) const;
    void fillDouble (const std::string& parm,
		     double& value) const;
    void fillUint (const std::string& parm,
		   unsigned& value) const;
    void fillBool (const std::string& parm,
		   bool& value) const;
    void fillStringVector (const std::string& parm,
			   std::vector<std::string>& value) const;
    //// @}

  protected:
    LOFAR::ACC::APS::ParameterSet itsParms;
  };


  // Write/read a ParameterSet into/from a blob.
  // @{
  LOFAR::BlobOStream operator<< (LOFAR::BlobOStream&,
                                 const LOFAR::ACC::APS::ParameterSet&);
  LOFAR::BlobIStream operator>> (LOFAR::BlobIStream&,
                                 LOFAR::ACC::APS::ParameterSet&);
  // @}

  inline std::string ParameterHandler::getString (const std::string& parm) const
    { return itsParms.getString (parm); }
  inline double ParameterHandler::getDouble (const std::string& parm) const
    { return itsParms.getDouble (parm); }
  inline unsigned ParameterHandler::getUint (const std::string& parm) const
    { return itsParms.getUint32 (parm); }
  inline bool ParameterHandler::getBool (const std::string& parm) const
    { return itsParms.getBool (parm); }
  inline std::vector<std::string> ParameterHandler::getStringVector
  (const std::string& parm) const
    { return itsParms.getStringVector (parm); }

}} /// end namespaces

#endif
