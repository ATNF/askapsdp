/// @file Quality.h
///
/// Quality: Encapsulate quality of a solution
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHQUALITY_H_
#define SCIMATHQUALITY_H_

#include <iostream>
#include <string>

namespace conrad
{
  namespace scimath
  {
    /// Encapsulate the quality of a solutin
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
