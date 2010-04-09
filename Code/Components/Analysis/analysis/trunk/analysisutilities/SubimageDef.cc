/// @file
///
/// @brief Define and access subimages of a FITS file.
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>

#include <gsl/gsl_sf_gamma.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageOpener.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
//  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

using namespace casa;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".subimagedef");

namespace askap {
    namespace analysis {

        SubimageDef::SubimageDef()
        {
            this->itsNAxis = 0;
            this->itsNSubX = 1;
            this->itsNSubY = 1;
            this->itsNSubZ = 1;
            this->itsOverlapX = this->itsOverlapY = this->itsOverlapZ = 0;
            this->itsImageName = "";
	    this->itsNSub = 0;
	    this->itsOverlap = 0;
        }

        SubimageDef::~SubimageDef()
        {
            if (this->itsNAxis > 0) {
                delete [] this->itsNSub;
                delete [] this->itsOverlap;
            }
        }

        SubimageDef::SubimageDef(const LOFAR::ParameterSet& parset)
        {
            this->itsNAxis = 0;
            this->itsImageName = parset.getString("image", "");
            this->itsNSubX = parset.getInt16("nsubx", 1);
            this->itsNSubY = parset.getInt16("nsuby", 1);
            this->itsNSubZ = parset.getInt16("nsubz", 1);
            this->itsOverlapX = parset.getInt16("overlapx", 0);
            this->itsOverlapY = parset.getInt16("overlapy", 0);
            this->itsOverlapZ = parset.getInt16("overlapz", 0);
        }


        SubimageDef& SubimageDef::operator= (const SubimageDef& s)
        {
            /// @details Copy constructor for SubimageDef, that does a deep
            /// copy of the itsNSub and itsOverlap arrays.
            if (this == &s) return *this;

            this->itsNSubX = s.itsNSubX;
            this->itsNSubY = s.itsNSubY;
            this->itsNSubZ = s.itsNSubZ;
            this->itsOverlapX = s.itsOverlapX;
            this->itsOverlapY = s.itsOverlapY;
            this->itsOverlapZ = s.itsOverlapZ;
            this->itsNAxis = s.itsNAxis;
            this->itsFullImageDim = s.itsFullImageDim;
            this->itsImageName = s.itsImageName;

            if (this->itsNAxis > 0) {
	        this->itsNSub = new int[this->itsNAxis];
	        this->itsOverlap = new int[this->itsNAxis];
                for (int i = 0; i < this->itsNAxis; i++) {
                    this->itsNSub[i] = s.itsNSub[i];
                    this->itsOverlap[i] = s.itsOverlap[i];
                }
            }

            return *this;
        }

        void SubimageDef::define(int numDim)
        {

            struct wcsprm *wcs;
            wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
            wcs->naxis = numDim;
            wcs->lng = 0;
            wcs->lat = 1;
            wcs->spec = 2;
            this->define(wcs);
            int nwcs = 1;
            wcsvfree(&nwcs, &wcs);

        }

        void SubimageDef::define(wcsprm *wcs)
        {
            /// @details Define all the necessary variables within the
            /// SubimageDef class. The image (given by the parameter "image"
            /// in the parset) is to be split up according to the nsubx/y/z
            /// parameters, with overlaps in each direction given by the
            /// overlapx/y/z parameters (these are in pixels).
            ///
            /// The WCS parameters in wcs determine which axes are the x, y and z
            /// axes. The number of axes is also determined from the WCS
            /// parameter set.
            ///
            /// @param wcs The WCSLIB definition of the world coordinate system
            this->itsNAxis = wcs->naxis;
            this->itsLng  = wcs->lng;
            this->itsLat  = wcs->lat;
            this->itsSpec = wcs->spec;

            if (this->itsNAxis > 0) {
                this->itsNSub = new int[this->itsNAxis];
                this->itsOverlap = new int[this->itsNAxis];

                for (int i = 0; i < this->itsNAxis; i++) {
                    if (i == this->itsLng) {
                        this->itsNSub[i] = this->itsNSubX;
                        this->itsOverlap[i] = this->itsOverlapX;
                    } else if (i == this->itsLat) {
                        this->itsNSub[i] = this->itsNSubY;
                        this->itsOverlap[i] = this->itsOverlapY;
                    } else if (i == this->itsSpec) {
                        this->itsNSub[i] = this->itsNSubZ;
                        this->itsOverlap[i] = this->itsOverlapZ;
                    } else {
                        this->itsNSub[i] = 1;
                        this->itsOverlap[i] = 0;
                    }

                }
            }

        }

        void SubimageDef::defineFITS(std::string FITSfilename)
        {
            /// @details Define all the necessary variables within the
            /// SubimageDef class. The image (given by the parameter "image"
            /// in the parset) is to be split up according to the nsubx/y/z
            /// parameters, with overlaps in each direction given by the
            /// overlapx/y/z parameters (these are in pixels).
            ///
            /// This version is designed for FITS files. The Duchamp
            /// function duchamp::FitsHeader::defineWCS() is used to extract
            /// the WCS parameters from the FITS header. This is then sent
            /// to SubimageDef::define(wcsprm *) to define everything.
            ///
            /// @param FITSfilename The name of the FITS file.
            duchamp::Param tempPar; // This is needed for defineWCS(), but we don't care about the contents.
            duchamp::FitsHeader imageHeader;
            this->itsImageName = FITSfilename;
            imageHeader.defineWCS(this->itsImageName, tempPar);
            this->define(imageHeader.getWCS());
        }

