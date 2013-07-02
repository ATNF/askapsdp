/// @file
///
/// Functions that provide interfaces between CASA images &
/// coordinates and more familiar Duchamp structures. Also provides
/// utility functions that enable rapid access to certain parts of
/// images or coordinate systems.
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#include <askap_analysisutilities.h>

#include <casainterface/CasaInterface.h>
#include <analysisparallel/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/SubImage.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/RecordFieldId.h>
#include <casa/Quanta.h>
#include <casa/Quanta/Unit.h>

#include <string>
#include <algorithm>
#include <stdlib.h>

#include <wcslib/wcs.h>
#include <wcslib/wcsfix.h>

#include <duchamp/duchamp.hh>
#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>

using namespace casa;
using namespace duchamp;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".casainterface");

namespace askap {

  namespace analysisutilities {

     long *getDim(const ImageInterface<Float>* imagePtr)
      {
	IPosition shape = imagePtr->shape();
	long *dim = new long[shape.size()];

	for (size_t i = 0; i < shape.size(); i++) {
	  dim[i] = shape(i);
	  ASKAPCHECK(dim[i] > 0, "Negative dimension: dim[" << i << "]=" << dim[i]);
	}
	
	return dim;
      }


      //**************************************************************//

        void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs)
        {
            /// @details Stores a wcsprm struct to a duchamp::FitsHeader,
            /// setting the parameters such as spectralDescription
            /// appropriately.
            /// @param head The duchamp::FitsHeader object containing the header information
            /// @param par The duchamp::Param object, use for information on the spectral units
            /// @param wcs The WCS information
            if (wcs->spec >= 0) { //if there is a spectral axis
                int index = wcs->spec;
                std::string desiredType, specType = wcs->ctype[index];
                std::string shortType = specType.substr(0, 4);

                if (shortType == "VELO" || shortType == "VOPT" || shortType == "ZOPT"
		    || shortType == "VRAD" || shortType == "BETA" ||
		    (shortType=="FREQ" && wcs->restfrq!=0)) {
                    if (wcs->restfrq != 0) {
                        // Set the spectral axis to a standard specification: VELO-F2V
                        desiredType = duchampVelocityType;

                        if (wcs->restwav == 0)
                            wcs->restwav = 299792458.0 /  wcs->restfrq;

                        head.setSpectralDescription(duchampSpectralDescription[VELOCITY]);
                    } else {
                        // No rest frequency defined, so put spectral dimension in frequency.
                        // Set the spectral axis to a standard specification: FREQ
//D1.1.13               duchampWarning("Cube Reader", "No rest frequency defined. Using frequency units in spectral axis.\n");
                        DUCHAMPWARN("Cube Reader", "No rest frequency defined. Using frequency units in spectral axis.");
                        desiredType = duchampFrequencyType;
                        par.setSpectralUnits("MHz");

                        if (strcmp(wcs->cunit[index], "") == 0) {
//D1.1.13                   duchampWarning("Cube Reader","No frequency unit given. Assuming frequency axis is in Hz.\n");
                            DUCHAMPWARN("Cube Reader","No frequency unit given. Assuming frequency axis is in Hz.");
                            strcpy(wcs->cunit[index], "Hz");
                        }

                        head.setSpectralDescription(duchampSpectralDescription[FREQUENCY]);
                    }
                } else {
                    desiredType = duchampFrequencyType;
                    par.setSpectralUnits("MHz");

                    if (strcmp(wcs->cunit[index], "") == 0) {
//D1.1.13               duchampWarning("Cube Reader","No frequency unit given. Assuming frequency axis is in Hz.\n");
                        DUCHAMPWARN("Cube Reader","No frequency unit given. Assuming frequency axis is in Hz.");
                        strcpy(wcs->cunit[index], "Hz");
                    }

                    head.setSpectralDescription(duchampSpectralDescription[FREQUENCY]);
                }

                // Now we need to make sure the spectral axis has the correct setup.
                //  We use wcssptr to translate it if it is not of the desired type,
                //  or if the spectral units are not defined.
                bool needToTranslate = false;
                //       if(strncmp(specType.c_str(),desiredType.c_str(),4)!=0)
                //  needToTranslate = true;
                std::string blankstring = "";

                if (strcmp(wcs->cunit[wcs->spec], blankstring.c_str()) == 0)
                    needToTranslate = true;

                if (needToTranslate) {
                    if (strcmp(wcs->ctype[wcs->spec], "VELO") == 0)
                        strcpy(wcs->ctype[wcs->spec], "VELO-F2V");

                    index = wcs->spec;
                    int status = wcssptr(wcs, &index, (char *)desiredType.c_str());

                    if (status) {
                        std::stringstream errmsg;
                        errmsg << "WCSSPTR failed! Code=" << status << ": "
                            << wcs_errmsg[status] << std::endl
                            << "(wanted to convert from type \"" << specType
                            << "\" to type \"" << desiredType << "\")";
//D1.1.13               duchampWarning("Cube Reader", errmsg.str());
                        DUCHAMPWARN("Cube Reader", errmsg.str());
                    }
                }
            } // end of if(wcs->spec>=0)

            // Save the wcs to the FitsHeader class that is running this function
            head.setWCS(wcs);
            head.setNWCS(1);
        }

