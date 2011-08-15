/// @file
/// @brief An interface for accessing calibration solutions for reading and writing.
/// @details This interface is used to access calibration parameters for both
/// reading and writing. It is derived from read-only version of the interface.
/// Various implementations are possible, i.e. parset-based, 
/// table-based and working via database ice service.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef I_CAL_SOLUTION_ACCESSOR_H
#define I_CAL_SOLUTION_ACCESSOR_H

// own includes
#include <calibaccess/ICalSolutionConstAccessor.h>

// casa includes
#include <measures/Measures/Stokes.h>

namespace askap {
namespace accessors {

/// @brief An interface for read-write access of calibration solutions.
/// @details This interface is used to access calibration parameters
/// read-only. A writable version of the interface is derived from this
/// class. Various implementations are possible, i.e. parset-based, 
/// table-based and working via database ice service.
/// @ingroup calibaccess
struct ICalSolutionAccessor : virtual public ICalSolutionConstAccessor {

   // virtual methods which need to be overridden in concrete 
   // implementation classes
   
   /// @brief set gains (J-Jones)
   /// @details This method writes parallel-hand gains for both 
   /// polarisations (corresponding to XX and YY)
   /// @param[in] index ant/beam index 
   /// @param[in] gains JonesJTerm object with gains and validity flags
   virtual void setGain(const JonesIndex &index, const JonesJTerm &gains) = 0;
   
   /// @brief set leakages (D-Jones)
   /// @details This method writes cross-pol leakages  
   /// (corresponding to XY and YX)
   /// @param[in] index ant/beam index 
   /// @param[in] leakages JonesDTerm object with leakages and validity flags
   virtual void setLeakage(const JonesIndex &index, const JonesDTerm &leakages) = 0;
   
   /// @brief set gains for a single bandpass channel
   /// @details This method writes parallel-hand gains corresponding to a single
   /// spectral channel (i.e. one bandpass element).
   /// @param[in] index ant/beam index 
   /// @param[in] bp JonesJTerm object with gains for the given channel and validity flags
   /// @param[in] chan spectral channel
   /// @note We may add later variants of this method assuming that the bandpass is
   /// approximated somehow, e.g. by a polynomial. For simplicity, for now we deal with 
   /// gains set explicitly for each channel.
   virtual void setBandpass(const JonesIndex &index, const JonesJTerm &bp, const casa::uInt chan) = 0;
   
   // additional (non-virtual) helper methods to simplify access (working via virtual methods)

   /// @brief set a single element of the Jones matrix (i.e gains or leakages)
   /// @details This method simplifies writing both gains and leakages solution. It reads the current
   /// gains and leakages and then replaces one element with the given value setting the validity flag.
   /// The stokes parameter is controlling which element of the Jones matrix is replaced. We assume that
   /// only linear polarisation products are used (an exception is thrown if it is not the case). 
   /// XX and YY represent parallel-hand gains (two elements of JonesJTerm) and XY and YX represent
   /// cross-pol leakages (two elements of JonesDTerm).
   /// @param[in] index ant/beam index
   /// @param[in] stokes what element to update (choose from XX,XY,YX and YY)
   /// @param[in] elem value to set
   void setJonesElement(const JonesIndex &index, const casa::Stokes::StokesTypes stokes, const casa::Complex &elem);
   
   /// @brief set a single element of the Jones matrix (i.e. gains or leakages)
   /// @details This version of the method gets explicitly defined antenna and beam indices.
   /// @param[in] ant ant index
   /// @param[in] beam beam index
   /// @param[in] stokes what element to update (choose from XX,XY,YX and YY)
   /// @param[in] elem value to set
   void setJonesElement(const casa::uInt ant, const casa::uInt beam, const casa::Stokes::StokesTypes stokes, const casa::Complex &elem);
   
   /// @brief set a single element of bandpass
   /// @details This method simplifies writing bandpass solution. It reads the current frequency-dependent
   /// gains for the given channel and then replaces one of the elements with the given value setting
   /// the validity flag. We assume that only linear polarisation frame is to be used with this method
   /// (an exception is thrown if it is not the case). At the moment, no polarisation leakage bandpass
   /// is supported (although it may be changed in the future). Therefore, only XX and YY polarisation indices
   /// are allowed here.
   /// @param[in] index ant/beam index
   /// @param[in] stokes what element to update (choose either XX or YY)
   /// @param[in] chan spectral channel of interest
   /// @param[in] elem value to set
   void setBandpassElement(const JonesIndex &index, const casa::Stokes::StokesTypes stokes, const casa::uInt chan, 
                           const casa::Complex &elem);
   
   /// @brief set a single element of bandpass
   /// @details This version of the method uses explicitly defined antenna and beam indices.
   /// @param[in] ant ant index
   /// @param[in] beam beam index
   /// @param[in] stokes what element to update (choose either XX or YY)
   /// @param[in] chan spectral channel of interest
   /// @param[in] elem value to set
   void setBandpassElement(const casa::uInt ant, const casa::uInt beam, const casa::Stokes::StokesTypes stokes, 
                           const casa::uInt chan, const casa::Complex &elem);
   
   /// @brief shared pointer definition
   typedef boost::shared_ptr<ICalSolutionAccessor> ShPtr;
   
};

} // namespace accessors
} // namespace askap

#endif // #ifndef I_CAL_SOLUTION_ACCESSOR_H