        duchamp::Section SubimageDef::section(int workerNum, std::string inputSubsection)
        {
            /// @details Return the subsection object for the given worker
            /// number. (These start at 0). The subimages are tiled across
            /// the cube with the x-direction varying quickest, then y, then
            /// z. The dimensions of the array are obtained with the
            /// getFITSdimensions(std::string) function.
            /// @return A duchamp::Section object containing all information
            /// on the subsection.

            if (this->itsFullImageDim.size() == 0) {
                ASKAPTHROW(AskapError, "SubimageDef::section : tried to define a section but the image dimensions have not been set!");
            }

            if (workerNum < 0) {
                ASKAPLOG_INFO_STR(logger, "Master node, so returning input subsection");
                return inputSubsection;
            } else {
                ASKAPLOG_INFO_STR(logger, "Input subsection to be used is " << inputSubsection);
                duchamp::Section inputSec(inputSubsection);
                inputSec.parse(this->itsFullImageDim);
                ASKAPLOG_INFO_STR(logger, "Input subsection is OK");
                long *sub = new long[this->itsNAxis];
		for(int i=0;i<this->itsNAxis;i++) sub[i]=0;
                sub[this->itsLng] = workerNum % this->itsNSub[0];
                sub[this->itsLat] = (workerNum % (this->itsNSub[0] * this->itsNSub[1])) / this->itsNSub[0];
                sub[this->itsSpec] = workerNum / (this->itsNSub[0] * this->itsNSub[1]);
                std::stringstream section;

                for (int i = 0; i < this->itsNAxis; i++) {
                    if (this->itsNSub[i] > 1) {
                        int length = inputSec.getDim(i);
                        float sublength = float(length) / float(this->itsNSub[i]);
                        int min = std::max(long(inputSec.getStart(i)), long(inputSec.getStart(i) + sub[i] * sublength - this->itsOverlap[i] / 2)) + 1;
                        int max = std::min(long(inputSec.getEnd(i) + 1), long(inputSec.getStart(i) + (sub[i] + 1) * sublength + this->itsOverlap[i] / 2));
                        section << min << ":" << max;
                    } else
                        section << inputSec.getSection(i);

                    if (i != this->itsNAxis - 1) section << ",";
                }

                std::string secstring = "[" + section.str() + "]";
                ASKAPLOG_INFO_STR(logger, "New subsection to be used is " << secstring);
                duchamp::Section sec(secstring);
                sec.parse(this->itsFullImageDim);
                return sec;
            }
        }

        void SubimageDef::writeAnnotationFile(std::string filename, duchamp::Section fullImageSubsection, duchamp::FitsHeader &head, std::string imageName, int numWorkers)
        {
            /// @details This creates a Karma annotation file that simply has the borders of the subimages plotted on it.

            std::ofstream fAnnot(filename.c_str());
            fAnnot << "# Borders of subimaages for image " << imageName << "\n#\n";
            fAnnot << "COLOR YELLOW\n";
            fAnnot << "COORD W\n";
            fAnnot << "FONT lucidasans-24\n";

            double *pix = new double[12];
            double *wld = new double[12];
            float xcentre, ycentre;

            for (int i = 0; i < 4; i++) pix[i*3+2] = 0;

            for (int w = 0; w < numWorkers; w++) {

                duchamp::Section workerSection = this->section(w, fullImageSubsection.getSection());
                pix[0] = pix[9] =  workerSection.getStart(0) - 0.5 - fullImageSubsection.getStart(0);  // x-start, in pixels relative to the image that has been read
                pix[1] = pix[4] =  workerSection.getStart(1) - 0.5 - fullImageSubsection.getStart(1);  // y-start
                pix[3] = pix[6] =  workerSection.getEnd(0)  + 0.5 - fullImageSubsection.getStart(0); // x-end
                pix[7] = pix[10] = workerSection.getEnd(1)  + 0.5 - fullImageSubsection.getStart(1); // y-end
                head.pixToWCS(pix, wld, 4);
                xcentre = (wld[0] + wld[3] + wld[6] + wld[9]) / 4.;
                ycentre = (wld[1] + wld[4] + wld[7] + wld[10]) / 4.;
                ASKAPLOG_INFO_STR(logger, "Setting section box for worker " << w + 1 << ": starts are (" << pix[0] + fullImageSubsection.getStart(0) << "," << pix[1] + fullImageSubsection.getStart(1) << ") or ("
                                      << analysis::decToDMS(wld[0], "RA") << "," << analysis::decToDMS(wld[1], "DEC") << ") in WCS");
                ASKAPLOG_INFO_STR(logger, "Setting section box for worker " << w + 1 << ": ends are (" << pix[3] + fullImageSubsection.getStart(0) << "," << pix[7] + fullImageSubsection.getStart(1) << ") or ("
                                      << analysis::decToDMS(wld[3], "RA") << "," << analysis::decToDMS(wld[7], "DEC") << ") in WCS");
                fAnnot << "CLINES ";

                for (int i = 0; i < 4; i++) fAnnot << wld[i*3] << " " << wld[i*3+1] << " ";

                fAnnot << wld[0] << " " << wld[1] << "\n";
                fAnnot << "TEXT " << xcentre << " " << ycentre << " " << w + 1 << "\n";
            }

	    delete [] pix;
	    delete [] wld;

            fAnnot.close();

        }

    }

}
