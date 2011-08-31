/// @file
///
/// @brief An interface for accessing calibration solutions for reading.
/// @details This interface is used to access calibration parameters
/// read-only. A writable version of the interface is derived from this
/// class. Various implementations are possible, i.e. parset-based, 
/// table-based and working via database ice service.
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
/// Based on the original version of this interface by Ben Humphreys <ben.humphreys@csiro.au>

#include <calibaccess/ICalSolutionConstAccessor.h>
#include <askap/AskapError.h>

namespace askap {

namespace accessors {

// just to keep compiler happy we define an empty virtual destructor
ICalSolutionConstAccessor::~ICalSolutionConstAccessor()
{
}

/// @brief obtain full 2x2 Jones Matrix taking all effects into account
/// @details This method returns resulting 2x2 matrix taking gain, leakage and
/// bandpass effects (for a given channel) into account. Invalid gains (and bandpass
/// values) are replaced by 1., invalid leakages are replaced by zeros. This method
/// calls gain, bandpass and leakage virtual methods
/// @param[in] index ant/beam index
/// @param[in] chan spectral channel of interest
/// @return 2x2 Jones matrix
/// @note The relation between leakage terms and Jones matrices matches 
/// the definition of Hamaker, Bregman & Sault. See their equation 
/// (14) for details. Our parameters d12 (corresponding to Stokes:XY) and
/// d21 (corresponding to Stokes::YX) correspond to d_{Ap} and d_{Aq} from
/// Hamaker, Bregman & Sault, respectively.
casa::SquareMatrix<casa::Complex, 2> ICalSolutionConstAccessor::jones(const JonesIndex &index, const casa::uInt chan) const
{
  casa::SquareMatrix<casa::Complex, 2> result(casa::SquareMatrix<casa::Complex, 2>::General);
  const JonesJTerm gTerm = gain(index);
  result(0,0) = gTerm.g1IsValid() ? gTerm.g1() : casa::Complex(1.,0.);
  result(1,1) = gTerm.g2IsValid() ? gTerm.g2() : casa::Complex(1.,0.);
  const JonesDTerm dTerm = leakage(index);

  result(0,1) = (dTerm.d12IsValid() ? dTerm.d12() : 0.) * result(1,1);
  result(1,0) = (dTerm.d21IsValid() ? -dTerm.d21() : 0.) * result(0,0);

  const JonesJTerm bpTerm = bandpass(index,chan);
  if (bpTerm.g1IsValid()) {
      result(0,0) *= bpTerm.g1();
      result(1,0) *= bpTerm.g1();
  }

  if (bpTerm.g2IsValid()) {
      result(0,1) *= bpTerm.g2();
      result(1,1) *= bpTerm.g2();
  }
  return result;
}
      
/// @brief obtain full 2x2 Jones Matrix taking all effects into account
/// @details This version of the method accepts antenna and beam indices explicitly and
/// does extra checks before calling the main method expressed via JonesIndex.
/// @param[in] ant antenna index
/// @param[in] beam beam index
/// @param[in] chan spectral channel of interest
/// @return 2x2 Jones matrix
casa::SquareMatrix<casa::Complex, 2> ICalSolutionConstAccessor::jones(const casa::uInt ant, 
                                     const casa::uInt beam, const casa::uInt chan) const
{
  ASKAPCHECK(chan < 16416, "Channel number is supposed to be less than 16416");
  return jones(JonesIndex(ant, beam), chan);
}
   
/// @brief obtain validity flag for the full 2x2 Jones Matrix
/// @details This method combines all validity flags for parameters used to compose Jones
/// matrix and returns true if all elements are valid and false if at least one constituent
/// is not valid
/// @param[in] index ant/beam index
/// @param[in] chan spectral channel of interest
/// @return true, if the matrix returned by jones(...) method called with the same parameters is
/// valid, false otherwise
bool ICalSolutionConstAccessor::jonesValid(const JonesIndex &index, const casa::uInt chan) const
{
  const JonesJTerm gTerm = gain(index);
  const JonesJTerm bpTerm = bandpass(index,chan);
  const JonesDTerm dTerm = leakage(index);
  return gTerm.g1IsValid() && gTerm.g2IsValid() && bpTerm.g1IsValid() && bpTerm.g2IsValid() &&
         dTerm.d12IsValid() && dTerm.d21IsValid();
}
   
/// @brief obtain validity flag for the full 2x2 Jones Matrix
/// @details This version of the method accepts antenna and beam indices explicitly and
/// does extra checks before calling the main method expressed via JonesIndex.
/// @param[in] ant antenna index
/// @param[in] beam beam index
/// @param[in] chan spectral channel of interest
/// @return true, if the matrix returned by jones(...) method called with the same parameters is
/// valid, false otherwise
bool ICalSolutionConstAccessor::jonesValid(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) const
{
  ASKAPCHECK(chan < 16416, "Channel number is supposed to be less than 16416");
  return jonesValid(JonesIndex(ant, beam), chan);
}

} // namespace accessors
} // namespace askap

