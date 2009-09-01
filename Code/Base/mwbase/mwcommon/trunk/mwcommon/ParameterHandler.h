/// @file
/// @brief Handle a LOFAR .parset file
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

#ifndef ASKAP_MWCOMMON_PARAMETERHANDLER_H
#define ASKAP_MWCOMMON_PARAMETERHANDLER_H

#include <Common/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

namespace askap { namespace mwbase {

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
    explicit ParameterHandler (const LOFAR::ParameterSet&);

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
    LOFAR::ParameterSet itsParms;
  };


  // Write/read a ParameterSet into/from a blob.
  // @{
  LOFAR::BlobOStream operator<< (LOFAR::BlobOStream&,
                                 const LOFAR::ParameterSet&);
  LOFAR::BlobIStream operator>> (LOFAR::BlobIStream&,
                                 LOFAR::ParameterSet&);
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
