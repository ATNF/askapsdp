/// @file fitting/Axes.h
/// @brief Axes: Represent a set of axes - names and extrema.
///
/// The axes are used to describe a multidimensional
/// parameter. For example,
/// @code
/// Axes imageAxes;
/// double arcsec=casa::C::pi/(3600.0*180.0);
/// double cell=5.0*arcsec;
/// imageAxes.add("RA", -double(npix)*cell/2.0, double(npix)*cell/2.0);
/// imageAxes.add("DEC", -double(npix)*cell/2.0, double(npix)*cell/2.0);
/// imageAxes.add("FREQ", 1e9, 1.2e9);
/// @endcode
///
/// @todo Add tabulated axes
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHAXES_H_
#define SCIMATHAXES_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

namespace conrad
{
  
  namespace scimath
  {
    /// @brief Describe axes of parameters
    ///
    /// An axis has a name and start and end values (doubles)
    /// An Axes is a ordered set of Axises.
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
      
      /// Do it have this axis?
      /// @param name Name of axis
      /// @return True if name exists
      bool has(const std::string& name) const;

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
      /// @param os an ostream
      /// @param axes the Axes instance
      friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& os, 
                                            Axes& axes);
      
    private:
      std::vector<std::string> itsNames;
      std::vector<double> itsStart;
      std::vector<double> itsEnd;
    };

    /// Use Domain as a synonym (for the moment).
    typedef Axes Domain;

  };
};
#endif
