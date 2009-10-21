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

#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageParamsHelper.h>
#include <imageaccess/ImageAccessFactory.h>
#include <dataaccess/PolConverter.h>
#include <fitting/Axes.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/Projection.h>

#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>

#include <measures/Measures/Stokes.h>

#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <string>

using namespace askap::scimath;
using namespace casa;

namespace askap
{
  namespace synthesis
  {
    /// @brief image accessor
    boost::shared_ptr<IImageAccess> SynthesisParamsHelper::theirImageAccessor;              
    
    
  
  
    // use // deliberatly here to avoid doxygen complaining about duplicated documentation 
    // here and in the header file
    // @brief populate scimath parameters from a LOFAR Parset object
    // @details One often needs a possibility to populate 
    // scimath::Params class from a Parset file (e.g. to load 
    // initial gains from an external file). A number of add methods
    // collected in this class happen to be image-specific. This is
    // a generic method, which just copies all numeric fields
    // @param[in] params a reference to scimath parameter object, where the
    // parameters from parset file will be added
    // @param[in] parset a const reference to a parset object
    // @return a reference to params passed as an input (for chaining)
    scimath::Params& operator<<(scimath::Params &params, const LOFAR::ParameterSet &parset)
    {
       for (LOFAR::ParameterSet::const_iterator ci = parset.begin();
            ci != parset.end();++ci) {
            try {
               vector<double> vec = parset.getDoubleVector(ci->first);
               casa::Vector<double> arr(vec.size());
               std::copy(vec.begin(),vec.end(),arr.cbegin());
               params.add(ci->first, arr);
            }
            catch (const LOFAR::Exception &) {
               // ignore non-numeric parameters
            }
       }
       return params;
    }
    
    void SynthesisParamsHelper::setUpImages(const askap::scimath::Params::ShPtr& params,
				const LOFAR::ParameterSet &parset)
    {
      try {
	     vector<string> images=parset.getStringVector("Names");
         std::vector<int> shape=parset.getInt32Vector("shape");
         std::vector<std::string> cellsize=parset.getStringVector("cellsize");
	  
         for (vector<string>::const_iterator it=images.begin();it!=images.end();++it) {
              ASKAPCHECK(it->find("image") == 0, "All image names given in Names are supposed to start from 'image', you have "<<
                         *it);               
              int nchan=parset.getInt32(*it+".nchan");
              std::vector<double> freq=parset.getDoubleVector(*it+".frequency");
              std::vector<std::string> direction=parset.getStringVector(*it+".direction");
              ASKAPCHECK(!parset.isDefined(*it+".shape"), "Parameters like Cimager.Images."<<*it<<
                         ".shape are deprecated. Use Cimager.Images.shape (same for all images)");
              ASKAPCHECK(!parset.isDefined(*it+".cellsize"), "Parameters like Cimager.Images."<<*it<<
                         ".cellsize are deprecated. Use Cimager.Images.cellsize (same for all images)");
              const int nfacets = parset.getInt32(*it+".nfacets",1);
              ASKAPCHECK(nfacets>0, "Number of facets is supposed to be a positive number, you gave "<<nfacets);
              ASKAPCHECK(shape.size()>=2, "Image is supposed to be at least two dimensional. "<<
                          "check shape parameter, you gave "<<shape);
              
              // required polarisation
              if (!parset.isDefined(*it+".polarisation")) {
                  ASKAPLOG_INFO_STR(logger, "Polarisation frame is not defined for "<<*it
                                    <<", only stokes I will be generated"); 		      
              }
              const std::vector<std::string> stokesVec = parset.getStringVector(*it+".polarisation", 
                             std::vector<std::string>(1,"I"));
              // there could be many ways to define stokes, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"
              // to allow some flexibility we have to concatenate all elements first and then 
              // allow the parser from PolConverter to take care of extracting the products.                                            
              std::string stokesStr;
              for (size_t i=0; i<stokesVec.size(); ++i) {
                   stokesStr += stokesVec[i];
              }
              casa::Vector<casa::Stokes::StokesTypes> stokes = PolConverter::fromString(stokesStr);

 
              const int nTaylorTerms = parset.getInt32(*it+".nterms",1);                           
              ASKAPCHECK(nTaylorTerms>0, "Number of Taylor terms is supposed to be a positive number, you gave "<<
                         nTaylorTerms);
              // 
              ImageParamsHelper iph(*it);
              for (int order = 0; order<2*nTaylorTerms-1; ++order) { 
                   if (nTaylorTerms>1) {
                       // this is an MFS case, setup Taylor terms                       
                       iph.makeTaylorTerm(order);
                       ASKAPLOG_INFO_STR(logger,"Setting up Taylor term "<<order);
                   }
                   if (nfacets == 1) {
                       ASKAPLOG_INFO_STR(logger, "Setting up new empty image "<< iph.paramName());        
		               add(*params, iph.paramName(), direction, cellsize, shape, freq[0], freq[1], nchan,stokes);
		           } else {
		               // this is a multi-facet case
		               ASKAPLOG_INFO_STR(logger, "Setting up "<<nfacets<<" x "<<nfacets<<
		                                         " new empty facets for image "<< iph.paramName());	
		               const int facetstep = parset.getInt32(*it+".facetstep",casa::min(shape[0],shape[1]));
		               ASKAPCHECK(facetstep>0, "facetstep parameter is supposed to be positive, you have "<<facetstep);
		               ASKAPLOG_INFO_STR(logger, "Facet centers will be "<<facetstep<<
		                           " pixels apart, each facet size will be "<<shape[0]<<" x "<<shape[1]); 
		               add(*params, iph.paramName(), direction, cellsize, shape, freq[0], freq[1], nchan, stokes, nfacets,facetstep);
		           }
		      }
		      ASKAPLOG_INFO_STR(logger, "Number of channels = "<<nchan);
		      ASKAPLOG_INFO_STR(logger, "Polarisation planes correspond to "<<PolConverter::toString(stokes));		      
	     }
	  }
	  catch (const LOFAR::APSException &ex) {
	      throw AskapError(ex.what());
	  }
	}
    
