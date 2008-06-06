//# MWStepBBSProp.cc
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

#include <mwcontrol/MWStepBBSProp.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;
using namespace std;

namespace askap { namespace cp {

  MWStepBBSProp::MWStepBBSProp()
  {}

  MWStepBBSProp::~MWStepBBSProp()
  {}

  void MWStepBBSProp::set (const vector<string>& station1,
                           const vector<string>& station2,
                           const DomainShape&    integration,
                           const vector<string>& corrType,
                           const string&         corrSelection,
                           const vector<string>& sources,
                           const vector<string>& extraSources,
                           const vector<string>& instrumentModel,
                           const string&         outputData)
  {
    itsStation1        = station1;
    itsStation2        = station2;
    itsIntegration     = integration;
    itsCorrType        = corrType;
    itsCorrSelection   = corrSelection;
    itsSources         = sources;
    itsExtraSources    = extraSources;
    itsInstrumentModel = instrumentModel;
    itsOutputData      = outputData;
  }

  void MWStepBBSProp::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWStepBBSProp", 1);
    bs << itsStation1 << itsStation2
       << itsIntegration << itsCorrType << itsCorrSelection
       << itsSources << itsExtraSources << itsInstrumentModel
       << itsOutputData;
    bs.putEnd();
  }

  void MWStepBBSProp::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWStepBBSProp");
    ASKAPASSERT (vers == 1);
    bs >> itsStation1 >> itsStation2
       >> itsIntegration >> itsCorrType >> itsCorrSelection
       >> itsSources >> itsExtraSources >> itsInstrumentModel
       >> itsOutputData;
    bs.getEnd();
  }

}} // end namespaces
