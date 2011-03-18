/// @file Quality.h
///
/// Quality: Encapsulate quality of a solution
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHQUALITY_H_
#define SCIMATHQUALITY_H_

#include <iostream>
#include <string>

namespace askap
{
  namespace scimath
  {
    /// Encapsulate the quality of a solutin
    /// @ingroup fitting
    class Quality
    {
    public:
      /// Only constructor
      Quality();
      
      virtual ~Quality();
      
      /// Set rank of equations
      /// @param rank Rank to be set 
      void setRank(const unsigned int rank) {itsRank=rank;};
      /// Get rank of equations
      unsigned int rank() const {return itsRank;};
      
      /// Set condition number of equations
      /// @param cond Condition number of equations
      void setCond(const double cond) {itsCond=cond;};
      
      /// Get condition number of equations
      double cond() const {return itsCond;};
      
      /// Set caller-defined info string
      /// @param info Caller-defined info string
      void setInfo(const std::string info) {itsInfo=info;}
      
      /// Get caller-defined info string
      const std::string& info() const {return itsInfo;};
      
      /// Set degrees of freedom of equations
      /// @param DOF degrees of freedom
      void setDOF(const unsigned int DOF) {itsDOF=DOF;};
      /// Get degrees of freedom of equations
      unsigned int DOF() const {return itsDOF;};
      
      /// Output quality
      /// @param os the outpus stream
      /// @param q the Quality instance
      friend std::ostream& operator<<(std::ostream& os, const Quality& q);
      
    private:
      /// Condition number
      double itsCond;
      /// Rank
      unsigned int itsRank;
      /// Degrees of freedom
      unsigned int itsDOF;
      /// Info string
      std::string itsInfo;
    };

  }
}
#endif                                            /*QUALITY_H_*/