    /// @brief load images according to the parset file
	/// @details This method is somewhat analogous to setUpImages, but it loads the images
	/// from the disk instead of setting them up from the scratch. Encapsulation of all loading
	/// of multiple images in a single method is required to provide a seamless handling of
	/// the faceted image.
	/// @param[in] params Images to be created here
	/// @param[in] parset a parset object to read the parameters from		
	void SynthesisParamsHelper::loadImages(const askap::scimath::Params::ShPtr& params, const LOFAR::ParameterSet &parset)
    {
      ASKAPDEBUGASSERT(params);
      try {
         const vector<string> images=parset.getStringVector("Names");
         for (vector<string>::const_iterator ci = images.begin(); ci != images.end(); ++ci) {
              ASKAPCHECK(ci->find("image") == 0, "All image names given in Names are supposed to start from 'image', you have "<<
                         *ci);
              // @todo add more checking that the image loaded from the disk conforms with the
              // parameters given in the parset file
              
              const int nfacets = parset.getInt32(*ci+".nfacets",1);
              ASKAPCHECK(nfacets>0, "Number of facets is supposed to be a positive number, you gave "<<nfacets);
              
              const int nTaylorTerms = parset.getInt32(*ci+".nterms",1);                                                      
              ASKAPCHECK(nTaylorTerms>0, "Number of Taylor terms is supposed to be a positive number, you gave "<<
                         nTaylorTerms);
              // 
              ImageParamsHelper iph(*ci);
              for (int order = 0; order<2*nTaylorTerms-1; ++order) { 
                   if (nTaylorTerms>1) {
                       // this is an MFS case, setup Taylor terms                       
                       iph.makeTaylorTerm(order);
                       ASKAPLOG_INFO_STR(logger,"Processing Taylor term "<<order);
                   }              
                   if (nfacets == 1) {
                       ASKAPLOG_INFO_STR(logger, "Reading image "<<iph.paramName());
                       SynthesisParamsHelper::loadImageParameter(*params,iph.paramName(),iph.paramName());
                   } else {
                       ASKAPLOG_INFO_STR(logger, "Loading multi-facet image image "<<iph.paramName());
                       SynthesisParamsHelper::getMultiFacetImage(*params,iph.paramName(),iph.paramName(), nfacets);
                   }
              }
         }            
         
	  }
	  catch (const LOFAR::APSException &ex) {
	      throw AskapError(ex.what());
	  }
    }
    
    void SynthesisParamsHelper::add(askap::scimath::Params& ip,
				    const string& name, const vector<string>& direction,
				    const vector<string>& cellsize, const vector<int>& shape,
				    const double freqmin, const double freqmax, const int nchan,
				    const casa::Vector<casa::Stokes::StokesTypes> &stokes)
    {
      int nx=shape[0];
      int ny=shape[1];
      ASKAPCHECK(cellsize.size() == 2, "Cell size should have exactly 2 parameters, you have "<<cellsize.size());
      ASKAPCHECK(direction.size() == 3, "Direction should have exactly 3 parameters, you have "<<direction.size());
      ASKAPCHECK(direction[2] == "J2000", "Only J2000 is implemented at the moment, you have requested "<<direction[2]);
      ASKAPCHECK(stokes.nelements()>=1, "At least one polarisation plane should be defined, you have defined none");      
      
      const double xcellsize =-1.0*convertQuantity(cellsize[0],"rad");
      const double ycellsize = convertQuantity(cellsize[1],"rad");
      
      const double ra = convertQuantity(direction[0],"rad");
      const double dec = convertQuantity(direction[1],"rad");
      
      /// @todo Do something with the frame info in direction[2]
      Axes axes;
      axes.add("RA", ra-double(nx)*xcellsize/2.0, ra+double(nx)*xcellsize/2.0);
      axes.add("DEC", dec-double(ny)*ycellsize/2.0, dec+double(ny)*ycellsize/2.0);
            
      axes.addStokesAxis(stokes);
      
      casa::Array<double> pixels(casa::IPosition(4, nx, ny, stokes.nelements(), nchan));
      pixels.set(0.0);
      axes.add("FREQUENCY", freqmin, freqmax);
      ip.add(name, pixels, axes);
    }
    