        //**************************************************************//


      std::string getFullSection(std::string filename)
      {
	/// @details Returns a full subsection string with the correct
	/// number of dimensions for the image in filename. For
	/// instance, a four-dimensional image will give the full
	/// subsection string "[*,*,*,*]"
	/// @param filename The image file to be examined
	/// @return A subsection string with an asterisk for each dimension

	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	const LatticeBase* lattPtr = ImageOpener::openImage(filename);
	
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested image \"" << filename << "\" does not exist or could not be opened.");
	
	const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	return duchamp::nullSection(imagePtr->shape().size());

      }

       //**************************************************************//

        std::vector<size_t> getCASAdimensions(std::string filename)
        {
            /// @details Equivalent of getFITSdimensions, but for
            /// casa images. Returns a vector with the axis dimensions of the given image
            /// @param filename The filename of the image
            /// @return An STL vector with all axis dimensions.
            ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
            const LatticeBase* lattPtr = ImageOpener::openImage(filename);

            if (lattPtr == 0)
                ASKAPTHROW(AskapError, "Requested image \"" << filename << "\" does not exist or could not be opened.");

            const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
            IPosition shape = imagePtr->shape();
            std::vector<size_t> dim(shape.size());

            for (uint i = 0; i < shape.size(); i++) dim[i] = shape(i);

            delete lattPtr;
            return dim;
        }

        //**************************************************************//

        float findSurroundingNoise(std::string filename, float xpt, float ypt, int noiseBoxSize)
        {
            /// @details This function finds the noise level (that is,
            /// the RMS deviation) for a box of a given size around a
            /// given position in a given image. The noise level is
            /// actually calculated with the MADFM, which is converted
            /// to an equivalent RMS assuming Gaussian statistics.
            ///
            /// The box is assumed to be square, centred at the provided
            /// coordinates, although it is truncated at the edge of the
            /// image.
            ///
            /// @param filename The name of the file
            /// @param xpt The x-coordinate of the centre of the box
            /// @param ypt The y-coordinate of the centre of the box
            /// @param noiseBoxSize The side length of the box

            ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
            const LatticeBase* lattPtr = ImageOpener::openImage(filename);

            if (lattPtr == 0)
                ASKAPTHROW(AskapError, "Requested image \"" << filename << "\" does not exist or could not be opened.");

            const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
            IPosition shape = imagePtr->shape();
            IPosition start(shape.size(), 0);
            IPosition end(shape.size(), 0);
            IPosition stride(shape.size(), 1);
            start(0) = std::max(int(xpt) - noiseBoxSize / 2, 0);
            start(1) = std::max(int(ypt) - noiseBoxSize / 2, 0);
            end(0) = std::min(size_t(xpt) + noiseBoxSize / 2, size_t(shape(0) - 1));
            end(1) = std::min(size_t(ypt) + noiseBoxSize / 2, size_t(shape(1) - 1));
            std::vector<float> array;
            imagePtr->getSlice(Slicer(start, end, stride, Slicer::endIsLast)).tovector(array);
            size_t arrsize = array.size();
            std::nth_element(array.begin(), array.begin() + arrsize / 2, array.end());
            float median = array[arrsize/2];

            if (arrsize % 2 == 0) {
                std::nth_element(array.begin(), array.begin() + arrsize / 2 - 1, array.end());
                median += array[arrsize/2-1];
                median /= 2.;
            }

            for (size_t i = 0; i < arrsize; i++) array[i] = fabs(array[i] - median);

            std::nth_element(array.begin(), array.begin() + arrsize / 2, array.end());
            float madfm = array[arrsize/2];

            if (arrsize % 2 == 0) {
                std::nth_element(array.begin(), array.begin() + arrsize / 2 - 1, array.end());
                median += array[arrsize/2-1];
                median /= 2.;
            }

            madfm = Statistics::madfmToSigma(madfm);
            delete lattPtr;
            return madfm;
        }

        //**************************************************************//

        casa::Array<casa::Float> getPixelsInBox(std::string imageName, casa::Slicer box, bool fixSlice)
        {
            /// @details Extract a set of pixel values from a region of
            /// an image. The region is defined by a casa::Slicer
            /// object, and the pixel values are returned in a
            /// casa::Vector object - this is suitable for
            /// RadioSource::findAlpha() etc.
            /// @param imageName The image to get the data from
            /// @param box The region within in the image
            /// @return A casa::Vector of casa::Double pixel values

	  //	  ASKAPLOG_DEBUG_STR(logger, "getPixelsInBox: starting to look in image " << imageName << " with slicer " << box);
            ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
            const LatticeBase* lattPtr = ImageOpener::openImage(imageName);

            if (lattPtr == 0)
                ASKAPTHROW(AskapError, "Requested image \"" << imageName << "\" does not exist or could not be opened.");

            const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);

	    lengthenSlicer(box,imagePtr->ndim());
	    casa::Slicer newSlicer=box;
	    if(fixSlice){
	      wcsprm *tempwcs = casaImageToWCS(imagePtr);
	      fixSlicer(newSlicer, tempwcs);
	    }

	    casa::Array<Float> array = imagePtr->getSlice(newSlicer,true);
            // float *data = array.data();
            // casa::Vector<Double> vec(array.size());

            // for (size_t i = 0; i < vec.size(); i++) vec(i) = casa::Double(data[i]);

            delete lattPtr;
            // return vec;
	    return array;
        }

        //**************************************************************//

         void readBeamInfo(const ImageInterface<Float>* imagePtr, duchamp::FitsHeader &head, duchamp::Param &par)
        {
            /// @details Reads the beam information (major axis, minor axis,
            /// position angle) from an already opened casa image and stores
            /// in the duchamp::FitsHeader provided.
            /// @param imagePtr The casa image
            /// @param head The duchamp::FitsHeader, where the beam information is stored
            /// @param par Used for default beam size in case there is no
            /// beam info in the image, or to store the beam size (in
            /// pixels) if there is.

            casa::Vector<casa::Quantum<Double> > beam = imagePtr->imageInfo().restoringBeam();
	    ASKAPLOG_DEBUG_STR(logger, "Read beam from casa image: " << beam );

            if (beam.size() == 0) {
                std::stringstream errmsg;
                ASKAPLOG_WARN_STR(logger, "Beam information not present. Using parameter set to determine size of beam.");
		if(par.getBeamFWHM()>0.) head.beam().setFWHM(par.getBeamFWHM(),PARAM);
		else if(par.getBeamSize()>0.) head.beam().setArea(par.getBeamSize(),PARAM);
		else head.beam().empty();
            } else {
                double bmaj = beam[0].getValue("deg");
                double bmin = beam[1].getValue("deg");
                double bpa = beam[2].getValue("deg");
                float pixScale = head.getAvPixScale();
		head.beam().define(bmaj/pixScale,bmin/pixScale,bpa,HEADER);
            }
	    par.setBeamAsUsed(head.beam());

	    ASKAPLOG_DEBUG_STR(logger, "Beam to be used: (maj,min,pa)=("<<head.beam().maj()<<","<<head.beam().min()<<","<<head.beam().pa()<<")");

        }

        //**************************************************************//

        wcsprm *casaImageToWCS(std::string imageName)
        {
            /// @details Read the WCS from an image using casacore methods
            /// to access it. Calls casaImageToWCS(ImageInterface<Float> *).
            /// @param imageName The name of the image to access
            /// @return A wcsprm pointer containing the wcs information of the image.
            ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
            const LatticeBase* lattPtr = ImageOpener::openImage(imageName);

            if (lattPtr == 0)
                ASKAPTHROW(AskapError, "Requested image \"" << imageName << "\" does not exist or could not be opened.");

            const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
            return casaImageToWCS(imagePtr);
        }

        //**************************************************************//

        wcsprm *casaImageToWCS(const ImageInterface<Float>* imagePtr)
        {
            /// @details Read the WCS from a casa image. Uses the
            /// CoordinateSystem::toFITSHeader() function to access the
            /// header records, then explicitly copies each WCS header to a
            /// WCSLIB wcsprm struct. Both wcsset() and wcsfix() are called
            /// on this, and then it is returned.
            /// @param imageName The already opened image.
            /// @return A wcsprm pointer containing the wcs information of
            /// the image.
            IPosition shape = imagePtr->shape();
            long *dim = new long[shape.size()];

            for (uint i = 0; i < shape.size(); i++) dim[i] = shape(i);

            CoordinateSystem coords = imagePtr->coordinates();
            Record hdr;

// 	    ASKAPLOG_DEBUG_STR(logger, "imagePtr->coordinates().SpectralCoordinate().frequencySystem(): " << casa::MFrequency::showType(coords.spectralCoordinate(coords.findCoordinate(casa::Coordinate::SPECTRAL)).frequencySystem()));
// 	    ASKAPLOG_DEBUG_STR(logger, "imagePtr->coordinates().SpectralCoordinate().velocityDoppler(): " << casa::MDoppler::showType(coords.spectralCoordinate(coords.findCoordinate(casa::Coordinate::SPECTRAL)).velocityDoppler()));
// 	    ASKAPLOG_DEBUG_STR(logger, "imagePtr->coordinates().SpectralCoordinate().worldAxisNames(): " << coords.spectralCoordinate(coords.findCoordinate(casa::Coordinate::SPECTRAL)).worldAxisNames());
            if (!coords.toFITSHeader(hdr, shape, true, 'c', true,true,true,false)) throw AskapError("casaImageToWCS: could not read FITS header parameters");
// 	    ASKAPLOG_DEBUG_STR(logger, "coords.toFITSHeader: " << hdr);

            std::vector<Double> vals;
            struct wcsprm *wcs;
            wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
            wcs->flag = -1;
            int ndim = shape.size();

            int status = wcsini(1, ndim, wcs);

            if (status)
                ASKAPTHROW(AskapError, "casaImageToWCS: wcsini failed! Code=" << status << ": " << wcs_errmsg[status]);

           status = wcsset(wcs);

            if (status){
                ASKAPTHROW(AskapError, "casaImageToWCS: wcsset failed! WCSLIB error code=" << status  << ": " << wcs_errmsg[status]);
	    }

            Array<String>::iterator it;
            int i;

	    if (hdr.isDefined("ctype")) {
                RecordFieldId ctypeID("ctype");
                Array<String> ctype = hdr.asArrayString(ctypeID);
                Array<String>::iterator it;
                i = 0;

                for (it = ctype.begin(); it != ctype.end(); it++) {
                    String str = *it;
                    strcpy(wcs->ctype[i++], str.c_str());

// 		    char stype[5],scode[4],sname[23],units[9],ptype[2],xtype[2];
// 		    int restflag;
// 		    status=spctyp(wcs->ctype[i-1],stype,scode,sname,units,ptype,xtype,&restflag);
// 		    ASKAPLOG_DEBUG_STR(logger, i-1<<" " <<status << " " <<wcs->ctype[i-1] << " : " 
// 				       << stype << " " << scode << " " << sname
// 				       << " " << units << " " << ptype << " " << xtype << " " << restflag);

                }
//  		ASKAPLOG_DEBUG_STR(logger, "1 wcs->ctype[spec] = " << wcs->ctype[wcs->spec] << " spec="<<wcs->spec << " wcs->ctype[2] = " << wcs->ctype[2]);
            }

            if (hdr.isDefined("cunit")) {
                RecordFieldId cunitID("cunit");
                Array<String> cunit = hdr.asArrayString(cunitID);
                i = 0;

                for (it = cunit.begin(); it != cunit.end(); it++) {
                    String str = *it;
                    strcpy(wcs->cunit[i++], str.c_str());
                }
            }

            if (hdr.isDefined("crpix")) {
                RecordFieldId crpixID("crpix");
                Array<Double>::iterator it2;
                Array<Double> crpix = hdr.asArrayDouble(crpixID);
                crpix.tovector(vals);

                for (uint i = 0; i < vals.size(); i++) wcs->crpix[i] = double(vals[i]);
            }

            if (hdr.isDefined("crval")) {
                RecordFieldId crvalID("crval");
                Array<Double> crval = hdr.asArrayDouble(crvalID);
                crval.tovector(vals);

                for (uint i = 0; i < vals.size(); i++) wcs->crval[i] = double(vals[i]);
            }

            if (hdr.isDefined("cdelt")) {
                RecordFieldId cdeltID("cdelt");
                Array<Double> cdelt = hdr.asArrayDouble(cdeltID);
                cdelt.tovector(vals);

                for (uint i = 0; i < vals.size(); i++) wcs->cdelt[i] = double(vals[i]);
            }

            if (hdr.isDefined("crota")) {
                RecordFieldId crotaID("crota");
                Array<Double> crota = hdr.asArrayDouble(crotaID);
                crota.tovector(vals);

                for (uint i = 0; i < vals.size(); i++) {
                    wcs->crota[i] = double(vals[i]);
                    wcs->altlin |= 4;
                }
            }

            if (hdr.isDefined("pc")) {
                RecordFieldId pcID("pc");
                Array<Double> pc = hdr.asArrayDouble(pcID);
                pc.tovector(vals);

                for (uint i = 0; i < vals.size(); i++) wcs->pc[i] = double(vals[i]);
            }

	    // THE FOLLOWING IS HARD-CODED FOR PV2_  - NEED TO MAKE THIS MORE FLEXIBLE PERHAPS
            if (hdr.isDefined("pv2_")) {
                RecordFieldId pvID("pv2_");
		Array<Double> pv2 = hdr.asArrayDouble(pvID);
                pv2.tovector(vals);

	    	int axis=2;
                for (uint i = 0; i < vals.size(); i++){
		  struct pvcard thispv;
		  thispv.i=axis;
		  thispv.m=i+1;
		  thispv.value=double(vals[i]);
		  wcs->pv[i] = thispv;
		  wcs->npv++;
		}
	    }


            if (hdr.isDefined("lonpole")) {
                RecordFieldId lonpoleID("lonpole");
                Double lonpole = hdr.asDouble(lonpoleID);
                wcs->lonpole = double(lonpole);
            }

            if (hdr.isDefined("equinox")) {
                RecordFieldId equinoxID("equinox");
                Double equinox = hdr.asDouble(equinoxID);
                wcs->equinox = double(equinox);
            }

	    // if (hdr.isDefined("restfreq")) {
            //     RecordFieldId restfreqID("restfreq");
            //     Double restfreq = hdr.asDouble(restfreqID);
            //     wcs->restfrq = double(restfreq);
            // }
	    // // ***IMPORTANT***
	    // // casacore-1.4.0 changes "restfreq" to "restfrq", in line
	    // // with FITS standard v3.0
	    // // perhaps better to use the abstract access via the SpectralCoordinate
 	    int specCoord = coords.findCoordinate(Coordinate::SPECTRAL);
	    if(specCoord>=0 && coords.spectralCoordinate(specCoord).restFrequency()>0.01){
	      wcs->restfrq = coords.spectralCoordinate(specCoord).restFrequency();
	    }


            if (hdr.isDefined("restwave")) {
                RecordFieldId restwaveID("restwave");
                Double restwave = hdr.asDouble(restwaveID);
                wcs->restwav = double(restwave);
            }

            if (hdr.isDefined("date-obs")) {
                RecordFieldId dateID("date-obs");
                String date = hdr.asString(dateID);
                strcpy(wcs->dateobs, date.c_str());
            }

// 	    wcsprt(wcs);

            int stat[NWCSFIX];
            // Applies all necessary corrections to the wcsprm structure
            //  (missing cards, non-standard units or spectral types, ...)
            status = wcsfix(1, (const int*)dim, wcs, stat);
	    
            if (status) {
                std::stringstream errmsg;
                errmsg << "casaImageToWCS: wcsfix failed: Function status returns are:\n";

                for (int i = 0; i < NWCSFIX; i++)
                    if (stat[i] > 0)
                        errmsg << i + 1 << ": WCSFIX error code=" << stat[i] << ": "
                            << wcsfix_errmsg[stat[i]] << std::endl;

                ASKAPTHROW(AskapError, errmsg.str());
            }

// 	    ASKAPLOG_DEBUG_STR(logger, "2 wcs->ctype[spec] = " << wcs->ctype[wcs->spec] << " spec="<<wcs->spec);

            status = wcsset(wcs);

            if (status){
                ASKAPTHROW(AskapError, "casaImageToWCS: wcsset failed! WCSLIB error code=" << status  << ": " << wcs_errmsg[status]);
	    }
	    
	    // Re-do the corrections to account for things like NCP projections
	    status = wcsfix(1, (const int*)dim, wcs, stat);
	    if(status) {
	      std::stringstream errmsg;
	      errmsg << "wcsfix failed: Function status returns are:\n";
	      for(int i=0; i<NWCSFIX; i++)
		if (stat[i] > 0) 
		  errmsg << i+1 << ": WCSFIX error code=" << stat[i] << ": "
			 << wcsfix_errmsg[stat[i]] << std::endl;
	      ASKAPTHROW(AskapError, "casaImageToWCS: " << errmsg.str());
	    }
	    
// 	    ASKAPLOG_DEBUG_STR(logger, "3 wcs->ctype[spec] = " << wcs->ctype[wcs->spec] << " spec="<<wcs->spec);
	    status = wcsset(wcs);
	    if (status)
	      ASKAPTHROW(AskapError, "casaImageToWCS: wcsset failed! WCSLIB error code=" << status  << ": " << wcs_errmsg[status]);
// 	    ASKAPLOG_DEBUG_STR(logger, "4 wcs->ctype[spec] = " << wcs->ctype[wcs->spec] << " spec="<<wcs->spec);

            delete [] dim;

// 	    wcsprt(wcs);

            return wcs;
        }

        //**************************************************************//

        casa::CoordinateSystem wcsToCASAcoord(wcsprm *wcs, int nstokes)
        {
            /// @details Convert a wcslib WCS specification to a casa-compatible specification.

            casa::CoordinateSystem csys;

            ASKAPLOG_INFO_STR(logger, "Defining direction coords");

            Matrix<Double> xform(2, 2);
            xform = 0.0; xform.diagonal() = 1.0;
            casa::DirectionCoordinate dirCoo(MDirection::J2000,
                                             Projection(Projection::SIN),
                                             wcs->crval[wcs->lng]*casa::C::pi / 180., wcs->crval[wcs->lat]*casa::C::pi / 180.,
                                             wcs->cdelt[wcs->lng]*casa::C::pi / 180., wcs->cdelt[wcs->lat]*casa::C::pi / 180.,
                                             xform,
                                             wcs->crpix[wcs->lng] - 1, wcs->crpix[wcs->lat] - 1);

            casa::SpectralCoordinate specCoo(MFrequency::TOPO,
                                             wcs->crval[wcs->spec],
                                             wcs->cdelt[wcs->spec],
                                             wcs->crpix[wcs->spec] - 1,
                                             wcs->restfrq);


            Vector<Int> stokes(nstokes);
            stokes(0) = casa::Stokes::I;

            if (nstokes == 4) {
                stokes(1) = casa::Stokes::Q;
                stokes(2) = casa::Stokes::U;
                stokes(3) = casa::Stokes::V;
            }

            casa::StokesCoordinate stokesCoo(stokes);

            for (int i = 0; i < wcs->naxis; i++) {
                if (i == wcs->lng || i == wcs->lat) {
                    i++;
                    csys.addCoordinate(dirCoo);
                } else if (i == wcs->spec) csys.addCoordinate(specCoo);
                else if (wcs->naxis == 4) csys.addCoordinate(stokesCoo);
            }


            return csys;

        }
        //**************************************************************//

        Slicer subsectionToSlicer(duchamp::Section &subsection)
        {
            Vector<int> secStarts(subsection.getStartList());
            Vector<int> secLengths(subsection.getDimList());

	    return Slicer(IPosition(secStarts), IPosition(secLengths));
        }

      Slicer subsectionToSlicer(duchamp::Section &subsection, wcsprm *wcs)
      {

	Vector<int> secStarts(subsection.getStartList());
	Vector<int> secLengths(subsection.getDimList());
	
	if(wcs->spec==3){
	  std::swap(secStarts(2),secStarts(3));
	  std::swap(secLengths(2),secLengths(3));
	}
	else if(wcs->spec != 2){
	  ASKAPTHROW(AskapError, "Unexpected value for wcs->spec = " << wcs->spec);
	}

	return Slicer(IPosition(secStarts), IPosition(secLengths));

      }

        //**************************************************************//

        void fixSlicer(Slicer &slice, wcsprm *wcs)
        {
            IPosition start = slice.start();
            IPosition end = slice.end();
            IPosition stride = slice.stride();

            for (size_t i = 0; i < start.size(); i++) {
                // set all axes that aren't position or spectral to just the first value.
                int index = int(i);

                if (index != wcs->lng && index != wcs->lat && index != wcs->spec) {
                    start(i) = 0;
                    end(i) = 0;
                }
            }

	    if(start >= end){
	      ASKAPLOG_FATAL_STR(logger, "ERROR with fixSlicer: start="<<start<<", end="<<end<<", stride="<<stride <<", caused by Slicer "<<slice<<" and WCS indices ("<<wcs->lng<<","<<wcs->lat<<","<<wcs->spec<<")");
	    }

            slice = Slicer(start, end, stride, Slicer::endIsLast);
        }

        //**************************************************************//

      void lengthenSlicer(Slicer &slice, int ndim)
        {
	  int oldDim=slice.ndim();
	  if(oldDim<ndim){
	    
            IPosition start = slice.start();
	    start.resize(ndim);
	    for(int i=oldDim; i<ndim;i++) start[i]=0;
            IPosition end = slice.end();
	    end.resize(ndim);
	    for(int i=oldDim; i<ndim;i++) end[i]=0;
            IPosition stride = slice.stride();
	    stride.resize(ndim);
	    for(int i=oldDim; i<ndim;i++) stride[i]=1;

 	    if(start >= end){
	      ASKAPLOG_FATAL_STR(logger, "ERROR with lengthenSlicer: start="<<start<<", end="<<end<<", stride="<<stride <<", caused by Slicer "<<slice<<" going from "<<oldDim<<"dim to "<<ndim<<"dim");
	    }

           slice = Slicer(start, end, stride, Slicer::endIsLast);
	  }

	}


  }

}



