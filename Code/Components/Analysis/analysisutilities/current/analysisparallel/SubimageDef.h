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
                /// @brief Copy function
                SubimageDef& operator= (const SubimageDef& s);
                /// @brief Default destructor
                virtual ~SubimageDef();

                /// @brief Set up the definition for a given number of dimensions
                void define(int numDim);
                /// @brief Set up the definition using a WCSLIB definition
                void define(wcsprm *wcs);
                /// @brief Set up the definition for a FITS file.
                void defineFITS(std::string FITSfilename);

                /// @brief Set the array of image dimensions
                /// @{
                void setImageDim(std::vector<int> dim) {itsFullImageDim = std::vector<long>(dim.size()); for (size_t i = 0; i < dim.size(); i++) itsFullImageDim[i] = dim[i];};
                void setImageDim(std::vector<long> dim) {itsFullImageDim = dim;};
		void setImageDim(long *dim, size_t size) {itsFullImageDim = std::vector<long>(size); for (size_t i = 0; i < size; i++) itsFullImageDim[i] = dim[i];};
                std::vector<long> getImageDim() {return itsFullImageDim;};
                ///@}

                /// @brief Set the image name.
                void setImage(std::string imageName) {itsImageName = imageName;};

                /// @brief Set the input subsection
                void setInputSubsection(std::string section) {itsInputSection = section;};

                /// @brief Return a subsection specification for a given worker
		//                duchamp::Section section(int workerNum, std::string inputSection);
                duchamp::Section section(int workerNum);
		/// @brief Define the subsection specification for *every* worker
		void defineAllSections();

                /// @brief Return the number of subimages.
                int numSubs() {return itsNSubX*itsNSubY*itsNSubZ;};
                /// @brief The number of axes
                int naxis() {return itsNAxis;};

                /// @brief Return the number of subdivisions in given directions
                /// @{
                int nsubx() {return itsNSubX;};
                int nsuby() {return itsNSubY;};
                int nsubz() {return itsNSubZ;};
                int *nsub() {return itsNSub;}
                /// @}

                /// @brief Return the size of the overlap in given directions
                /// @{
                int overlapx() {return itsOverlapX;};
		void setOverlapX(int o){itsOverlapX=o;};
                int overlapy() {return itsOverlapY;};
		void setOverlapY(int o){itsOverlapX=o;};
                int overlapz() {return itsOverlapZ;};
		void setOverlapZ(int o){itsOverlapX=o;};
                int *overlap() {return itsOverlap;};
                /// @}

                /// @brief Create a Karma annotation file showing the borders of the subimages.
		//                void writeAnnotationFile(std::string filename, duchamp::Section fullImageSubsection, duchamp::FitsHeader &head, std::string imageName, askap::askapparallel::AskapParallel& comms);
                void writeAnnotationFile(std::string filename, duchamp::FitsHeader &head, std::string imageName, askap::askapparallel::AskapParallel& comms);

		/// @brief Which worker(s) does a given location fall in?
		std::set<int> affectedWorkers(int x, int y, int z);
		std::set<int> affectedWorkers(float x, float y, float z);
		std::set<int> affectedWorkers(casa::IPosition pos);
		/// @brief Which workers does a given slice overlap with?
		std::set<int> affectedWorkers(casa::Slicer &slice);

            protected:
                /// @brief Number of subdivisions in the x-direction
                int itsNSubX;
                /// @brief Number of subdivisions in the y-direction
                int itsNSubY;
                /// @brief Number of subdivisions in the z-direction
                int itsNSubZ;
                /// @brief Size of the overlap between subimages in the x-direction
                int itsOverlapX;
                /// @brief Size of the overlap between subimages in the y-direction
                int itsOverlapY;
                /// @brief Size of the overlap between subimages in the z-direction
                int itsOverlapZ;
                /// @brief The array of NSub(X,Y,Z) values, ordered in the appropriate sense according to the WCS
                int *itsNSub;
                /// @brief The array of Overlap(X,Y,Z) values, ordered in the appropriate sense according to the WCS
                int *itsOverlap;
                /// @brief The number of axes (the size of the itsNSub and itsOverlap arrays)
                int itsNAxis;
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
        };

    }

}

#endif