    /// @brief Add a parameter as a faceted image
    /// @param[in] ip Parameters
    /// @param[in] name Name of parameter
    /// @param[in] direction Strings containing [ra, dec, frame] (common tangent point)
    /// @param[in] cellsize Cellsize as a string e.g. [12arcsec, 12arcsec]
    /// @param[in] shape Number of pixels in RA and DEC for each facet e.g. [256, 256]
    /// @param[in] freqmin Minimum frequency (Hz)
    /// @param[in] freqmax Maximum frequency (Hz)
    /// @param[in] nchan Number of spectral channels
    /// @param[in] stokes Polarisation frame (vector of stokes enums)
    /// @param[in] nfacets Number of facets in each axis (assumed the same for both axes)
    /// @param[in] facetstep Offset in pixels between facet centres (equal to shape to
    ///            have no overlap between adjacent facets), assumed the same for both axes    
    void SynthesisParamsHelper::add(askap::scimath::Params& ip, const string& name, 
       const vector<string>& direction, 
       const vector<string>& cellsize, 
       const vector<int>& shape,
       const double freqmin, const double freqmax, const int nchan,
       const casa::Vector<casa::Stokes::StokesTypes> &stokes,
       const int nfacets, const int facetstep)
    {
      ASKAPDEBUGASSERT(nfacets>0);
      ASKAPDEBUGASSERT(facetstep>0);
      const int nx=shape[0];
      const int ny=shape[1];
      ASKAPCHECK(cellsize.size() == 2, "Cell size should have exactly 2 parameters, you have "<<cellsize.size());
      ASKAPCHECK(direction.size() == 3, "Direction should have exactly 3 parameters, you have "<<direction.size());
      ASKAPCHECK(direction[2] == "J2000", "Only J2000 is implemented at the moment, you have requested "<<direction[2]);
      ASKAPCHECK(stokes.nelements()>=1, "At least one polarisation plane should be defined, you have defined none");      
      
      const double xcellsize =-1.0*convertQuantity(cellsize[0],"rad");
      const double ycellsize = convertQuantity(cellsize[1],"rad");
      
      const double ra = convertQuantity(direction[0],"rad");
      const double dec = convertQuantity(direction[1],"rad");

      // zero-filled array is the same for all facets as it is copied inside Params
      // class
      casa::Array<double> pixels(casa::IPosition(4, nx, ny, stokes.nelements(), nchan));
      pixels.set(0.0);
      
      // have to create facet parameter in two steps as it could be 
      // a Taylor decomposition 
      ImageParamsHelper iph(name);
      // a loop over facets
      for (int ix=0;ix<nfacets;++ix) {
           for (int iy=0;iy<nfacets;++iy) {

                // for debugging to avoid running out of memory
                //if (ix!=1 || iy!=1) continue;
                      
                const double raCentre = ra+facetstep*xcellsize*double(ix-nfacets/2);
                const double decCentre = dec+facetstep*ycellsize*double(iy-nfacets/2);
                
                /// @todo Do something with the frame info in direction[2]
                Axes axes;
                axes.add("RA", raCentre-double(nx)*xcellsize/2.0, raCentre+double(nx)*xcellsize/2.0);
                axes.add("DEC", decCentre-double(ny)*ycellsize/2.0, decCentre+double(ny)*ycellsize/2.0);
      
                // we need to ship around the tangent point somehow as it affects the way this
                // faceted images are used. One way is to specify an extra fixed parameter and another
                // is to attach it to each facet itself. The latter has an advantage for parallel
                // processing as all necessary info is readily available with any facet, although
                // there is some minor duplication of the data.
                // 
                // In the future, we may allow having a keyword-type axis in the Axes object which 
                // is essentially an axis with single pixel only. At this stage, we will just set up
                // a normal axis with the same start and stop values
                axes.add("RA-TANGENT",ra,ra);
                axes.add("DEC-TANGENT",dec,dec);
                // another fake axis to know which part of the image actually contains useful
                // information. Otherwise, this parameter is impossible to derive from a
                // single facet only (and we may need, e.g., to clip the outer edges in each
                // major cycle)
                axes.add("FACETSTEP",double(facetstep),double(facetstep));
      
                axes.addStokesAxis(stokes);
      
                axes.add("FREQUENCY", freqmin, freqmax);
                
                // add/change facet indices
                iph.makeFacet(ix,iy);
                ip.add(iph.paramName(), pixels, axes);    
                
                // for debigging
                //if (ix!=0 || iy!=0) ip.fix(iph.paramName());
           }
      }
      
    }
    
    /// @brief helper method to clip the outer edges of the image
    /// @details For experiments with faceting we want to be able to clip the outer
    /// edges of each model image (beyond the facet step) to zero. This is one way to
    /// reduce cross-talk problem (when facets overlap). This method encapsulates all
    /// the required operations. It takes facet step from the fake image axis FACETSTEP
    /// and does nothing if such a parameter doesn't exist or is larger than the shape
    /// along the directional axes.
    /// @param[in] ip parameters
    /// @param[in] name full name of the image (i.e. with .facet.x.y for facets)
    void SynthesisParamsHelper::clipImage(const askap::scimath::Params &ip, const string &name)
    {
       const askap::scimath::Axes axes(ip.axes(name));
       if (!axes.has("FACETSTEP")) {
           // it is not a facet image, do nothing.
           return;
       }
       const int facetStep = int(axes.start("FACETSTEP"));
       ASKAPDEBUGASSERT(facetStep>0);
       casa::Array<double> pixels = ip.value(name);
       const casa::IPosition shape = pixels.shape();
       ASKAPDEBUGASSERT(shape.nelements()>=2);
       casa::IPosition end(shape);
       for (uint index=0;index<end.nelements();++index) {
            ASKAPDEBUGASSERT(end[index]>=1);
            end[index]--;
       }

       if (shape[0]>facetStep+1) {
           // need clipping along the first axis
           casa::IPosition start(shape.nelements(),0);
           end[0] = (shape[0]-facetStep)/2-1;
           end[1] = shape[1]-1; // although this step is strictly speaking unnecessary
           pixels(start,end).set(0.); 
           
           end[0] = shape[0]-1;
           start[0] = (shape[0]+facetStep)/2;
           pixels(start,end).set(0.);
       }
       
       if (shape[1]>facetStep+1) {
           // need clipping along the second axis
           casa::IPosition start(shape.nelements(),0);
           start[0]=(shape[0]-facetStep)/2;
           end[0]=(shape[0]+facetStep)/2;
           if (start[0]<0) {
               start[0] = 0;
           }
           if (end[0]+1 > shape[0]) {
               end[0] = shape[0] - 1;
           }
           start[1] = 0;
           end[1] = (shape[1]-facetStep)/2-1;
           pixels(start,end).set(0.);
           
           start[1] = (shape[1]+facetStep)/2;
           end[1] = shape[1]-1;
           pixels(start,end).set(0.);
       }       
    }
    
    /// @brief helper method to store restoring beam for an image
    /// @details We have to carry restore beam parameters together with the image.
    /// This is done by creating 2 fake axes MAJMIN (with start = maj and end = min)
    /// and PA with position angle. All angles are given in radians. The presence of
    /// this fake axes distinguishes a restored image from model image. Restored image
    /// will have units Jy/beam instead of Jy/pixel and beam info will be added to the
    /// image (in saveImageParamter).
    /// @param[in] ip parameters
    /// @param[in] name full name of the parameter representing this image
    /// @param[in] beam major, minor axes and position anlge as quantities
    void SynthesisParamsHelper::setBeam(askap::scimath::Params &ip, const string &name,
                            const casa::Vector<casa::Quantum<double> > &beam)
    {
       askap::scimath::Axes &axes = ip.axes(name);
       ASKAPDEBUGASSERT(beam.nelements()>=3);
       if (axes.has("MAJMIN")) {
           axes.update("MAJMIN",beam[0].getValue("rad"),beam[1].getValue("rad"));
       } else {
           axes.add("MAJMIN",beam[0].getValue("rad"),beam[1].getValue("rad"));
       }
       
       if (axes.has("PA")) {
           axes.update("PA",beam[2].getValue("rad"),0.);
       } else {
           axes.add("PA",beam[2].getValue("rad"),0.);
       }       
    }
    
