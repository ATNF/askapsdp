/// @file
///
/// Provides a simple way of defining a subimage of a FITS file.
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSISUTILITIES_SUBIMAGE_H_
#define ASKAP_ANALYSISUTILITIES_SUBIMAGE_H_

#include <string>
#include <vector>
#include <set>

#include <askapparallel/AskapParallel.h>

#include <Common/ParameterSet.h>

#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>
#include <duchamp/fitsHeader.hh>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

namespace askap {
namespace analysisutilities {

/// @ingroup analysisutilities
/// @brief A class to encapsulate a subimage definition
/// @details This class holds information to define the way the
/// full image should be subdivided amongst workers. It uses the
/// definitions from the parameter set and the image WCS to define
/// duchamp::Section objects for each desired subimage.
class SubimageDef {
    public:
        /// @brief Default constructor
        SubimageDef();
        /// @brief Constructor using parset.
        SubimageDef(const LOFAR::ParameterSet& parset);
        /// @brief Default destructor
        virtual ~SubimageDef() {};

        /// @brief Set up the definition for a given number of
        /// dimensions
        void define(const int numDim);

        /// @brief Set up the definition using a WCSLIB definition
        /// @details Define all the necessary variables within the
        /// SubimageDef class. The image (given by the parameter
        /// "image" in the parset) is to be split up according to the
        /// nsubx/y/z parameters, with overlaps in each direction
        /// given by the overlapx/y/z parameters (these are in
        /// pixels).
        ///
        /// The WCS parameters in wcs determine which axes are the x,
        /// y and z axes. The number of axes is also determined from
        /// the WCS parameter set.
        ///
        /// @param wcs The WCSLIB definition of the world coordinate
        /// system
        void define(const wcsprm *wcs);

        /// @brief Set up the definition for a FITS file.
        /// @details Define all the necessary variables within the
        /// SubimageDef class. The image (given by the parameter
        /// "image" in the parset) is to be split up according to the
        /// nsubx/y/z parameters, with overlaps in each direction
        /// given by the overlapx/y/z parameters (these are in
        /// pixels).
        ///
        /// This version is designed for FITS files. The Duchamp
        /// function duchamp::FitsHeader::defineWCS() is used to
        /// extract the WCS parameters from the FITS header. This is
        /// then sent to SubimageDef::define(wcsprm *) to define
        /// everything.
        ///
        /// @param FITSfilename The name of the FITS file.
        void defineFITS(const std::string &FITSfilename);

        /// @brief Set the array of image dimensions
        /// @{
        void setImageDim(const std::vector<int> &dim);
        void setImageDim(const std::vector<long> &dim);
        void setImageDim(const std::vector<size_t> &dim);
        void setImageDim(const long *dim, const size_t size);
        void setImageDim(const size_t *dim, const size_t size);
        const std::vector<long> getImageDim() {return itsFullImageDim;};
        ///@}

        /// @brief Set the image name.
        void setImage(const std::string &imageName) {itsImageName = imageName;};

        /// @brief Set the input subsection
        void setInputSubsection(const std::string &section) {itsInputSection = section;};

        /// @brief Return a subsection specification for a given worker
        /// @details Return the subsection object for the given worker
        /// number. (These start at 0). The subimages are tiled across
        /// the cube with the x-direction varying quickest, then y, then
        /// z.
        /// @return A duchamp::Section object containing all information
        /// on the subsection.
        duchamp::Section section(const int workerNum);

        /// @brief Define the subsection specification for *every* worker
        void defineAllSections();

        /// @brief Return the bottom-left-corner of a worker's subsection
        const casa::IPosition blc(const int workerNum);

        /// @brief Return the number of subimages.
        const unsigned int numSubs() {return itsNSubX * itsNSubY * itsNSubZ;};
        /// @brief The number of axes
        const unsigned int naxis() {return itsNAxis;};

        /// @brief Return the number of subdivisions in given directions
        /// @{
        const unsigned int nsubx() {return itsNSubX;};
        const unsigned int nsuby() {return itsNSubY;};
        const unsigned int nsubz() {return itsNSubZ;};
        const std::vector<unsigned int> nsub() {return itsNSub;}
        /// @}

        /// @brief Return the size of the overlap in given directions
        /// @{
        const unsigned int overlapx() {return itsOverlapX;};
        void setOverlapX(int o) {itsOverlapX = o;};
        const unsigned int overlapy() {return itsOverlapY;};
        void setOverlapY(int o) {itsOverlapY = o;};
        const unsigned int overlapz() {return itsOverlapZ;};
        void setOverlapZ(int o) {itsOverlapZ = o;};
        const std::vector<unsigned int> overlap() {return itsOverlap;};
        /// @}

        /// @brief Create a Karma annotation file showing the borders
        /// of the subimages.
        /// @details This creates a Karma annotation file that simply has
        /// the borders of the subimages plotted on it, along with the
        /// worker number at the centre of each subimage.
        void writeAnnotationFile(duchamp::FitsHeader &head,
                                 askap::askapparallel::AskapParallel& comms);

        /// @brief Which worker(s) does a given location fall in?
        const std::set<int> affectedWorkers(const int x, const int y, const int z);
        const std::set<int> affectedWorkers(const float x, const float y, const float z);
        const std::set<int> affectedWorkers(const casa::IPosition &pos);
        /// @brief Which workers does a given slice overlap with?
        const std::set<int> affectedWorkers(const casa::Slicer &slice);

    protected:
        /// @brief Number of subdivisions in the x-direction
        unsigned int itsNSubX;
        /// @brief Number of subdivisions in the y-direction
        unsigned int itsNSubY;
        /// @brief Number of subdivisions in the z-direction
        unsigned int itsNSubZ;
        /// @brief Size of the overlap between subimages in the x-direction
        unsigned int itsOverlapX;
        /// @brief Size of the overlap between subimages in the y-direction
        unsigned int itsOverlapY;
        /// @brief Size of the overlap between subimages in the z-direction
        unsigned int itsOverlapZ;
        /// @brief The array of NSub(X,Y,Z) values, ordered in the
        /// appropriate sense according to the WCS
        std::vector<unsigned int> itsNSub;
        /// @brief The array of Overlap(X,Y,Z) values, ordered in the
        /// appropriate sense according to the WCS
        std::vector<unsigned int> itsOverlap;
        /// @brief The number of axes (the size of the itsNSub and
        /// itsOverlap arrays)
        unsigned int itsNAxis;
        /// @brief The dimensions of the full image
        std::vector<long> itsFullImageDim;
        /// @brief The name of the image
        std::string itsImageName;
        /// @brief The subsection of the input image
        std::string itsInputSection;
        /// @brief The set of subsection specifications for all workers
        std::vector<duchamp::Section> itsSectionList;
        /// @brief Which axis in the longitude axis
        int itsLng;
        /// @brief Which axis in the latitude axis
        int itsLat;
        /// @brief Which axis in the spectral axis
        int itsSpec;
        /// The karma annotation file containing the map of subimage boundaries
        std::string itsAnnotationFile;
};

}

}

#endif
