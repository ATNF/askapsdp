/// @file
/// 
/// @brief This class is used inside the measurement equation object implementing
/// pre-averaging (or pre-summing to be exact) algorithm for calibration. Strictly
/// speaking it is not an adapter and it doesn't behave as an accessor. However, it
/// mimics the accessor interface, so we can reuse the existing code to a greater
/// extent. In addition, we can extend the code to a more complicated types of calibration 
/// later (i.e. with equations using more metadata). Current implementation is derived 
/// from DataAccessorAdapter just to speed up the development. None of the functionality
/// of this base class is used (except throwing exceptions if methods which are not
/// intended to be used are called). The plan is to always keep the DataAccessorAdapter
/// in the detached state.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <measurementequation/PreAvgCalBuffer.h>
#include <askap/AskapError.h>


using namespace askap;
using namespace askap::synthesis;

/// @brief default constructor
/// @details preaveraging is initialised based on the first encountered accessor
PreAvgCalBuffer::PreAvgCalBuffer() {}
   
/// @brief constructor with explicit averaging parameters
/// @details This version of the constructor explicitly defines the number of 
/// antennas and beams to initialise the buffer appropriately.
/// @param[in] nAnt number of antennas, indices are expected to run from 0 to nAnt-1
/// @param[in] nBeam number of beams, indices are expected to run from 0 to nBeam-1
PreAvgCalBuffer::PreAvgCalBuffer(casa::uInt nAnt, casa::uInt nBeam) : itsAntenna1(nBeam*nAnt*(nAnt-1)/2), 
      itsAntenna2(nBeam*nAnt*(nAnt-1)/2), itsBeam(nBeam*nAnt*(nAnt-1)/2), itsFlag(nBeam*nAnt*(nAnt-1)/2,1,4),
      itsSumModelAmps(nBeam*nAnt*(nAnt-1)/2,1,4), itsSumVisProducts(nBeam*nAnt*(nAnt-1)/2,1,4)
{
  initialise(nAnt,nBeam);
}      
   
/// @brief initialise accumulation via an accessor
/// @details This method resets the buffers and sets the shape using the given accessor
/// as a template.
/// @param[in] acc template accessor
void PreAvgCalBuffer::initialise(const IConstDataAccessor &acc)
{
  // resize buffers
  const casa::uInt numberOfRows = acc.nRow();
  const casa::uInt numberOfPol = acc.nPol();
  if (itsFlag.shape() != casa::IPosition(3,numberOfRows, 1, numberOfPol)) {
      // resizing buffers
      itsAntenna1.resize(numberOfRows);
      itsAntenna2.resize(numberOfRows);
      itsBeam.resize(numberOfRows);
      itsFlag.resize(numberOfRows, 1, numberOfPol);
      itsSumModelAmps.resize(numberOfRows, 1, numberOfPol);
      itsSumVisProducts.resize(numberOfRows, 1, numberOfPol);      
  }
  // initialise buffers
  itsAntenna1 = acc.antenna1();
  itsAntenna2 = acc.antenna2();
  itsBeam = acc.feed1();
  casa::uInt unusedBeamId = casa::max(itsBeam)*10;
  const casa::Vector<casa::uInt>& feed2 = acc.feed2();
  for (casa::uInt row=0; row<numberOfRows; ++row) {
       if (itsBeam[row] != feed2[row]) {
           // so it is kept flagged (not very tidy way of doing the check,
           // we can introduce a separate vector to track this unlikely condition)
           itsBeam[row] = unusedBeamId;
       }
  }
  // all elements are flagged until at least something is averaged in
  itsFlag.set(true); 
  itsSumModelAmps.set(0.);
  itsSumVisProducts.set(casa::Complex(0.,0.));  
}
   
/// @brief initialise accumulation explicitly
/// @details This method resets the buffers and sets the shape to accommodate the given
/// number of antennas and beams (i.e. the buffer size is nBeams*nAnt*(nAnt-1)/2)
/// @param[in] nAnt number of antennas, indices are expected to run from 0 to nAnt-1
/// @param[in] nBeam number of beams, indices are expected to run from 0 to nBeam-1
void PreAvgCalBuffer::initialise(casa::uInt nAnt, casa::uInt nBeam)
{
  const casa::uInt numberOfRows = nBeam*nAnt*(nAnt-1)/2;
  if (itsFlag.shape() != casa::IPosition(3,int(numberOfRows),1,4)) {
     // resizing buffers
     itsAntenna1.resize(numberOfRows);
     itsAntenna2.resize(numberOfRows);
     itsBeam.resize(numberOfRows);
     itsFlag.resize(numberOfRows,1,4);
     itsSumModelAmps.resize(numberOfRows,1,4);
     itsSumVisProducts.resize(numberOfRows,1,4);
  }
  // initialising buffers
  itsFlag.set(true); // everything is bad, unless at least one sample is summed into the buffer
  itsSumModelAmps.set(0.);
  itsSumVisProducts.set(casa::Complex(0.,0.));
  
  for (casa::uInt beam=0,row=0; beam<nBeam; ++beam) {
       for (casa::uInt ant1=0; ant1<nAnt; ++ant1) {
            for (casa::uInt ant2=0; ant2<ant1; ++ant2,++row) {
                 ASKAPDEBUGASSERT(row<numberOfRows);
                 itsAntenna1[row] = ant1;
                 itsAntenna2[row] = ant2;
                 itsBeam[row] = beam;
            }
       }
  }
}
   
// implemented accessor methods
   
/// The number of rows in this chunk
/// @return the number of rows in this chunk
casa::uInt PreAvgCalBuffer::nRow() const throw()
{
  return itsBeam.nelements();
}
  	
/// The number of spectral channels (equal for all rows)
/// @return the number of spectral channels
casa::uInt PreAvgCalBuffer::nChannel() const throw()
{
  // for now, only averaging into 1 spectral channel is supported
  return 1;
}

/// The number of polarization products (equal for all rows)
/// @return the number of polarization products (can be 1,2 or 4)
casa::uInt PreAvgCalBuffer::nPol() const throw()
{
  return itsFlag.nplane();
}

/// First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& PreAvgCalBuffer::antenna1() const
{
  return itsAntenna1;
}

/// Second antenna IDs for all rows
/// @return a vector with IDs of the second antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& PreAvgCalBuffer::antenna2() const
{
  return itsAntenna2;
}
  
/// First feed IDs for all rows
/// @return a vector with IDs of the first feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& PreAvgCalBuffer::feed1() const
{
  return itsBeam;
}

/// Second feed IDs for all rows
/// @return a vector with IDs of the second feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& PreAvgCalBuffer::feed2() const
{
  return itsBeam;
}

/// Cube of flags corresponding to the output of visibility() 
/// @return a reference to nRow x nChannel x nPol cube with flag 
///         information. If True, the corresponding element is flagged bad.
const casa::Cube<casa::Bool>& PreAvgCalBuffer::flag() const
{
  return itsFlag;
}

// access to accumulated statistics
   
/// @brief obtain weighted sum of model amplitudes
/// @return nRow x nChannel x nPol cube with sums of 
/// absolute values of complex visibilities (real-valued)
const casa::Cube<casa::Float>& PreAvgCalBuffer::sumModelAmps() const
{
  return itsSumModelAmps;
}
   
/// @brief obtain weighted sum of products of model and measured visibilities
/// @return nRow x nChannel x nPol cube with weighted sums of 
/// products between measured and conjugated model visibilities (complex-valued)
const casa::Cube<casa::Complex>& PreAvgCalBuffer::sumVisProducts() const
{
  return itsSumVisProducts;
}