    /// @brief add a parameter as a merged faceted image
    /// @details Each facet is represented by a number of independent parameters with
    /// the appropriate names. This method looks at the coordinate systems of all
    /// subimages and forms a parameter representing merged image. It can then be
    /// populated with the data from the appropriate slices.
    /// @param[in] ip parameters
    /// @param[in] name Base name of the parameter (i.e. without .facet.0.0)
    /// @param[in] nfacets number of facets defined
    void SynthesisParamsHelper::add(askap::scimath::Params& ip, const string &name,
              const int nfacets) 
    {
       ASKAPDEBUGASSERT(nfacets>1);
       // no consistency check of the coordinate systems of individual patches at this stage
       
       // create image handler in two steps because name may contain a taylor-order suffix
       ImageParamsHelper iph(name);
       iph.makeFacet(0,0);
       const askap::scimath::Axes axes(ip.axes(iph.paramName()));
       ASKAPDEBUGASSERT(axes.has("RA") && axes.has("DEC") && axes.has("RA-TANGENT") &&
                        axes.has("DEC-TANGENT") && axes.has("STOKES") && axes.has("FREQUENCY"));
       const casa::IPosition shape = ip.value(iph.paramName()).shape();
       ASKAPDEBUGASSERT(shape.nelements()>=2);
       const double raCellSize = (axes.end("RA")-axes.start("RA"))/double(shape[0]);
       const double decCellSize = (axes.end("DEC")-axes.start("DEC"))/double(shape[1]);
       const double facetFactor = double(-nfacets/2);
       ASKAPDEBUGASSERT(facetFactor!=0.);
       const double raFacetStep = ((axes.start("RA")+axes.end("RA"))/2.-axes.start("RA-TANGENT"))/
                            raCellSize/facetFactor;
       const double decFacetStep = ((axes.start("DEC")+axes.end("DEC"))/2.-axes.start("DEC-TANGENT"))/
                            decCellSize/facetFactor;
       ASKAPCHECK(casa::abs(raFacetStep-decFacetStep)<0.5, "facet steps deduced from "<<
                  iph.paramName()<<" are notably different for ra and dec axes. Should be the same integer number");
       const int facetSize = int(raFacetStep);
       
       Axes newAxes(axes);       
       newAxes.update("RA",newAxes.start("RA-TANGENT")+facetSize*raCellSize*(double(-nfacets/2)-0.5),
                newAxes.start("RA-TANGENT")+facetSize*raCellSize*(double(nfacets-1-nfacets/2)+0.5));
       newAxes.update("DEC",newAxes.start("DEC-TANGENT")+facetSize*decCellSize*(double(-nfacets/2)-0.5),
                newAxes.start("DEC-TANGENT")+facetSize*decCellSize*(double(nfacets-1-nfacets/2)+0.5));
       // add a fake axis to peserve facetSize for futher operations with the merged image
       // without it we would have to redetermine this value
       if (newAxes.has("FACETSTEP")) {
           newAxes.update("FACETSTEP", raFacetStep, decFacetStep);
       } else {
           newAxes.add("FACETSTEP", raFacetStep, decFacetStep);
       }
       
       casa::IPosition newShape(shape);
       newShape[0]=facetSize*nfacets;
       newShape[1]=facetSize*nfacets;
 
       casa::Array<double> pixels(newShape);
       pixels.set(0.0);
       ip.add(iph.taylorName(), pixels, newAxes);
    }

