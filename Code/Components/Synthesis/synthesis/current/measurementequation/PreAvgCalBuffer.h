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

#ifndef SYNTHESIS_PREAVGCALBUFFER_H
#define SYNTHESIS_PREAVGCALBUFFER_H

#include <dataaccess/DataAccessorAdapter.h>
#include <dataaccess/IConstDataAccessor.h>
#include <measurementequation/IMeasurementEquation.h>

#include <boost/shared_ptr.hpp>

namespace askap {

namespace synthesis {

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
/// @note At the moment all frequency channels are summed up together. Later we may want
/// to implement a partial averaging in frequency.
/// @ingroup measurementequation
class PreAvgCalBuffer : public accessors::DataAccessorAdapter {
public:
   /// @brief default constructor
   /// @details preaveraging is initialised based on the first encountered accessor
   PreAvgCalBuffer();
   
   /// @brief constructor with explicit averaging parameters
   /// @details This version of the constructor explicitly defines the number of 
   /// antennas and beams to initialise the buffer appropriately.
   /// @param[in] nAnt number of antennas, indices are expected to run from 0 to nAnt-1
   /// @param[in] nBeam number of beams, indices are expected to run from 0 to nBeam-1
   PreAvgCalBuffer(casa::uInt nAnt, casa::uInt nBeam);
   
   /// @brief initialise accumulation via an accessor
   /// @details This method resets the buffers and sets the shape using the given accessor
   /// as a template.
   /// @param[in] acc template accessor
   void initialise(const IConstDataAccessor &acc);
   
   /// @brief initialise accumulation explicitly
   /// @details This method resets the buffers and sets the shape to accommodate the given
   /// number of antennas and beams (i.e. the buffer size is nBeams*nAnt*(nAnt-1)/2)
   /// @param[in] nAnt number of antennas, indices are expected to run from 0 to nAnt-1
   /// @param[in] nBeam number of beams, indices are expected to run from 0 to nBeam-1
   void initialise(casa::uInt nAnt, casa::uInt nBeam);
   
   // implemented accessor methods
   
   /// The number of rows in this chunk
   /// @return the number of rows in this chunk
   virtual casa::uInt nRow() const throw();
  	
   /// The number of spectral channels (equal for all rows)
   /// @return the number of spectral channels
   virtual casa::uInt nChannel() const throw();

   /// The number of polarization products (equal for all rows)
   /// @return the number of polarization products (can be 1,2 or 4)
   virtual casa::uInt nPol() const throw();

   /// First antenna IDs for all rows
   /// @return a vector with IDs of the first antenna corresponding
   /// to each visibility (one for each row)
   virtual const casa::Vector<casa::uInt>& antenna1() const;

   /// Second antenna IDs for all rows
   /// @return a vector with IDs of the second antenna corresponding
   /// to each visibility (one for each row)
   virtual const casa::Vector<casa::uInt>& antenna2() const;
  
   /// First feed IDs for all rows
   /// @return a vector with IDs of the first feed corresponding
   /// to each visibility (one for each row)
   virtual const casa::Vector<casa::uInt>& feed1() const;

   /// Second feed IDs for all rows
   /// @return a vector with IDs of the second feed corresponding
   /// to each visibility (one for each row)
   virtual const casa::Vector<casa::uInt>& feed2() const;

   /// Cube of flags corresponding to the output of visibility() 
   /// @return a reference to nRow x nChannel x nPol cube with flag 
   ///         information. If True, the corresponding element is flagged bad.
   virtual const casa::Cube<casa::Bool>& flag() const;

   // access to accumulated statistics
   
   /// @brief obtain weighted sum of model amplitudes
   /// @return nRow x nChannel x nPol cube with sums of 
   /// absolute values of complex visibilities (real-valued)
   const casa::Cube<casa::Float>& sumModelAmps() const;
   
   /// @brief obtain weighted sum of products of model and measured visibilities
   /// @return nRow x nChannel x nPol cube with weighted sums of 
   /// products between measured and conjugated model visibilities (complex-valued)
   const casa::Cube<casa::Complex>& sumVisProducts() const;
   
   // the actual summing in of an accessor with data
   
   /// @brief process one accessor
   /// @details This method processes the given accessor and updates the internal 
   /// buffers. The measurement equation is used to calculate model visibilities 
   /// corresponding to measured visibilities.
   /// @param[in] acc input accessor with measured data
   /// @param[in] me shared pointer to the measurement equation
   /// @note only predict method of the measurement equation is used.
   void accumulate(const IConstDataAccessor &acc, const boost::shared_ptr<IMeasurementEquation const> &me);

protected:
   /// @brief helper method to find a match row in the buffer
   /// @details It goes over antenna and beam indices and finds a buffer row which 
   /// corresponds to the given indices.
   /// @param[in] ant1 index of the first antenna
   /// @param[in] ant2 index of the second antenna
   /// @param[in] beam beam index
   /// @return row number in the buffer corresponding to the given (ant1,ant2,beam) or -1 if 
   /// there is no match
   int findMatch(casa::uInt ant1, casa::uInt ant2, casa::uInt beam); 
   
private:
   /// @brief indices of the first antenna for all rows
   casa::Vector<casa::uInt> itsAntenna1;   
   
   /// @brief indices of the second antenna for all rows
   casa::Vector<casa::uInt> itsAntenna2;   
   
   /// @brief indices of the beam for all rows
   /// @note beam cross-products are not supported here
   casa::Vector<casa::uInt> itsBeam;
   
   /// @brief flags for all rows, channels and polarisations
   casa::Cube<casa::Bool> itsFlag;
   
   /// @brief buffer for accumulated statistics
   /// @details nRow x nChannel x nPol cube with sums of 
   /// absolute values of complex visibilities (real-valued)
   casa::Cube<casa::Float> itsSumModelAmps;
   
   /// @brief buffer for accumulated statistics
   /// @return nRow x nChannel x nPol cube with weighted sums of 
   /// products between measured and conjugated model visibilities (complex-valued)
   casa::Cube<casa::Complex> itsSumVisProducts;
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef SYNTHESIS_PREAVGCALBUFFER_H

