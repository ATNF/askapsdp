/// @file fitting/Axes.h
/// @brief Axes: Represent a set of axes - names and extrema.
///
/// The axes are used to describe a multidimensional
/// parameter. For example,
/// @code
/// Axes imageAxes;
/// double arcsec=casa::C::pi/(3600.0*180.0);
/// double cell=5.0*arcsec;
/// imageAxes.add("RA_LIN", -double(npix)*cell/2.0, double(npix)*cell/2.0);
/// imageAxes.add("DEC_LIN", -double(npix)*cell/2.0, double(npix)*cell/2.0);
/// imageAxes.add("FREQ", 1e9, 1.2e9);
/// @endcode
/// @note Direction axes are handled by casacore's DirectionCoordinate. The example above implies 
/// the linear axes, rather than sine projection.
///
/// @todo Add tabulated axes
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
#ifndef SCIMATHAXES_H_
#define SCIMATHAXES_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <measures/Measures/Stokes.h>
#include <casa/Arrays/Vector.h>


#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <boost/shared_ptr.hpp>

namespace askap
{
  
  namespace scimath
  {
    /// @brief Describe axes of parameters
    ///
    /// An axis has a name and start and end values (doubles)
    /// An Axes is a ordered set of Axises.
    /// @ingroup fitting
    class Axes
    {
    public:
      /// Make an empty set of axes
      Axes();
      
      /// Assignment operator
      Axes& operator=(const Axes& other);
      
      /// Copy constructor
      Axes(const Axes& other);
      
      ~Axes();
      
      /// Add an axis
      /// @param name Name of axis
      /// @param start Start value
      /// @param end End value
      void add(const std::string& name, const double start, const double end);

      /// @brief update an axis
      /// @details Sometimes it is handy to modify one axis only without resorting to 
      /// axis by axis copy. This method assigns new start and end values to a given
      /// axis. It is equivalent to add if the required axis doesn't exist.
      /// @param[in] name name of axis
      /// @param[in] start start value 
      /// @param[in] end end value
      void update(const std::string &name, const double start, const double end); 
      
      /// Do it have this axis?
      /// @param name Name of axis
      /// @return True if name exists
      bool has(const std::string& name) const;
      
      /// @brief check whether direction axis is present
      /// @return true if direction axis is defined
      inline bool hasDirection() const { return itsDirectionAxis;}

      /// Order of this axis
      /// @param name Name of axis
      /// @return sequence of axis (0 rel)
      int order(const std::string& name) const;
      
      /// Return the axis names
      const std::vector<std::string>& names() const;
      
      /// Return start value
      /// @param name Name of axis
      double start(const std::string& name) const;
      
      /// Return end value
      /// @param name Name of axis
      double end(const std::string& name) const;
      
      /// Return start values
      const std::vector<double>& start() const;
      
      /// Return end values
      const std::vector<double>& end() const;

      // methods enforcing interpretation for different types of axes

      /// @brief form vector of stokes enums from STOKES axis
      /// @return vector of stokes enums
      /// @note An axis names STOKES must be present, otherwise an exception will be thrown
      casa::Vector<casa::Stokes::StokesTypes> stokesAxis() const;
      
      /// @brief add STOKES axis formed from the vector of stokes enums
      /// @details This is a reverse operation to stokesAxis. If the STOKES axis
      /// already exists, the values are updated.
      /// @param[in] stokes a vector of stokes enums
      void addStokesAxis(const casa::Vector<casa::Stokes::StokesTypes> &stokes);      
      
      // operations with direction axis
      
      /// @brief extract parameters of the direction axis
      /// @return a const reference to casacore's DirectionCoordinate object
      const casa::DirectionCoordinate& directionAxis() const;
      
      /// @brief add direction axis
      /// @details This method is reverse to directionAxis. It adds or updates direction 
      /// coordinate. 
      /// @param[in] dc direction coordinate
      void addDirectionAxis(const casa::DirectionCoordinate &dc);
      
      /// Output to an ostream
      /// @param os an ostream
      /// @param axes the Axes instance
      friend std::ostream& operator<<(std::ostream& os, const Axes& axes);
      
      /// IO to a BlobStream
      /// @param os an ostream
      /// @param axes the Axes instance
      friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, 
                                            const Axes& axes); 

      /// IO from a BlobStream
      /// @param is an ostream
      /// @param axes the Axes instance
      friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, 
                                            Axes& axes);
      
    private:
      std::vector<std::string> itsNames;
      std::vector<double> itsStart;
      std::vector<double> itsEnd;
      
      /// @details Due to projections, we can't represent direction coordinates 
      /// in the linear way using just start and stop value (the easiest way to
      /// understand this is to consider what goes on near the poles). This 
      /// object defines the direction coordinate in the most general way. 
      /// @note Sine projection is currently always assumed
      boost::shared_ptr<casa::DirectionCoordinate> itsDirectionAxis;
    };

    /// Use Domain as a synonym (for the moment).
    typedef Axes Domain;

  };
};
#endif