    /// @brief obtain an array corresponding to a single facet of a merged faceted image
    /// @details Each facet is represented by a number of independent parameters with
    /// the names containing .facet.x.y at the end. One of the add methods can add a 
    /// parameter representing merged image (with the name without any suffixes). This 
    /// method allows to translate the name of the facet (with suffixes) into a slice of
    /// the merged array corresponding to this particular facet. The suffixes are removed
    /// automatically to locate the merged image. This is the core method necessary for 
    /// merging individual facets together (which happens inside ImageRestoreSolver).
    /// @param[in] ip parameters
    /// @param[in] name name of the facet parameter (with suffix like .facet.0.0)
    /// @return an array of doubles representing a subimage of the merged image
    casa::Array<double> SynthesisParamsHelper::getFacet(askap::scimath::Params &ip, const string &name) 
    {
      ASKAPDEBUGASSERT(ip.has(name));
      // parse the name
      ImageParamsHelper iph(name);
      // name with the suffixes related to facets removed (and taylor suffix preserved if present)
      const std::string mergedName = iph.taylorName();
      ASKAPCHECK(ip.has(mergedName), "Merged image ("<<mergedName<<") doesn't exist");
      // there is no consistency check that the given facet corresponds to this particular
      // merged image and coordinate systems match. 
      
      // now find blc and trc of the patch inside the big image
      const askap::scimath::Axes axes(ip.axes(mergedName));
      ASKAPDEBUGASSERT(axes.has("FACETSTEP"));
      ASKAPCHECK(casa::abs(axes.start("FACETSTEP")-axes.end("FACETSTEP"))<0.5, "facet steps extracted from "<<
                 iph.name()<<" are notably different for ra and dec axes. Should be the same integer number");
      const int facetStep = int(axes.start("FACETSTEP"));

      casa::Array<double> mergedImage = ip.value(mergedName);
      casa::IPosition blc(mergedImage.shape());
      casa::IPosition trc(mergedImage.shape());
      ASKAPDEBUGASSERT(blc.nelements()>=2);
      // adjust extra dimensions
      for (size_t i=2;i<blc.nelements();++i) {
           blc[i] = 0;
           ASKAPDEBUGASSERT(trc[i]!=0);
           trc[i] -= 1;           
      }
      
      casa::IPosition patchShape = ip.value(name).shape();
      ASKAPDEBUGASSERT(patchShape.nelements()>=2);
      ASKAPDEBUGASSERT((facetStep<=patchShape[0]) && (facetStep<=patchShape[1]));
      
      ASKAPDEBUGASSERT(facetStep>=1);
      blc[0] = iph.facetX()*facetStep;
      trc[0] = blc[0]+facetStep-1;
      blc[1] = iph.facetY()*facetStep;
      trc[1] = blc[1]+facetStep-1;

      /*
      const casa::DirectionCoordinate csPatch = directionCoordinate(ip,name);
      const casa::DirectionCoordinate csFull = directionCoordinate(ip,iph.name());
      casa::Vector<double> world(2);
      // first get blc
      casa::Vector<double> blcPixel(2);
      blcPixel(0)=double((patchShape[0]-facetStep)/2);
      blcPixel(1)=double((patchShape[1]-facetStep)/2);
      std::cout<<blcPixel<<endl;
      csPatch.toWorld(world,blcPixel);
      csFull.toPixel(blcPixel,world);
      std::cout<<blcPixel<<endl;

      // now get trc
      casa::Vector<double> trcPixel(2);
      trcPixel[0]=double((patchShape[0]+facetStep)/2-1);
      trcPixel[1]=double((patchShape[1]+facetStep)/2-1);
      ASKAPDEBUGASSERT((trcPixel[0]>0) && (trcPixel[1]>0));
      std::cout<<trcPixel<<endl;
      csPatch.toWorld(world,trcPixel);
      csFull.toPixel(trcPixel,world);
      std::cout<<trcPixel<<endl;
      for (size_t dim=0;dim<2;++dim) {
           const int pix1 = int(blcPixel[dim]);
           const int pix2 = int(trcPixel[dim]);
           blc[dim] = pix1>pix2 ? pix2 : pix1;
           trc[dim] = pix1>pix2 ? pix1 : pix2;
      }
      */
      // ready to make a slice
      //std::cout<<blc<<" "<<trc<<" "<<facetStep<<" "<<mergedImage.shape()<<std::endl;
      ASKAPDEBUGASSERT((trc[0]-blc[0]+1 == facetStep) && (trc[1]-blc[1]+1 == facetStep));
      return mergedImage(blc,trc);
    }
    
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. [8.0arcsec,8.0arcsec]. This method allows
    /// to parse vector of strings corresponding to such parameter and return
    /// a vector of double values in the required units.
    /// @param[in] strval input vector of strings
    /// @param[in] unit required units (given as a string)
    /// @return vector of doubles with converted values
    std::vector<double> SynthesisParamsHelper::convertQuantity(const std::vector<std::string> &strval,
                       const std::string &unit)
    {
       std::vector<double> result(strval.size());
       std::vector<std::string>::const_iterator inIt = strval.begin();
       for (std::vector<double>::iterator outIt = result.begin(); inIt != strval.end(); 
                                                ++inIt,++outIt) {
            ASKAPDEBUGASSERT(outIt != result.end());                                       
            *outIt = convertQuantity(*inIt,unit);
       }
       return result;
    }
    
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. 8.0arcsec. This method allows
    /// to parse a single string corresponding to such a parameter and return
    /// a double value converted to the requested units.
    /// @param[in] strval input string
    /// @param[in] unit required units (given as a string)
    /// @return converted value
    double SynthesisParamsHelper::convertQuantity(const std::string &strval,
                       const std::string &unit)
    {
       casa::Quantity q;
      
       casa::Quantity::read(q, strval);
       return q.getValue(casa::Unit(unit));
    }
    
    /// @brief save a 2D array as a CASA image
    /// @details This method is intended to be used largely for debugging. To save image from
    /// parameter class use saveImageParameter method
    /// @param[in] imagename name of the output image file
    /// @param[in] arr input array
    void SynthesisParamsHelper::saveAsCasaImage(const std::string &imagename, const casa::Array<casa::Float> &arr)
    {
      size_t nDim = arr.shape().nonDegenerate().nelements();
      casa::Vector<casa::String> names(2);
      ASKAPASSERT(nDim>=2);
      names[0]="x"; names[1]="y";
      casa::Vector<double> increment(2 ,1.);
      
      casa::Matrix<double> xform(2,2,0.);
      xform.diagonal() = 1.;
      casa::LinearCoordinate linear(names, casa::Vector<casa::String>(2,"pixel"),
             casa::Vector<double>(2,0.),increment, xform, casa::Vector<double>(2,0.));
      
      casa::CoordinateSystem coords;
      coords.addCoordinate(linear);
      
      for (size_t dim=2; dim<nDim; ++dim) {
           casa::Vector<casa::String> addname(1);
           addname[0]="addaxis"+utility::toString<size_t>(dim-1);
           casa::Matrix<double> xform(1,1,1.);
           casa::LinearCoordinate lc(addname, casa::Vector<casa::String>(1,"pixel"),
              casa::Vector<double>(1,0.), casa::Vector<double>(1,1.),xform, 
              casa::Vector<double>(1,0.));
           coords.addCoordinate(lc);
      }
      casa::PagedImage<casa::Float> result(casa::TiledShape(arr.nonDegenerate().shape()), coords, imagename);
      casa::ArrayLattice<casa::Float> lattice(arr.nonDegenerate());
      result.copyData(lattice);
    }
    
