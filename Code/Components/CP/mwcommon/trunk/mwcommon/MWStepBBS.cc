//# MWStepBBS.cc: A step consisting of several other steps.
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;
using namespace std;

namespace conrad { namespace cp {

  MWStepBBS::~MWStepBBS()
  {}

  void MWStepBBS::set (const vector<string>& station1,
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

  void MWStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWStepBBS", 1);
    bs << itsStation1 << itsStation2
       << itsIntegration << itsCorrType << itsCorrSelection
       << itsSources << itsExtraSources << itsInstrumentModel
       << itsOutputData;
    bs.putEnd();
  }

  void MWStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWStepBBS");
    CONRADASSERT (vers == 1);
    bs >> itsStation1 >> itsStation2
       >> itsIntegration >> itsCorrType >> itsCorrSelection
       >> itsSources >> itsExtraSources >> itsInstrumentModel
       >> itsOutputData;
    bs.getEnd();
  }

}} // end namespaces
