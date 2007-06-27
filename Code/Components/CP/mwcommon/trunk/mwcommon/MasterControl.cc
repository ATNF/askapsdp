//# MasterControl.cc: Master controller of distributed VDS processing
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MasterControl.h>
#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MWCorrectStep.h>
#include <mwcommon/MWSubtractStep.h>
#include <mwcommon/MWPredictStep.h>
#include <mwcommon/MWBlobIO.h>

using namespace std;


namespace conrad { namespace cp {

  MasterControl::MasterControl (const MWConnectionSet::ShPtr& prediffers,
				const MWConnectionSet::ShPtr& solvers)
    : itsPrediffers (prediffers),
      itsSolvers    (solvers)
  {}

  MasterControl::~MasterControl()
  {}

  void MasterControl::setInitInfo (const std::string& msName,
				   const std::string& colName,
				   const std::string& skyDB,
				   const std::string& instDB,
				   unsigned int subBand,
				   bool calcUVW,
				   const ObsDomain& fullDomain)
  {
    itsFullDomain = fullDomain;
    // Fill the DataHolder as much as possible.
    LOFAR::BlobString buf;
    int workerId = 0;
    for (int i=0; i<itsPrediffers->size(); ++i) {
      buf.resize (0);
      MWBlobOut out(buf, MasterControl::Init, 0, workerId);
      out.blobStream() << msName << "" << colName << skyDB << instDB
                       << subBand << calcUVW;
      out.finish();
      itsPrediffers->write (i, buf);
      ++workerId;
    }
    for (int i=0; i<itsSolvers->size(); ++i) {
      buf.resize (0);
      MWBlobOut out(buf, MasterControl::Init, 0, workerId);
      out.blobStream() << msName << "" << colName << skyDB << instDB
                       << subBand << calcUVW;
      out.finish();
      itsSolvers->write (i, buf);
      ++workerId;
    }
    // Now read the replies back. They contain no info, but merely show
    // the worker is alive.
    readAllWorkers (true, true);
  }

  void MasterControl::setWorkDomainSpec (const WorkDomainSpec& wds)
  {
    itsWds = wds;
  }

  void MasterControl::processSteps (const MWStep& step)
  {
    // Iterate through the full observation domain.
    ObsDomain workDomain;
    while (itsFullDomain.getNextWorkDomain (workDomain, itsWds.getShape())) {
      // Send WorkDomain to all prediffers and solver.
      LOFAR::BlobString buf;
      MWBlobOut out(buf, MasterControl::SetWd, 0);
      out.blobStream() << workDomain;
      out.finish();
      itsPrediffers->writeAll (buf);
      itsSolvers->writeAll (buf);
      readAllWorkers (true, true);
      // Iterate through all steps and execute them.
      step.visit (*this);
    }
  }
   
  void MasterControl::quit()
  {
    // Send an end command.
    LOFAR::BlobString buf;
    MWBlobOut out(buf, -1, 0);
    out.finish();
    itsPrediffers->writeAll (buf);
    itsSolvers->writeAll (buf);
  }

  void MasterControl::visitSolve (const MWSolveStep& step)
  {
    // Send the solve step info to the prediffers and the solver.
    LOFAR::BlobString buf;
    {
      // Write command into buffer.
      MWBlobOut out (buf, MasterControl::Step, 0);
      out.blobStream() << step;
      out.finish();
    }
    itsPrediffers->writeAll (buf);
    itsSolvers->write (0, buf);
    // Read reply back from solver.
    itsSolvers->read (0, buf);
    // Read the reply back from each prediffer and send that to the solver.
    for (int i=0; i<itsPrediffers->size(); ++i) {
      itsPrediffers->read (i, buf);
      itsSolvers->write (0, buf);
    }
    // Iterate as long as the solver has not converged.
    bool converged = false;
    while (!converged) {
      // Tell prediffers to form the equations.
      buf.resize (0);
      {
	MWBlobOut out (buf, MasterControl::GetEq, 0);
	out.finish();
      }
      itsPrediffers->writeAll (buf);
      // Read the reply back from each prediffer and send that to the solver.
      for (int i=0; i<itsPrediffers->size(); ++i) {
	itsPrediffers->read (i, buf);
	itsSolvers->write (0, buf);
      }
      // Tell the solver to do the solve, get the solution and send that
      // to each prediffer.
      buf.resize (0);
      {
	MWBlobOut out (buf, MasterControl::Solve, 0);
	out.finish();
      }
      itsSolvers->write (0, buf);
      itsSolvers->read (0, buf);
      itsPrediffers->writeAll (buf);
      // Interpret the result to see if we have converged.
      MWBlobIn bin(buf);
      bin.blobStream() >> converged;
    }
  }

  void MasterControl::visitCorrect (const MWCorrectStep& step)
  {
    LOFAR::BlobString buf;
    MWBlobOut out (buf, MasterControl::Step, 0);
    out.blobStream() << step;
    out.finish();
    itsPrediffers->writeAll (buf);
    readAllWorkers (true, false);
  }

  void MasterControl::visitSubtract (const MWSubtractStep& step)
  {
    LOFAR::BlobString buf;
    MWBlobOut out (buf, MasterControl::Step, 0);
    out.blobStream() << step;
    out.finish();
    itsPrediffers->writeAll (buf);
    readAllWorkers (true, false);
  }

  void MasterControl::visitPredict (const MWPredictStep& step)
  {
    LOFAR::BlobString buf;
    MWBlobOut out (buf, MasterControl::Step, 0);
    out.blobStream() << step;
    out.finish();
    itsPrediffers->writeAll (buf);
    readAllWorkers (true, false);
  }

  void MasterControl::readAllWorkers (bool prediffers, bool solvers)
  {
    LOFAR::BlobString buf;
    if (prediffers) {
      for (int i=0; i<itsPrediffers->size(); ++i) {
        itsPrediffers->read (i, buf);
      }    
    }
    if (solvers) {
      for (int i=0; i<itsSolvers->size(); ++i) {
        itsSolvers->read (i, buf);
      }    
    }
  }

}} // end namespaces