    void SynthesisParamsHelper::saveImageParameter(const askap::scimath::Params& ip, const string& name,
						const string& imagename)
    {     
      const casa::Array<double> imagePixels(ip.value(name));
      ASKAPDEBUGASSERT(imagePixels.ndim()!=0);
      const casa::CoordinateSystem imageCoords(coordinateSystem(ip,name));
      
      casa::Array<float> floatImagePixels(imagePixels.shape());
      casa::convertArray<float, double>(floatImagePixels, imagePixels);
      
      imageHandler().create(imagename, floatImagePixels.shape(), imageCoords);
      imageHandler().write(imagename, floatImagePixels);
        
      const Axes &axes = ip.axes(name);
      if (axes.has("MAJMIN")) {
          // this is a restored image with beam parameters set
          ASKAPCHECK(axes.has("PA"),"PA axis should always accompany MAJMIN");
          imageHandler().setUnits(imagename, "Jy/beam");          
          imageHandler().setBeamInfo(imagename, axes.start("MAJMIN"), axes.end("MAJMIN"),
                                     axes.start("PA"));
      } else {
          imageHandler().setUnits(imagename, "Jy/pixel");
      }
    }
    
    /// @brief obtain image handler
    /// @details For some operations it may be necessary to access the (global) instance of the
    /// image handler. This method allows that. An exception is thrown if no image handler has
    /// been previously set up.
    /// @return a reference to image handler
    IImageAccess& SynthesisParamsHelper::imageHandler()
    {
      ASKAPCHECK(theirImageAccessor, "setUpImageHandler has to be called before any read/write operation");
      return *theirImageAccessor;
    }
    
    /// @brief setup image handler
    /// @details This method uses the factory to setup a helper class handling the
    /// operations with images (default is casa). It is necessary to call this method
    /// at least once before any read or write operation can happen.
    /// @param[in] parset a parset file containing parameters describing which image handler to use
    /// @note The key parameter describing the image handler is "imagetype". By default, the
    /// casa image handler is created (however, a call to this method is still required)
    void SynthesisParamsHelper::setUpImageHandler(const LOFAR::ParameterSet &parset)
    {
      theirImageAccessor = imageAccessFactory(parset);
    }
 
    
    void SynthesisParamsHelper::loadImageParameter(askap::scimath::Params& ip, const string& name,
						 const string& imagename)
    {
      casa::Array<float> pixels = imageHandler().read(imagename);
      casa::Array<double> imagePixels(pixels.shape());
      casa::convertArray<double, float>(imagePixels, pixels);
      
      casa::CoordinateSystem imageCoords = imageHandler().coordSys(imagename);
      
      /// Fill in the axes information
      Axes axes;
      /// First do the direction
      int whichDir=imageCoords.findCoordinate(Coordinate::DIRECTION);
      ASKAPCHECK(whichDir>-1, "No direction coordinate present in model");
      casa::DirectionCoordinate radec(imageCoords.directionCoordinate(whichDir));
      
      casa::Vector<casa::String> units(2);
      units.set("rad");
      radec.setWorldAxisUnits(units);
      
      casa::Vector<double> refPix(radec.referencePixel());
      casa::Vector<double> refInc(radec.increment());
      casa::Vector<double> refValue(radec.referenceValue());
      
      casa::Vector<double> start(2);
      casa::Vector<double> end(2);
      
      for (int i=0;i<2;++i) {
	     start(i)=refValue(i)-refInc(i)*(refPix(i));
	     end(i)=refValue(i)+refInc(i)*(double(imagePixels.shape()(i))-refPix(i));
	  }
      
      axes.add("RA", start(0), end(0));
      axes.add("DEC", start(1), end(1));
      
      int whichStokes = imageCoords.findCoordinate(Coordinate::STOKES);
      if (whichStokes<0) {
          const casa::Vector<casa::Stokes::StokesTypes> dummyStokes(1,casa::Stokes::I);
          axes.addStokesAxis(dummyStokes);
      } else {
          casa::StokesCoordinate sc(imageCoords.stokesCoordinate(whichStokes));
          const casa::Vector<casa::Int> stokesAsInt = sc.stokes();
          casa::Vector<casa::Stokes::StokesTypes> stokes(stokesAsInt.nelements());
          for (casa::uInt pol=0; pol<stokes.nelements(); ++pol) {
               stokes[pol] = casa::Stokes::StokesTypes(stokesAsInt[pol]);
          }
          axes.addStokesAxis(stokes);
      }
      
      int whichSpectral=imageCoords.findCoordinate(Coordinate::SPECTRAL);
      ASKAPCHECK(whichSpectral>-1, "No spectral coordinate present in model");
      casa::SpectralCoordinate freq(imageCoords.spectralCoordinate(whichSpectral));
      int nChan=imagePixels.shape()(whichSpectral);
      double startFreq, endFreq;
      freq.toWorld(startFreq, 1.0);
      freq.toWorld(endFreq, double(nChan));
      axes.add("FREQUENCY", startFreq, endFreq);
      
      ip.add(name, imagePixels.reform(IPosition(4, imagePixels.shape()(0), imagePixels.shape()(1), 1, nChan)),
	     axes);
      
    }
    
    /// @brief Get parameters corresponding to all facets from a CASA image
    /// @param[in] ip Parameters
    /// @param[in] name Base name of the parameter (.facet.x.y will be added)
    /// @param[in] fileName Base name of the image file (.facet.x.y will be added)
    /// @param[in] nfacets Number of facets on each axis (assumed the same for both axes)    
    void SynthesisParamsHelper::getMultiFacetImage(askap::scimath::Params &ip, const string &name,
           const string &fileName, const int nfacets)
    {
      ASKAPCHECK(nfacets>0, "The number of facets is supposed to be positive, you have "<<nfacets);
      // create helper in two steps because the name may represent a Taylor term
      ImageParamsHelper iph(name);
      for (int ix=0; ix<nfacets; ++ix) {
           for (int iy=0; iy<nfacets; ++iy) {
                // assign facet indices to the helper
                iph.makeFacet(ix,iy);
                const std::string paramName = iph.paramName();
                loadImageParameter(ip,paramName,paramName);                            
           }
      }
    }
    
