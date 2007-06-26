//#  MWStepTester.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_MWSTEPTESTER_H
#define CONRAD_MWCONTROL_MWSTEPTESTER_H

#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/MWStep.h>
#include <Blob/BlobOStream.h>


namespace conrad { namespace cp {

  class MWStepTester: public MWStepVisitor
  {
  public:
    MWStepTester (int streamId, LOFAR::BlobOStream* out);

    virtual ~MWStepTester();

    // Get the resulting operation.
    int getResultOperation() const
      { return itsOperation; }

  private:
    // Process the various MWStep types.
    //@{
    virtual void visitSolve    (const MWSolveStep&);
    virtual void visitCorrect  (const MWCorrectStep&);
    virtual void visitSubtract (const MWSubtractStep&);
    virtual void visitPredict  (const MWPredictStep&);
    //@}

    // Write the boolean result in the output stream buffer.
    void writeResult (bool result);

    //# Data members.
    int                 itsStreamId;
    int                 itsOperation;
    LOFAR::BlobOStream* itsOut;
  };

}} // end namespaces

#endif
