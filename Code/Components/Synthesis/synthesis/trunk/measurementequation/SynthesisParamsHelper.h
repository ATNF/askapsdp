/// @file
/// @brief Helper functions for dealing with Params for synthesis
///
/// Adds some useful functions specific to synthesis
/// @todo Function to output nicely formatted axes
/// @todo Functions to read/write images
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
#ifndef SYNSYNTHESISPARAMSHELPER_H_
#define SYNSYNTHESISPARAMSHELPER_H_

#include <casa/Arrays/Array.h>

#include <fitting/Params.h>

#include <APS/ParameterSet.h>
#include <images/Images/TempImage.h>
#include <images/Images/ImageInterface.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief populate scimath parameters from a LOFAR Parset object
    /// @details One needs often needs a possibility to populate 
    /// scimath::Params class from a Parset file (e.g. to load 
    /// initial gains from an external file). A number of add methods
    /// collected in this class happen to be image-specific. This is
    /// a generic method, which just copies all numeric fields
    /// @param[in] params a reference to scimath parameter object, where the
    /// parameters from parset file will be added
    /// @param[in] parset a const reference to a parset object
    /// @return a reference to params passed as an input (for chaining)
    scimath::Params& operator<<(scimath::Params &params, const LOFAR::ACC::APS::ParameterSet &parset);
  
    /// @brief Helper functions for synthesis processing using Params
    /// @ingroup measurementequation
    class SynthesisParamsHelper 
    {
      public:
        
        /// @brief set up images according to the parset file
		/// @param[in] params Images to be created here
		/// @param[in] parset a parset object to read the parameters from
		/// @note (MV)This method is probably a duplication of the one of 
		/// add methods - needs to be cleared
		/// (MV, dec 2008) not any more. With faceting it is handy to have a separate method
		static void setUpImages(const askap::scimath::Params::ShPtr& params, const LOFAR::ACC::APS::ParameterSet &parset);
		
		/// @brief load images according to the parset file
		/// @details This method is somewhat analogous to setUpImages, but it loads the images
		/// from the dist instead of setting them up from the scratch. Encapsulation of all loading
		/// of multiple images in a single method is required to provide a seamless handling of
		/// the faceted image.
		/// @param[in] params Images to be created here
		/// @param[in] parset a parset object to read the parameters from		
		static void loadImages(const askap::scimath::Params::ShPtr& params, const LOFAR::ACC::APS::ParameterSet &parset);
        
        /// @brief load component-related parameters from a parset file
        /// @details Parameter layout is different in scimath::Params and
        /// parset files for some reason. Typically a source is defined with
        /// parameters like flux.i.name, direction.ra.name, ... within the
        /// scimath::Params, but in the parset file the names of the parameters
        /// are sources.name.flux.i, sources.name.direction.ra, etc). This
        /// method translates the parameter names and copies the values accross.
        /// @param[in] params a shared pointer to the parameter container
        /// @param[in] parset a parset object to read the data from
        /// @param[in] srcName name of the source
        /// @param[in] baseKey a prefix added to parset parameter names (default
        /// is "sources.", wich matches the current layout of the parset file)
        static void copyComponent(const askap::scimath::Params::ShPtr &params,
                 const LOFAR::ACC::APS::ParameterSet &parset, 
                 const std::string &srcName, const std::string &baseKey = "sources.");
        
        /// @brief check whether parameter list defines at least one component
        /// @details Parameter lists can have a mixture of components and
        /// images defined. This method checks whether the given parameter
        /// list defines at least one component.
        /// @param[in] params a shared pointer to the parameter container
        /// @return true, if at least one component is defined
        static bool hasComponent(const askap::scimath::Params::ShPtr &params);
       
        /// @brief check whether parameter list defines at least one image
        /// @details Parameter lists can have a mixture of components and
        /// images defined. This method checks whether the given parameter
        /// list defines at least one image.
        /// @param[in] params a shared pointer to the parameter container
        /// @return true, if at least one image is defined
        static bool hasImage(const askap::scimath::Params::ShPtr &params);
        
      
        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param direction Strings containing [ra, dec, frame]
        /// @param cellsize Cellsize as a string e.g. [12arcsec, 12arcsec]
        /// @param shape Number of pixels in RA and DEC e.g. [256, 256]
        /// @param freqmin Minimum frequency (Hz)
        /// @param freqmax Maximum frequency (Hz)
        /// @param nchan Number of spectral channels
        static void add(askap::scimath::Params& ip, const string& name, 
          const vector<string>& direction, 
          const vector<string>& cellsize, 
          const vector<int>& shape,
          const double freqmin, const double freqmax, const int nchan);

        /// @brief Add a parameter as a faceted image
        /// @param[in] ip Parameters
        /// @param[in] name Name of parameter
        /// @param[in] direction Strings containing [ra, dec, frame] (common tangent point)
        /// @param[in] cellsize Cellsize as a string e.g. [12arcsec, 12arcsec]
        /// @param[in] shape Number of pixels in RA and DEC for each facet e.g. [256, 256]
        /// @param[in] freqmin Minimum frequency (Hz)
        /// @param[in] freqmax Maximum frequency (Hz)
        /// @param[in] nchan Number of spectral channels
        /// @param[in] nfacets Number of facets in each axis (assumed the same for both axes)
        /// @param[in] facetstep Offset in pixels between facet centres (equal to shape to
        ///            have no overlap between adjacent facets), assumed the same for both axes
        static void add(askap::scimath::Params& ip, const string& name, 
          const vector<string>& direction, 
          const vector<string>& cellsize, 
          const vector<int>& shape,
          const double freqmin, const double freqmax, const int nchan,
          const int nfacets, const int facetstep);
          
        /// @brief Add a set of parameters from a parset
        /// @param ip Parameters
        /// @param parset ParameterSet
        /// @param baseKey basekey for parameters e.g. "Images."
        static void add(askap::scimath::Params& ip,
          const LOFAR::ACC::APS::ParameterSet& parset,
          const std::string& baseKey);
          
        /// @brief Add a parameter as an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Name of image file
        static void add(askap::scimath::Params& ip, const string& name, 
          const string& image);
          
        /// @brief Get a parameter from a CASA image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param imagename Name of image file
        static void getFromCasaImage(askap::scimath::Params& ip, const string& name,
          const string& imagename);
          
        /// @brief Get parameters corresponding to all facets from a CASA image
        /// @param[in] ip Parameters
        /// @param[in] name Base name of the parameter (.facet.x.y will be added)
        /// @param[in] fileName Base name of the image file (.facet.x.y will be added)
        /// @param[in] nfacets Number of facets on each axis (assumed the same for both axes)
        static void getMultiFacetImage(askap::scimath::Params &ip, const string &name,
           const string &fileName, const int nfacets);
        
        /// @brief Save a parameter as a CASA image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param imagename Name of image file
        static void saveAsCasaImage(const askap::scimath::Params& ip, const string& name,
          const string& imagename);
          
        /// @brief save a 2D array as a CASA image
        /// @details This method is intended to be used largely for debugging. To save image from
        /// parameter class use another saveAsCasaImage method
        /// @param[in] imagename name of the output image file
        /// @param[in] arr input array
        static void saveAsCasaImage(const std::string &imagename, const casa::Array<casa::Float> &arr);
        
        /// @brief Copy a parameter to a CASA TempImage
        /// Note that this will be a reference if possible
        /// @param ip Parameters
        /// @param name Name of parameter
        static boost::shared_ptr<casa::TempImage<float> > 
          tempImage(const askap::scimath::Params& ip, 
          const string& name);
       
        /// @brief Create a coordinate system for a parameter
        /// @param ip Parameters
        /// @param name Name of parameter
        static casa::CoordinateSystem 
          coordinateSystem(const askap::scimath::Params& ip, 
          const string& name);
       
        /// @brief Create a direction coordinate for a parameter
        /// @param ip Parameters
        /// @param name Name of parameter
        static casa::DirectionCoordinate 
          directionCoordinate(const askap::scimath::Params& ip, 
          const string& name);
       
        /// @brief Update a parameter from an image
        /// @param ip Parameters
        /// @param name Name of parameter
        /// @param image Image to be drawn from 
        static void update(askap::scimath::Params& ip, const string& name, 
          const casa::ImageInterface<float>& image);
        
        /// @brief a helper template method to check whether the element is
        /// present in a container.
        /// @details This method is used to make the code more readable. It is very
        /// generic and can be moved to Base if needed elsewhere.
        /// @param[in] cont a container (stl)
        /// @param[in] val value to check
        /// @return true if an element equal to val is present in the container
        template<typename C, typename V>
        static bool hasValue(const C &cont, const V &val)
           { return cont.find(val) != cont.end(); }        
        
        /// @brief A helper method to parse strings of quantities
        /// @details Many parameters in parset file are given as quantities or
        /// vectors of quantities, e.g. [8.0arcsec,8.0arcsec]. This method allows
        /// to parse vector of strings corresponding to such parameter and return
        /// a vector of double values in the required units.
        /// @param[in] strval input vector of strings
        /// @param[in] unit required units (given as a string)
        /// @return vector of doubles with converted values
        static std::vector<double> convertQuantity(const std::vector<std::string> &strval,
                       const std::string &unit);

        /// @brief A helper method to parse string of quantities
        /// @details Many parameters in parset file are given as quantities or
        /// vectors of quantities, e.g. 8.0arcsec. This method allows
        /// to parse a single string corresponding to such a parameter and return
        /// a double value converted to the requested units.
        /// @param[in] strval input string
        /// @param[in] unit required units (given as a string)
        /// @return converted value
        static double convertQuantity(const std::string &strval,
                       const std::string &unit);                       
                               
        /// @brief A helper method to build a list of faceted images
        /// @details All multi-facet images are split between a number of 
        /// parameters named like "image.i.fieldname.facet.0.0". Single
        /// facet images correspond to parameters named like "image.i.fieldname".
        /// This method reads a supplied vector of names (may be either all names
        /// or just free parameters extracted from Params object) and builds a map
        /// of the image name (up to and including fieldname) and the number of
        /// facets. It also does the necessary checks that all required facets are
        /// defined and throws an exception if it is not the case.
        /// @param[in] names parameter names to work with
        /// @param[out] facetmap a map of (possibly truncated names) and the number of facets
        /// @note 1. facetmap.size()<=names.size() after the call to this method
        /// 2. This method just adds the content to the facet map without erasing the
        /// existing information.
        static void listFacets(const std::vector<std::string> &names,
                               std::map<std::string, int> &facetmap);

        /// @brief A helper method to form a parameter name representing a facet
        /// @details All multi-facet images are split between a number of parameters
        /// named like "image.i.fieldname.facet.0.0". This method forms a full string
        /// name from the prefix name and two integer numbers (this operation is required
        /// in a few places throughout the code).
        /// @param[in] prefixName the name before ".facet.x.y"
        /// @param[in] xFacet the first facet index
        /// @param[in] yFacet the second facet index
        /// @return the full parameter name corresponding to the given facet 
        static std::string facetParamName(const std::string &prefixName, int xFacet,
                   int yFacet);
    };

  }
}
#endif
