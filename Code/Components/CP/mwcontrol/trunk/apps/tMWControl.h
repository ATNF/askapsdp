//#  MWIos.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_tMWCONTROL_H
#define CONRAD_tMWCONTROL_H

#include <iostream>
#include <fstream>
#include <string>
#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/MWStep.h>
#include <mwcontrol/PredifferProxy.h>
#include <Blob/BlobOStream.h>
#include <mwcontrol/SolverProxy.h>

#define MWCOUT MWIos::os()

namespace conrad { namespace cp {

  // MPI has the problem that the output of cout is unpredictable.
  // Therefore the output of tMWControl is using a separate output
  // file for each rank.
  // This class makes this possible. The alias MWCOUT can be used for it.
  class MWIos
  {
  public:
    static void setName (const std::string& name)
      { itsName = name; }
    static std::ofstream& os()
      { if (!itsIos) makeIos(); return *itsIos; }

  private:
    static void makeIos();

    static std::string    itsName;
    static std::ofstream* itsIos;
  };

}} // end namespaces

//#  MWStepTester.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

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

//#  PredifferTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$
namespace conrad { namespace cp {

  class PredifferTest: public PredifferProxy
  {
  public:
    PredifferTest();

    ~PredifferTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out);
  };

}} // end namespaces

//#  SolverTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

namespace conrad { namespace cp {

  class SolverTest: public SolverProxy
  {
  public:
    SolverTest();

    ~SolverTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out);

  private:
    int itsMaxIter;
    int itsNrIter;
  };

}} // end namespaces

#endif
