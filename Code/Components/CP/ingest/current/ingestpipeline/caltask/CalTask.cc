/// @file CalTask.cc
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "CalTask.h"

// Include package level header file
#include "askap_cpingest.h"

// CASA includes
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "utils/PolConverter.h"

// boost includes
//#include "boost/scoped_ptr.hpp"

// Local package includes
#include "ingestpipeline/datadef/VisChunk.h"

ASKAP_LOGGER(logger, ".CalTask");

using namespace askap;
using namespace askap::cp;

/// @brief constructor
/// @details Initialise calibration task by passing parameters
/// coded in the parset
/// @param[in] parset parameters
CalTask::CalTask(const LOFAR::ParameterSet& parset) :
    itsParset(parset), itsGainsParset(parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    if (itsParset.isDefined("gainsfile")) {
        itsGainsParset = LOFAR::ParameterSet(itsParset.getString("gainsfile"));
    }
}

/// @brief destructor
CalTask::~CalTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
}

/// @brief main method to apply calibration
/// @details It modifies the visibility chunk in situ by applying
/// current calibration
/// @param[in] chunk shared pointer to visibility chunk
void CalTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");
    ASKAPDEBUGASSERT(chunk);
    const casa::Vector<casa::Stokes::StokesTypes> &stokes = chunk->stokes();
    ASKAPCHECK(scimath::PolConverter::isLinear(stokes), "Calibration task requires Linear polarisation!");
    ASKAPDEBUGASSERT(stokes.nelements() > 0);
    // form indices to account for possibility of incomplete polarisation vectors
    casa::Vector<casa::uInt> polIndices(stokes.nelements());
    for (casa::uInt pol = 0; pol<polIndices.nelements(); ++pol) {
         ASKAPCHECK(scimath::PolConverter::isValid(stokes[pol]), "Unrecognised polarisation type "<<stokes[pol]<<" is found");
	 polIndices[pol] = scimath::PolConverter::getIndex(stokes[pol]);
	 ASKAPDEBUGASSERT(polIndices[pol]<4);
	 ASKAPDEBUGASSERT(polIndices[pol]>=0);
    }
    //

    casa::Matrix<casa::Complex> matr(4,4);
    casa::Matrix<casa::Complex> reciprocal;
    casa::Vector<casa::Complex> calibratedVector(polIndices.nelements());
    for (casa::uInt row=0; row<chunk->nRow(); ++row) {
         fillMuellerMatrix(matr, chunk->antenna1()[row], chunk->antenna2()[row],
	        chunk->beam1()[row],chunk->beam2()[row]);
         casa::Complex det = 0.;
	 casa::invertSymPosDef(reciprocal, det, matr);
	 const float tolerance = 1e-5;
         if (casa::abs(det)<tolerance)  {
	     // throw an exception for now, but will probably do an intelligent flagging in the future
	     ASKAPTHROW(AskapError, "Unable to apply gains, determinate too close to 0. D="<<casa::abs(det));
	 }
	 casa::Matrix<casa::Complex> thisRow = chunk->visibility().yzPlane(row);
	 // current gains are not frequency-dependent. Same matrix can be applied to all channels
         for (casa::uInt chan = 0; chan<chunk->nChannel();++chan) {
	      ASKAPDEBUGASSERT(chan < thisRow.nrow());
	      casa::Vector<casa::Complex> polVector = thisRow.row(chan);
	      ASKAPDEBUGASSERT(polVector.nelements() == polIndices.nelements());
              calibratedVector.set(0.);
	      for (casa::uInt pol1=0; pol1<polIndices.nelements();++pol1) {
		   for (casa::uInt pol2 = 0; pol2<polIndices.nelements(); ++pol2) {
	                calibratedVector[pol1] += reciprocal(polIndices[pol1],polIndices[pol2]) * polVector[pol2];
		   }
	      }
	      polVector = calibratedVector;
	 }
    }
}

/// @brief obtain gain for a given antenna/beam/pol
/// @details This is a helper method to separate extraction of
/// the values from the parset file from the actual calculation.
/// Based on the given number of antenna, beam and polarisation
/// (X or Y coded as 0 and 1), it returns the value of the 
/// parallel-hand gain (i.e. Gxx or Gyy)
/// @param[in] ant 0-based antenna id
/// @param[in] beam 0-based beam id
/// @param[in] pol Either 0 for XX or 1 for YY
casa::Complex CalTask::getGain(casa::uInt ant, casa::uInt beam, casa::uInt pol) const
{
  std::string name("gain.");
  if (pol == 0) {
      name += "g11.";
  } else if (pol == 1) {
      name += "g22.";
  } else {
     ASKAPTHROW(AskapError, "Polarisation index is supposed to be either 0 or 1, you have pol="<<pol);
  }
  name += askap::utility::toString<casa::uInt>(ant) + "." + askap::utility::toString<casa::uInt>(beam);
  return readComplex(name);
}

/// @brief helper method to load complex parameter
/// @details It reads the value from itsGainsParset and
/// forms a complex number.
/// @param[in] name parameter name
/// @return complex number
casa::Complex CalTask::readComplex(const std::string &name) const 
{
  casa::Vector<float> val = itsGainsParset.getFloatVector(name);
  ASKAPCHECK(val.nelements() > 0, "Expect at least one element for "<<name<<" gain parameter");
  if (val.nelements() == 1) {
      return val[0];
  }
  ASKAPCHECK(val.nelements() == 2, "Expect either one or two elements to define complex value, you have: "<<val);
  return casa::Complex(val[0],val[1]);
}

/// @brief fill Mueller matrix
/// @details This method forms the measurement equation
/// defined by Mueller matrix for a given baseline and beams.
/// The method is implemented in a general way, so it supports
/// correlations corresponding to a different beam. However, in
/// practice beam1 and beam2 are likely to be the same most of 
/// the time.
/// @param[in] matr Mueler matrix to fill (must already be resized to 4x4)
/// @param[in] ant1 first antenna id (0-based)
/// @param[in] ant2 second antenna id (0-based)
/// @param[in] beam1 beam id at the first antenna (0-based)
/// @param[in] beam2 beam id at the second antenna (0-based)
void CalTask::fillMuellerMatrix(casa::Matrix<casa::Complex> &matr, casa::uInt ant1, 
         casa::uInt ant2, casa::uInt beam1, casa::uInt beam2) const
{
  ASKAPDEBUGASSERT((matr.nrow() == 4) && (matr.ncolumn() == 4));
  matr.set(0.);
  // without cross-pols Mueller matrix is just diagonal
  for (casa::uInt pol1 = 0, cnt=0; pol1 < 2; ++pol1) {
       for (casa::uInt pol2 = 0; pol2 < 2; ++pol2,++cnt) {
	    ASKAPDEBUGASSERT(cnt<4);
            matr(cnt,cnt) = getGain(ant1,beam1,pol1) * conj(getGain(ant2,beam2,pol2));
       }
  }
}
