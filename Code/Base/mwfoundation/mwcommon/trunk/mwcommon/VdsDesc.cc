//# VdsDesc.cc: Describe an entire visibility data set
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/VdsDesc.h>
#include <askap/AskapUtil.h>
#include <ostream>

using namespace std;

namespace askap { namespace mwbase {

  VdsDesc::VdsDesc (const VdsPartDesc& desc,
                    const vector<string>& antNames)
    : itsDesc     (desc),
      itsAntNames (antNames)
  {}

  VdsDesc::VdsDesc (const string& parsetName)
  {
    init (LOFAR::ACC::APS::ParameterSet (parsetName));
  }

  void VdsDesc::init (const LOFAR::ACC::APS::ParameterSet& parset)
  {
    itsDesc = VdsPartDesc (parset);
    itsAntNames = parset.getStringVector ("AntNames");
    int npart = parset.getInt32 ("NParts");
    for (int i=0; i<npart; ++i) {
      ostringstream prefix;
      prefix << "Part" << i << '.';
      LOFAR::ACC::APS::ParameterSet subset = parset.makeSubset (prefix.str());
      itsParts.push_back (VdsPartDesc(subset));
    }
  }

  void VdsDesc::write (ostream& os) const
  {
    itsDesc.write (os, "");
    os << "AntNames = " << itsAntNames << endl;
    os << "NParts = " << itsParts.size() << endl;
    for (unsigned i=0; i<itsParts.size(); ++i) {
      ostringstream prefix;
      prefix << "Part" << i << '.';
      itsParts[i].write (os, prefix.str());
    }
  }

  int VdsDesc::antNr (const string& name) const
  {
    vector<string>::const_iterator inx =
                 find (itsAntNames.begin(), itsAntNames.end(), name);
    if (inx == itsAntNames.end()) {
      return -1;
    }
    return inx - itsAntNames.begin();
  }

  vector<int> VdsDesc::antNrs (const casa::Regex& names) const
  {
    vector<int> result;
    for (unsigned i=0; i<itsAntNames.size(); ++i) {
      if (casa::String(itsAntNames[i]).matches (names)) {
	result.push_back (i);
      }
    }
    return result;
  }

//   int VdsDesc::findPart (const string& fileSystem,
// 			     const vector<int>& done) const
//   {
//     for (unsigned i=0; i<itsParts.size(); ++i) {
//       if (done[i] < 0  &&  itsParts[i].getFileSys() == fileSystem) {
// 	return i;
//       }
//     }
//     return -1;
//   }

}} // end namespaces