    boost::shared_ptr<casa::TempImage<float> >
    SynthesisParamsHelper::tempImage(const askap::scimath::Params& ip,
				     const string& name)
    {
      const casa::Array<double> imagePixels(ip.value(name));
      
      casa::CoordinateSystem imageCoords(coordinateSystem(ip, name));
      
      boost::shared_ptr<casa::TempImage<float> >
	im(new casa::TempImage<float> (TiledShape(imagePixels.shape()),
				       imageCoords));
      
      im->setUnits("Jy/pixel");
      
      casa::Array<float> floatImagePixels(imagePixels.shape());
      casa::convertArray<float, double>(floatImagePixels, imagePixels);
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      im->copyData(latImagePixels);
      return im;
    }
            
    casa::CoordinateSystem
    SynthesisParamsHelper::coordinateSystem(const askap::scimath::Params& ip,
					    const string& name)
    {
      const Axes axes(ip.axes(name));
      
      casa::DirectionCoordinate radec(directionCoordinate(ip, name));
      
      casa::CoordinateSystem imageCoords;
      imageCoords.addCoordinate(radec);
      
      // default is a dummy stokes coordinate with only stokes I present
      casa::Vector<int> iquv(1);
      iquv(0) = Stokes::I;
      if (axes.has("STOKES")) {
          casa::Vector<casa::Stokes::StokesTypes> stokes = axes.stokesAxis();
          ASKAPDEBUGASSERT(stokes.nelements()>=1);
          iquv.resize(stokes.nelements());
          for (size_t pol=0; pol<stokes.nelements(); ++pol) {
               iquv[pol] = int(stokes[pol]);
          }   
      }
      
      casa::StokesCoordinate stokes(iquv);
      imageCoords.addCoordinate(stokes);
      
      int nchan=ip.value(name).shape()(2);
      double restfreq = 0.0;
      double crpix = (nchan-1)/2;
      double crval = (axes.start("FREQUENCY")+axes.end("FREQUENCY"))/2.0;
      double cdelt = (axes.end("FREQUENCY")-axes.start("FREQUENCY"))/double(nchan);
      casa::SpectralCoordinate freq(casa::MFrequency::TOPO, crval, cdelt, crpix, restfreq);
      imageCoords.addCoordinate(freq);
      
      return imageCoords;
    }
    
    casa::DirectionCoordinate
    SynthesisParamsHelper::directionCoordinate(const askap::scimath::Params& ip,
					       const string& name)
    {
      const Axes axes(ip.axes(name));
      ASKAPCHECK(axes.has("RA-TANGENT") == axes.has("DEC-TANGENT"), 
          "Either both RA and DEC have to be defined for a tangent point or none of them");
      
      casa::Matrix<double> xform(2,2);
      xform = 0.0; xform.diagonal() = 1.0;
      int nx=ip.value(name).shape()(0);
      int ny=ip.value(name).shape()(1);
      const casa::Quantum<double> centreLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
      const casa::Quantum<double> centreLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");
      
      const casa::Quantum<double> incLon((axes.end("RA")-axes.start("RA"))/double(nx), "rad");
      const casa::Quantum<double> incLat((axes.end("DEC")-axes.start("DEC"))/double(ny), "rad");
      
      if (!axes.has("RA-TANGENT")) {
          // this is not faceting, centre of the image is a tangent point
          const casa::DirectionCoordinate radec(MDirection::J2000,Projection(Projection::SIN), 
                                     centreLon, centreLat, incLon, incLat, xform, nx/2, ny/2);      
          return radec;
      }
      // we have to deal with the user specified tangent point here as it may be 
      // different from the image centre
      const casa::Quantum<double> tangentLon(axes.start("RA-TANGENT"), "rad");
      const casa::Quantum<double> tangentLat(axes.start("DEC-TANGENT"), "rad");
      // need to find reference pixel, do it with a temporary coordinate class by
      // getting the world coordinates for the image centre
      const casa::DirectionCoordinate temp(MDirection::J2000,Projection(Projection::SIN), 
                                      tangentLon, tangentLat, incLon, incLat, xform, 0, 0);      
      casa::Vector<casa::Double> pixel;
      temp.toPixel(pixel,casa::MVDirection(centreLon, centreLat));
      ASKAPDEBUGASSERT(pixel.nelements()==2);
      const casa::DirectionCoordinate radec(MDirection::J2000,Projection(Projection::SIN), 
                                   tangentLon, tangentLat, incLon, incLat, xform, 
                                   double(nx)/2.-pixel[0], double(ny)/2.-pixel[1]);      
      return radec;
    }
    
    void SynthesisParamsHelper::update(askap::scimath::Params& ip, const string& name,
				       const casa::ImageInterface<float>& im)
    {
      /// This next copy should be a reference unless it is too big
      casa::Array<float> floatImagePixels(im.shape());
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      latImagePixels.copyData(im);
      
      casa::Array<double> imagePixels(im.shape());
      casa::convertArray<double, float>(imagePixels, floatImagePixels);
      ip.update(name, imagePixels);
    }
    
    /// @brief check whether parameter list defines at least one component
    /// @details Parameter lists can have a mixture of components and
    /// images defined. This method checks whether the given parameter
    /// list defines at least one component.
    /// @param[in] params a shared pointer to the parameter container
    /// @return true, if at least one component is defined
    bool SynthesisParamsHelper::hasComponent(const askap::scimath::Params::ShPtr &params)
    {
       ASKAPDEBUGASSERT(params);
       return params->completions("flux.i").size()!=0;
    }
    
