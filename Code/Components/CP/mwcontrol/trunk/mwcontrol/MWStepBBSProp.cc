//# MWStepBBSProp.cc
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
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
