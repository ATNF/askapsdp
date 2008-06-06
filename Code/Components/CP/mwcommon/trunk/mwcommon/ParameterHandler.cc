//#  ParameterHandler.cc: 
//#
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
//#
//#  $Id$

#include <mwcommon/ParameterHandler.h>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace askap { namespace cp {

  ParameterHandler::ParameterHandler (const ParameterSet& parSet)
    : itsParms (parSet)
  {}

  string ParameterHandler::getString (const string& parm,
				      const string& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getString (parm);
    }
    return defVal;
  }

  double ParameterHandler::getDouble (const string& parm,
				      double defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getDouble (parm);
    }
    return defVal;
  }

  unsigned ParameterHandler::getUint (const string& parm,
				      unsigned defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getUint32 (parm);
    }
    return defVal;
  }

  bool ParameterHandler::getBool (const string& parm,
				  bool defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getBool (parm);
    }
    return defVal;
  }

  vector<string> ParameterHandler::getStringVector
  (const string& parm, const vector<string>& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getStringVector (parm);
    }
    return defVal;
  }

  void ParameterHandler::fillString (const string& parm,
				     string& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getString (parm);
    }
  }

  void ParameterHandler::fillDouble (const string& parm,
				     double& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getDouble (parm);
    }
  }

  void ParameterHandler::fillUint (const string& parm,
				   unsigned& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getUint32 (parm);
    }
  }

  void ParameterHandler::fillBool (const string& parm,
				   bool& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getBool (parm);
    }
  }

  void ParameterHandler::fillStringVector (const string& parm,
					   vector<string>& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getStringVector (parm);
    }
  }


  LOFAR::BlobOStream operator<< (LOFAR::BlobOStream& bs,
                                 const LOFAR::ACC::APS::ParameterSet& m)
  {
    bs.putStart ("ParameterSet", 1);
    bs << static_cast<LOFAR::uint32>(m.size());
    for (LOFAR::ACC::APS::ParameterSet::const_iterator it=m.begin();
         it!=m.end();
         ++it) {
      bs << it->first << it->second;
    }
    bs.putEnd();
    return bs;
  }

  LOFAR::BlobIStream operator>> (LOFAR::BlobIStream& bs,
                                 LOFAR::ACC::APS::ParameterSet& m)
  {
    bs.getStart ("ParameterSet");
    m.clear();
    LOFAR::uint32 size;
    bs >> size;
    std::string k,v;
    for (LOFAR::uint32 i=0; i<size; ++i) {
      bs >> k >> v;
      m.add (k, v);
    }
    bs.getEnd();
    return bs;
  }

}} // end namespaces