    /// @brief check whether parameter list defines at least one image
    /// @details Parameter lists can have a mixture of components and
    /// images defined. This method checks whether the given parameter
    /// list defines at least one image.
    /// @param[in] params a shared pointer to the parameter container
    /// @return true, if at least one image is defined
    bool SynthesisParamsHelper::hasImage(const askap::scimath::Params::ShPtr &params)
    {
       ASKAPDEBUGASSERT(params);
       return params->completions("image").size()!=0;
    }
    
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
    void SynthesisParamsHelper::listFacets(const std::vector<std::string> &names,
                          std::map<std::string, int> &facetmap)
    {       
       // temporary maps, just to check that no facets were missed
       std::map<std::string, std::set<int> >  tempMapX;
       std::map<std::string, std::set<int> >  tempMapY;
       
       for (std::vector<std::string>::const_iterator ci = names.begin(); ci!=names.end(); ++ci) {
            ImageParamsHelper iph(*ci);
            // name with the facet-related suffixes removed (and taylor suffix preserved, if present)
            const std::string baseName = iph.taylorName();
            if (iph.isFacet()) {
                tempMapX[baseName].insert(iph.facetX());
                tempMapY[baseName].insert(iph.facetY());
                facetmap[baseName] = 0; // a flag that we need to figure out the exact number later   
            } else {
                // this is not a faceted image, just add it to the final list
                facetmap[baseName] = 1; // one facet                
            } 
       }
       for (std::map<std::string, int>::iterator it = facetmap.begin(); it!=facetmap.end(); ++it) {
            if (it->second == 0) {  
                ASKAPDEBUGASSERT(hasValue(tempMapX, it->first));
                ASKAPDEBUGASSERT(hasValue(tempMapY, it->first));
                
                // the code below assumes equal number of facets in both axes. It should be
                // modified slightly to lift this restriction. 
                
                ASKAPDEBUGASSERT(tempMapX[it->first].size());
                ASKAPDEBUGASSERT(tempMapY[it->first].size());
                
                
                const int maxFacetX = *(std::max_element(tempMapX[it->first].begin(),
                                      tempMapX[it->first].end()));
                const int maxFacetY = *(std::max_element(tempMapY[it->first].begin(),
                                      tempMapY[it->first].end()));
                const int nFacets = (maxFacetX > maxFacetY ? maxFacetX : maxFacetY)+1;      
                
                // doing checks
                for (int facet = 0; facet<nFacets; ++facet) {
                     ASKAPCHECK(hasValue(tempMapX[it->first],facet), "Facet "<<facet<<
                          " is missing for the first axis");                          
                     ASKAPCHECK(hasValue(tempMapY[it->first],facet), "Facet "<<facet<<
                          " is missing for the second axis");
                }
                
                it->second = nFacets;
            }
       }
    }
    
    /// @brief A helper method to build a list of images representing Taylor terms
    /// @details Different Taylor terms in the multi-frequency algorithm are 
    /// represented by parameters named like "image.fieldname.taylor.0". This method
    /// reads a supplied vector of names (may be just free parameters or all names
    /// available) and builds a map of the actual image name (without suffixes) and
    /// the number of Taylor orders encountered. It also does check that all orders
    /// starting from 0 are present and throws an exception if it is not the case.
    /// To some extent this method is similar to listFacets, but is intended for
    /// Taylor terms.
    /// @param[in] names parameter names to work with
    /// @param[out] taylormap a map of (possibly truncated names) and the number of
    ///             Taylor terms (1 means no decomposition into Taylor series, i.e. no MFS)
    /// @note 1. taylormap.size()<=names.size() after a call to this method, if it was originally
    ///          empty
    ///       2. This method just adds new elements to the taylormap without erasing the
    ///          existing information.
    void SynthesisParamsHelper::listTaylor(const std::vector<std::string> &names,
                           std::map<std::string, int> &taylormap)
    {
       // temporary map, just to check that no Taylor terms are missed
       // (parameters may not come in any particular order)
       std::map<std::string, std::set<int> >  tempMap;
       
       for (std::vector<std::string>::const_iterator ci = names.begin(); ci!=names.end(); ++ci) {
            ImageParamsHelper iph(*ci);
            // name with the taylor-related suffixes removed (and facet suffixes preserved, if present)
            const std::string baseName = iph.facetName();
            if (iph.isTaylorTerm()) {
                // this is a Taylor term, we need to remember all orders sited for this base name
                tempMap[baseName].insert(iph.order());
                taylormap[baseName] = 0; // just a flag, we need to figure out the exact number later
            } else {
                // this is not an MFS'ed image, add it to the final list
                taylormap[baseName] = 1; // single order
            }
       }

       for (std::map<std::string, int>::iterator it = taylormap.begin(); it!=taylormap.end(); ++it) {
            if (it->second == 0) {  
                // This is the MFS case, need to figure out the exact number of Taylor terms
                ASKAPDEBUGASSERT(hasValue(tempMap, it->first));                                
                ASKAPDEBUGASSERT(tempMap[it->first].size());                
                
                const int nTaylorTerms = *(std::max_element(tempMap[it->first].begin(),
                                      tempMap[it->first].end())) + 1;
                
                // doing checks
                for (int order = 0; order<nTaylorTerms; ++order) {
                     ASKAPCHECK(hasValue(tempMap[it->first],order), "Taylor term "<<order<<
                          " is missing for the image "<<it->first);
                }
                
                it->second = nTaylorTerms;
            }
       }     
    }                       
    
    
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
    void SynthesisParamsHelper::copyComponent(const askap::scimath::Params::ShPtr &params,
           const LOFAR::ParameterSet &parset, 
           const std::string &srcName, const std::string &baseKey)
    {
       ASKAPDEBUGASSERT(params);
       // first, create a list of parameters describing the component
       // if the value of the map is true, the parameter is mandatory
       // (in the future we may have a more flexible code here filling this map)
       std::map<std::string, bool>  parameterList;
       parameterList["flux.i"] = true;
       parameterList["direction.ra"] = true;
       parameterList["direction.dec"] = true;
       parameterList["shape.bmaj"] = false;
       parameterList["shape.bmin"] = false;
       parameterList["shape.bpa"] = false;
       
       // now iterate through all parameters
       for (std::map<std::string, bool>::const_iterator ci = parameterList.begin();
            ci!=parameterList.end(); ++ci) {
            const std::string parName = baseKey+srcName+"."+ci->first;
            if (parset.isDefined(parName)) {
                const double val = parset.getDouble(parName);
                params->add(ci->first+"."+srcName, val);
            } else {
                if (ci->second) {
                    ASKAPTHROW(AskapError, "Parameter "<<parName<<
                           " is required to define the source "<<srcName<<
                           ", baseKey="<<baseKey);
                }
            }
       }
    }
  }
}

