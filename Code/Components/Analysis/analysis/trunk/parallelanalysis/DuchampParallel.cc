/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
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

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <APS/Exceptions.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

// boost includes
#include <boost/shared_ptr.hpp>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <parallelanalysis/DuchampParallel.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <duchamp/duchamp.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object3D.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelanalysis");

using namespace std;
using namespace askap;
using namespace askap::cp;

using namespace duchamp;

namespace askap
{
  namespace analysis
  {

    bool DuchampParallel::is2D()
    {
      /// @details Check whether the image is 2-dimensional, by
      /// looking at the dim array in the Cube object, and counting
      /// the number of dimensions that are greater than 1
      int numDim=0;
      long *dim = this->itsCube.getDimArray();
      for(int i=0;i<this->itsCube.getNumDim();i++) if(dim[i]>1) numDim++;
      return numDim<=2;
    }
    
    //**************************************************************//

    DuchampParallel::DuchampParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : AskapParallel(argc, argv)
    {
      /// @details The constructor reads parameters from the parameter
      /// set parset. This set can include Duchamp parameters, as well
      /// as particular cduchamp parameters such as masterImage and
      /// sectionInfo.

      // First do the setup needed for both workers and master

      this->itsCube.pars() = parseParset(parset);

      this->itsIsFITSFile = parset.getBool("isFITS", true);
      this->itsFlagDoFit = parset.getBool("doFit", false);
      this->itsSummaryFile = parset.getString("summaryFile", "duchamp-Summary.txt");
      this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-Results-Fits.ann");

      this->itsFitter = sourcefitting::FittingParameters(parset.makeSubset("Fitter."));


      // Now read the correct image name according to worker/master state.
      if(isMaster()){
	this->itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );
	this->itsCube.pars().setFlagRobustStats(false);
	if(isParallel()) this->itsSectionList = makeSubImages(this->itsNNode-1,parset);
	this->itsImage = substitute(parset.getString("image"));
	this->itsCube.pars().setImageFile(this->itsImage);

      }

      if(isWorker()) {
 	 this->itsImage = substitute(parset.getString("image"));
	if(isParallel()) this->itsImage = getSubImageName(this->itsImage,itsRank-1,itsNNode-1);
	
	this->itsCube.pars().setImageFile(this->itsImage);
// 		splitImage(parset);

	// get the list of subsections so that all nodes can access it.
	if(isParallel()){
 	  this->itsCube.pars().setSubsection( getSection(itsRank-1,parset).getSection() );
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<itsRank-1<<" is using subsection " << this->itsCube.pars().getSubsection());
	}

	this->itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );	
	this->itsCube.pars().setFlagRobustStats( parset.getBool("flagRobust",true) );
      }

      if(this->itsCube.pars().getFlagLog()){
	ASKAPLOG_INFO_STR(logger, "Setting up logfile " << this->itsCube.pars().getLogFile() );
	std::ofstream logfile(this->itsCube.pars().getLogFile().c_str());
	logfile << "New run of the CDuchamp sourcefinder: ";
	time_t now = time(NULL);
	logfile << asctime( localtime(&now) );
	// Write out the command-line statement
	logfile << "Executing statement : ";
	for(int i=0;i<argc;i++) logfile << argv[i] << " ";
	logfile << std::endl;
	logfile << this->itsCube.pars();
	logfile.close();
      } 
	

    }

    //**************************************************************//

    void DuchampParallel::splitImage(const LOFAR::ACC::APS::ParameterSet& parset)
    {
      /// @details Generates a subsection string for the current
      /// worker based on the number of nodes and the requested
      /// distribution of subimages. This is designed for enabling
      /// access to a subsection of an existing image. This differs
      /// from makeSubImages() in that it does not create new images,
      /// but rather saves the subsection in the itsCube parameter
      /// file so that the duchamp::Cube::getCube() function just
      /// reads the requested subsection.
      ///
      /// @todo Enable the overlap to be based on the beam size?

      if(isParallel() && isWorker()){

	std::vector<duchamp::Section> sectionlist = getSectionList(itsNNode-1, parset);

	if(sectionlist.size()>0){

	  this->itsCube.pars().setFlagSubsection(true);
	  this->itsCube.pars().setSubsection(sectionlist[itsRank-1].getSection());
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<itsRank<<" is using subsection " 
			    << sectionlist[itsRank-1].getSection());

	}

      }
      
    }

    //**************************************************************//

    void DuchampParallel::readData()
    {
      /// @details Reads in the data using duchamp functionality and
      /// the image name defined in the constructor.

      if(isWorker()) {
	
	// Get the OK from the Master so that we know that the subimages have been created.
	bool OK=true; 
	if(isParallel()){
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  itsConnectionSet->read(0, bs);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("goInput");
	  ASKAPASSERT(version==1);
	  in >> OK;
	  in.getEnd();
	}
	if(OK){
	  ASKAPLOG_INFO_STR(logger,  "#"<<itsRank<<": About to read data from image " << this->itsCube.pars().getFullImageFile());

	  int result;
	  if(this->itsIsFITSFile) result = this->itsCube.getCube();
	  else result = casaImageToCube(this->itsCube);

// 	  if(this->itsCube.getCube()==duchamp::FAILURE){
	  if(result==duchamp::FAILURE){
	    ASKAPLOG_ERROR_STR(logger, "#"<<itsRank<<": Could not read in data from image " << this->itsImage);
	    ASKAPTHROW(AskapError, "Unable to read image " << this->itsImage)
	  }
	  else {
	    ASKAPLOG_INFO_STR(logger,  "#"<<itsRank<<": Read data from image " << this->itsImage);
	    std::stringstream ss;
	    ss << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ();
	    ASKAPLOG_INFO_STR(logger, "#"<<itsRank<<": Dimensions are " << ss.str() );
	    if(this->itsCube.getDimZ()==1) this->itsCube.pars().setMinChannels(0);  
	  }

	  if(this->itsCube.pars().getFlagATrous()){
	    ASKAPLOG_INFO_STR(logger,  "Reconstructing");
	    this->itsCube.ReconCube();
	  }
	  else if(this->itsCube.pars().getFlagSmooth()) {
	    ASKAPLOG_INFO_STR(logger,  "Smoothing");	  
	    this->itsCube.SmoothCube();
	  }

	}
	else{
	  ASKAPLOG_ERROR_STR(logger, "#"<<itsRank<<": Could not read data from image " << this->itsImage << " as it's not ready.");
	  ASKAPTHROW(AskapError, "Unable to read image " << this->itsImage)
	}
      }
      else {

	int result;
	if(this->itsIsFITSFile) result = this->itsCube.getMetadata();
	else result = casaImageToMetadata(this->itsCube);

// 	if(this->itsCube.getMetadata()==duchamp::FAILURE){
	if(result==duchamp::FAILURE){
	  ASKAPLOG_ERROR_STR(logger, "MASTER: Could not read in metadata from image " << this->itsImage << ".");
	  ASKAPTHROW(AskapError, "Unable to read image " << this->itsImage)
	}
	else {
	  ASKAPLOG_INFO_STR(logger,  "MASTER: Read metadata from image " << this->itsImage);
	}
// 	this->itsCube.header().defineWCS(this->itsCube.pars().getImageFile(), this->itsCube.pars());
// 	this->itsCube.header().readHeaderInfo(this->itsCube.pars().getImageFile(), this->itsCube.pars());
	if(this->itsCube.getDimZ()==1) this->itsCube.pars().setMinChannels(0);  
	this->itsCube.convertFluxUnits();

	// Send out the OK to the workers, so that they know that the subimages have been created.
	LOFAR::BlobString bs;
	bs.resize(0);
	LOFAR::BlobOBufString bob(bs);
	LOFAR::BlobOStream out(bob);
	out.putStart("goInput",1);
	out << true;
	out.putEnd();
	itsConnectionSet->writeAll(bs);

      }

    }
      
    //**************************************************************//

    void DuchampParallel::findSources()
    {
      /// @details Searches the image/cube for objects, using the
      /// appropriate search function given the user
      /// parameters. Merging of neighbouring objects is then done,
      /// and all WCS parameters are calculated.
      ///
      /// This is only done on the workers.

      if(isWorker()) {
        ASKAPLOG_INFO_STR(logger,  "Finding lists from image " << this->itsImage);

	// remove mininum size criteria, so we don't miss anything on the borders.
	int minpix = this->itsCube.pars().getMinPix();
	int minchan = this->itsCube.pars().getMinChannels();

	if(isParallel()){
	  this->itsCube.pars().setMinPix(1);
	  this->itsCube.pars().setMinChannels(1);
	}

	if(this->itsCube.getSize()>0){

	  if(this->itsCube.pars().getFlagATrous()){
	    ASKAPLOG_INFO_STR(logger,  "Searching with reconstruction first");
	    this->itsCube.ReconSearch();
	  }
	  else if(this->itsCube.pars().getFlagSmooth()) {
	    ASKAPLOG_INFO_STR(logger,  "Searching with smoothing first");	  
	    this->itsCube.SmoothSearch();
	  }
	  else {
	    ASKAPLOG_INFO_STR(logger,  "Searching, no smoothing or reconstruction done.");
	    this->itsCube.CubicSearch();
	  }
	}

	// merge the objects, and grow them if necessary.
	this->itsCube.ObjectMerger();

	this->itsCube.calcObjectWCSparams();

	int16 num = this->itsCube.getNumObj();
        ASKAPLOG_INFO_STR(logger,  "Found " << num << " objects in worker " << this->itsRank);

	if(isParallel()){
	  this->itsCube.pars().setMinPix(minpix);
	  this->itsCube.pars().setMinChannels(minchan);
	}
      }
    }

    //**************************************************************//

    void DuchampParallel::fitSources()
    {

      /// @details The list of RadioSource objects is populated: one
      /// for each of the detected objects. If the 2D profile fitting
      /// is requested, all sources that are not on the image boundary
      /// are fitted by the RadioSource::fitGauss(float *, long *)
      /// function. The fitting for those on the boundary is left for
      /// the master to do after they have been combined with objects
      /// from other subimages.
      ///
      /// @todo Make the boundary determination smart enough to know
      /// which side is adjacent to another subimage.

      if(isWorker()){

	// don't do fit if we have a spectral axis.
	bool flagIs2D = !this->itsCube.header().canUseThirdAxis() || this->is2D();

	this->itsFlagDoFit = this->itsFlagDoFit && flagIs2D;
	
	ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<": Fitting source profiles.");
	duchamp::FitsHeader head = this->itsCube.getHead();
// 	float noise;
// 	if(this->itsCube.pars().getFlagUserThreshold()) noise = 1.;
// 	else noise = this->itsCube.stats().getSpread();
// 	ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<": Setting noise level to " << noise);
	float threshold = this->itsCube.stats().getThreshold();
	if(this->itsCube.pars().getFlagGrowth()){
	  if(this->itsCube.pars().getFlagUserGrowthThreshold())
	    threshold = this->itsCube.pars().getGrowthThreshold();
	  else
	    threshold = this->itsCube.stats().snrToValue(this->itsCube.pars().getGrowthCut());
	}

	int numObj = this->itsCube.getNumObj();
 	for(int i=0;i<numObj;i++){
	  sourcefitting::RadioSource src(this->itsCube.getObject(i));
	  src.setNoiseLevel( this->itsCube, this->itsFitter );
	  src.setDetectionThreshold( threshold );
	  src.setHeader( head );
	  src.defineBox(this->itsCube.getDimArray(), this->itsFitter);

	  // Only do fit if object is not next to boundary
	  src.setAtEdge(this->itsCube);
	  if(this->itsNNode==1) src.setAtEdge(false);
	  if(!src.isAtEdge() && this->itsFlagDoFit){
	    ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<": Fitting source #"<<i+1<<" / "<<numObj<<".");
// 	    src.fitGauss(this->itsCube.getArray(),this->itsCube.getDimArray());
	    src.fitGauss(this->itsCube.getArray(),this->itsCube.getDimArray(), this->itsFitter);
	  }
	  this->itsSourceList.push_back(src);
	}
	
      }

    }

    //**************************************************************//

    void DuchampParallel::sendObjects() 
    {

      /// @details The RadioSource objects on each worker, which
      /// contain each detected object, are sent to the Master node
      /// via LOFAR Blobs.
      /// @todo Voxellist is really only needed for the boundary sources.
      ///
      /// In the non-parallel case, we put together a voxel list. Not
      /// sure whether this is necessary at this point.
      /// @todo Sort out voxelList necessity.

      if(isWorker()){

	int16 num = this->itsCube.getNumObj(), rank=this->itsRank;

	if(isParallel()){

	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("detW2M",1);
	  out << rank << num;
	  std::vector<sourcefitting::RadioSource>::iterator src = this->itsSourceList.begin();
	  for(;src<this->itsSourceList.end();src++){

	    // for each RadioSource object, send to master
	    out << *src;
   
	    if( src->isAtEdge() ){

	      int xmin,xmax,ymin,ymax,zmin,zmax;
	      xmin = std::max(0 , int(src->boxXmin()));
	      xmax = std::min(this->itsCube.getDimX()-1, src->boxXmax());
	      ymin = std::max(0 , int(src->boxYmin()));
	      ymax = std::min(this->itsCube.getDimY()-1, src->boxYmax());
	      zmin = std::max(0 , int(src->boxZmin() ));
	      zmax = std::min(this->itsCube.getDimZ()-1, src->boxZmax());
	  
 	      int numVox = (xmax-xmin+1)*(ymax-ymin+1)*(zmax-zmin+1);
	      out << numVox;
	  
	      for(int32 x=xmin; x<=xmax; x++){
		for(int32 y=ymin; y<=ymax; y++){
		  for(int32 z=zmin; z<=zmax; z++){
		
		    bool inObject = src->pixels().isInObject(x,y,z);
		    float flux = this->itsCube.getPixValue(x,y,z);
		
		    out << inObject << x << y << z << flux;
		
		  }
		}
	      }
	    }

	  }
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  ASKAPLOG_INFO_STR(logger, "Sent detection list to the master from worker " << this->itsRank );
	}
        else{ 

	}     

      }    
    }

    //**************************************************************//

    void DuchampParallel::receiveObjects()
    {
      /// @details On the Master node, receive the list of RadioSource
      /// objects sent by the workers. Also receives the list of
      /// detected and surrounding voxels - these will be used to
      /// calculate parameters of any merged boundary sources.
      /// @todo Voxellist is really only needed for the boundary sources.
      
      if(isMaster()){
 
	ASKAPLOG_INFO_STR(logger,  "MASTER: Retrieving lists from workers" );

	LOFAR::BlobString bs;
	int16 rank, numObj;
	for (int i=1; i<itsNNode; i++)
	  {
	    itsConnectionSet->read(i-1, bs);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    int version=in.getStart("detW2M");
	    ASKAPASSERT(version==1);
	    in >> rank >> numObj;
	    ASKAPLOG_INFO_STR(logger, "MASTER: Starting to read " 
			      << numObj << " objects from worker #"<< rank-1);
	    for(int obj=0;obj<numObj;obj++){
	      sourcefitting::RadioSource src;
	      in >> src;
	      ASKAPLOG_INFO_STR(logger, "MASTER: Read Source " << obj+1 << " from worker#"<<rank-1);
	      // Correct for any offsets;
	      src.setXOffset(this->itsSectionList[i-1].getStart(0));
	      src.setYOffset(this->itsSectionList[i-1].getStart(1));
	      src.setZOffset(this->itsSectionList[i-1].getStart(2));
	      src.addOffsets();
	      src.defineBox(this->itsCube.getDimArray(), this->itsFitter);
	      for(unsigned int f=0;f<src.fitset().size();f++){
		src.fitset()[f].setXcenter(src.fitset()[f].xCenter() + src.getXOffset());
		src.fitset()[f].setYcenter(src.fitset()[f].yCenter() + src.getYOffset());
	      }
	      // And now set offsets to zero as we are in the master cube
	      src.setXOffset(0); src.setYOffset(0); src.setZOffset(0);

	      this->itsSourceList.push_back(src);
	      
	      if( src.isAtEdge() ){
	      
		int numVox;
		in >> numVox;
		for(int p=0;p<numVox;p++){
		  int32 x,y,z; 
		  float flux;
		  bool inObj;
		  in >> inObj >> x >> y >> z >> flux; 
		  x += this->itsSectionList[i-1].getStart(0);
		  y += this->itsSectionList[i-1].getStart(1);
		  z += this->itsSectionList[i-1].getStart(2);
		  PixelInfo::Voxel vox(x,y,z,flux);
		  this->itsVoxelList.push_back(vox);
		}

	      }

	    }
	    ASKAPLOG_INFO_STR(logger, "MASTER: Received list of size " 
			      << numObj << " from worker #"<< rank);
	    ASKAPLOG_INFO_STR(logger, "MASTER: Now have " 
			      << this->itsSourceList.size() << " objects");
	    in.getEnd();

	  }
	      
      }

    }

    //**************************************************************//

    void DuchampParallel::cleanup()
    {
      /// @details Done on the Master node. This function gathers the
      /// sources that are marked as on the boundary of subimages, and
      /// combines them via the duchamp::Cubes::ObjectMerger()
      /// function. The resulting sources are then fitted (if so
      /// required) and have their WCS parameters calculated by the
      /// calcObjectParams() function.
      ///
      /// Once this is done, these sources are added to the cube
      /// detection list, along with the non-boundary objects. The
      /// final list of RadioSource objects is then sorted (by the
      /// Name field) and given object IDs.


      if(isMaster()){
	
	ASKAPLOG_INFO_STR(logger, "MASTER: Beginning the cleanup");

	std::vector<sourcefitting::RadioSource> backuplist = this->itsSourceList;
	std::vector<sourcefitting::RadioSource> edgeSources, goodSources;
	std::vector<sourcefitting::RadioSource>::iterator src;
	for(src=this->itsSourceList.begin();src<this->itsSourceList.end();src++){
	  if(src->isAtEdge()) edgeSources.push_back(*src);
	  else goodSources.push_back(*src);
	}

	//	ASKAPLOG_INFO_STR(logger, "MASTER: edgeSources.size="<<edgeSources.size()<<
	//		  " goodSources.size="<<goodSources.size());
	this->itsSourceList.clear();

	duchamp::FitsHeader head = this->itsCube.getHead();
	float noise;
	if(this->itsCube.pars().getFlagUserThreshold()) noise = 1.;
	else noise = this->itsCube.stats().getStddev();
	float threshold;
	if(this->itsCube.pars().getFlagUserThreshold()) threshold = this->itsCube.pars().getThreshold();
	else threshold = this->itsCube.stats().getThreshold();

	if(edgeSources.size()>0){ // if there are edge sources

	  for(src=edgeSources.begin();src<edgeSources.end();src++) this->itsCube.addObject(*src);
	  
	  this->itsCube.pars().setFlagGrowth(false);
	  this->itsCube.ObjectMerger();
	  this->calcObjectParams();
	  ASKAPLOG_INFO_STR(logger, "MASTER: num sources in cube = "<<this->itsCube.getNumObj());

	  for(int i=0;i<this->itsCube.getNumObj();i++){
	    ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source #"<<i+1<<".");
	    sourcefitting::RadioSource src(this->itsCube.getObject(i));
	    src.setNoiseLevel( noise );
	    src.setDetectionThreshold( threshold );
	    src.setHeader( head );
 	    src.defineBox(this->itsCube.getDimArray(), this->itsFitter);

	    if(this->itsFlagDoFit) src.fitGauss(&this->itsVoxelList);
	    
	    this->itsSourceList.push_back(src);
	  }
	  
	}

	for(src=goodSources.begin();src<goodSources.end();src++){
	  src->setHeader(head);
	  // Need to check that there are no small sources present that violate the minimum size criteria
	  if( (src->hasEnoughChannels(this->itsCube.pars().getMinChannels()))
	  && (src->getSpatialSize() >= this->itsCube.pars().getMinPix()) )
	    this->itsSourceList.push_back(*src);
	}

	std::stable_sort(this->itsSourceList.begin(), this->itsSourceList.end());

	this->itsCube.clearDetectionList();
	for(src=this->itsSourceList.begin();src<this->itsSourceList.end();src++){
	  src->setID(src-this->itsSourceList.begin()+1);
	  this->itsCube.addObject(duchamp::Detection(*src));
	}

      }
      

    }

    //**************************************************************//

    void DuchampParallel::calcObjectParams()
    {

      /// @details A function to calculate object parameters
      /// (including WCS parameters), making use of the this->itsVoxelList
      /// set of voxels. The function finds the voxels that appear in
      /// each object in itsCube, making a vector of vectors of
      /// voxels, then passes this vector to
      /// duchamp::Cube::calcObjectWCSparams().

      int numVox = this->itsVoxelList.size();
      int numObj = this->itsCube.getNumObj();
      if(numObj>0){
	std::vector<PixelInfo::Voxel> templist[numObj];
	for(int i=0;i<this->itsCube.getNumObj();i++){ 
	  // for each object, make a vector list of voxels that appear in it.
	  
	  std::vector<PixelInfo::Voxel> 
	    objVoxList=this->itsCube.getObject(i).getPixelSet();
	  std::vector<PixelInfo::Voxel>::iterator vox;
	  // get the fluxes of each voxel
	  for(vox=objVoxList.begin();vox<objVoxList.end();vox++){
	    int ct=0;
	    while(ct<numVox && !vox->match(this->itsVoxelList[ct])){
	      ct++;
	    }
	    if(numVox!=0 && ct==numVox){ // there has been no match -- problem!
	      ASKAPLOG_ERROR(logger, "MASTER: Found a voxel in the object lists that doesn't appear in the base list.");
	    }
	    else vox->setF( this->itsVoxelList[ct].getF() );
	  }
	
	  templist[i] = objVoxList;
	
	}
	std::vector< std::vector<PixelInfo::Voxel> > 
	  bigVoxSet (templist, templist + numObj);
	this->itsCube.calcObjectWCSparams(bigVoxSet);
      }      
    }

    //**************************************************************//

    void DuchampParallel::printResults()
    {
      /// @details The final list of detected objects is written to
      /// the terminal and to the results file in the standard Duchamp
      /// manner.

      if(isMaster()) {
	ASKAPLOG_INFO_STR(logger, "MASTER: Found " << this->itsCube.getNumObj() << " sources.");
	
	this->itsCube.prepareOutputFile();
// 	if(this->itsCube.getNumObj()>0){
// 	  // no flag-setting, as it's hard to do when we don't have
// 	  // all the pixels. Particularly the negative flux flags
// 	  this->itsCube.sortDetections();
// 	}
	this->itsCube.outputDetectionList();

	if(this->itsCube.pars().getFlagKarma()){
	  std::ofstream karmafile(this->itsCube.pars().getKarmaFile().c_str());
	  this->itsCube.outputDetectionsKarma(karmafile);
	  karmafile.close();
	}

	//	if(this->itsFlagDoFit){
	  std::ofstream summaryFile(this->itsSummaryFile.c_str());
	  std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();
	  std::vector<sourcefitting::RadioSource>::iterator src=this->itsSourceList.begin();
	  for(;src<this->itsSourceList.end();src++)
	    src->printSummary(summaryFile, columns, src==this->itsSourceList.begin());
	  summaryFile.close();

	  if(this->itsFlagDoFit) this->writeFitAnnotation();
	  //	}

      }
      else{
      }
    }

    //**************************************************************//

 
    void DuchampParallel::writeFitAnnotation()
    {
      /// @details This function writes a Karma annotation file
      /// showing the location and shape of the fitted 2D Gaussian
      /// components. It makes use of the
      /// RadioSource::writeFitToAnnotationFile() function. The file
      /// written to is given by the input parameter
      /// fitAnnotationFile.

      if(this->itsSourceList.size()>0){
	
	std::ofstream outfile(this->itsFitAnnotationFile.c_str());
	ASKAPLOG_INFO_STR(logger, "Writing to annotation file: " << this->itsFitAnnotationFile );
	outfile << "COLOR BLUE\n";
	outfile << "COORD W\n";
	outfile << "PA SKY\n";
	std::vector<sourcefitting::RadioSource>::iterator src;
	for(src=this->itsSourceList.begin();src<this->itsSourceList.end();src++){
	  outfile << "# Source " << int(src-this->itsSourceList.begin())+1 << ":\n";
	  src->writeFitToAnnotationFile(outfile);
	}
	
	outfile.close();
	
      }

    }

    //**************************************************************//
    
    void DuchampParallel::gatherStats()
    {
      /// @details A front-end function that calls all the statistics
      /// functions. Net effect is to find the mean/median and
      /// rms/MADFM for the entire dataset and store these values in
      /// the master's itsCube statsContainer.

      if(!this->itsCube.pars().getFlagUserThreshold() || this->itsCube.pars().getFlagGrowth()){
	findMeans();
	combineMeans();
	broadcastMean();
	findRMSs();
	combineRMSs();
      }
      //	if(this->itsCube.pars().getFlagUserThreshold())//{
      else 
	this->itsCube.stats().setThreshold( this->itsCube.pars().getThreshold() );
    }


    //**************************************************************//

    void DuchampParallel::findMeans()
    {
      /// @details In the parallel case, this finds the mean or median
      /// (according to the flagRobustStats parameter) of the worker's
      /// image/cube, then sends that value to the master via LOFAR
      /// Blobs.
      ///
      /// In the serial (non-parallel) case, all stats for the cube
      /// are calculated in the standard manner via the
      /// duchamp::Cube::setCubeStats() function.

      if(isWorker()) {
	
	if(isParallel()){

	  ASKAPLOG_INFO_STR(logger, "Finding mean: worker " << this->itsRank);

	  if(this->itsCube.pars().getFlagATrous()) this->itsCube.ReconCube();
	  else if(this->itsCube.pars().getFlagSmooth()) this->itsCube.SmoothCube();

	  int32 size = this->itsCube.getSize();
	  float mean = 0.,rms;
	  float *array;
	  // make a mask in case there are blank pixels.
	  bool *mask = this->itsCube.pars().makeBlankMask(this->itsCube.getArray(), this->itsCube.getSize());

	  if(size>0){
	  
	    if(this->itsCube.pars().getFlagATrous())       array = this->itsCube.getArray();
	    else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
	    else                                     array = this->itsCube.getArray();
	  
	    // calculate both mean & rms, but ignore rms for the moment.
	    if(this->itsCube.pars().getFlagRobustStats()) findMedianStats(array,size,mask,mean,rms);
	    else                                    findNormalStats(array,size,mask,mean,rms);

	  }
	  double dmean = mean;

	  ASKAPLOG_INFO_STR(logger, "#" << this->itsRank << ": Mean = " << mean );
	
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("meanW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << dmean << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  //	  itsConnectionSet->write(this->itsRank,bs);
	  ASKAPLOG_INFO_STR(logger, "Sent mean to the master from worker " << this->itsRank );
	}
	else {
	  // serial case -- can just calculate all stats at once.
	  ASKAPLOG_INFO_STR(logger, "Calculating stats: worker " << this->itsRank);
	  this->itsCube.setCubeStats();
	  ASKAPLOG_INFO_STR(logger, "Stats are as follows:");
	  std::cout << this->itsCube.stats();
	}

      }
      else {
      }
    }

    //**************************************************************//

    void DuchampParallel::findRMSs() 
    {
      /// @details In the parallel case, this finds the rms or the
      /// median absolute deviation from the median (MADFM) (dictated
      /// by the flagRobustStats parameter) of the worker's
      /// image/cube, then sends that value to the master via LOFAR
      /// Blobs. To calculate the rms/MADFM, the mean of the full
      /// dataset must be read from the master (again passed via LOFAR
      /// Blobs). The calculation uses the findSpread() function.
      ///
      /// In the serial case, nothing is done, as we have already
      /// calculated the rms in the findMeans() function.

      if(isWorker() && isParallel()) {
	ASKAPLOG_INFO_STR(logger, "About to calculate rms on worker " << this->itsRank);

	// first read in the overall mean for the cube
	double mean=0;
	if(isParallel()) {
	  LOFAR::BlobString bs1;
	  itsConnectionSet->read(0, bs1);
	  LOFAR::BlobIBufString bib(bs1);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("meanM2W");
	  ASKAPASSERT(version==1);
	  in >> mean;
	  in.getEnd();
	}
	else{
	  mean = this->itsCube.stats().getMiddle();
	}
	// use it to calculate the rms for this section
 	int32 size = this->itsCube.getSize();
	double rms = 0.;
	float *array;

	if(size > 0){
	  
	  if(this->itsCube.pars().getFlagATrous()){
	    array = new float[size];
	    for(int i=0;i<size;i++) array[i] = this->itsCube.getPixValue(i) - this->itsCube.getReconValue(i);
	  }
	  else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
	  else array = this->itsCube.getArray();
	  if(this->itsCube.pars().getFlagBlankPix()){
	    bool *mask = this->itsCube.pars().makeBlankMask(array, this->itsCube.getSize());
	    rms = findSpread(this->itsCube.pars().getFlagRobustStats(),mean,size,array,mask);
	  }
	  else rms = findSpread(this->itsCube.pars().getFlagRobustStats(),mean,size,array);

	}

	ASKAPLOG_INFO_STR(logger, "#" << this->itsRank << ": rms = " << rms );

	// return it to the master
	  LOFAR::BlobString bs2;
	  bs2.resize(0);
	  LOFAR::BlobOBufString bob(bs2);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("rmsW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << rms << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs2);
	  ASKAPLOG_INFO_STR(logger, "Sent local rms to the master from worker " << this->itsRank );

      }
      else {
      }

    }

    //**************************************************************//

    void DuchampParallel::combineMeans()
    {
      /// @details The master reads the mean/median values from each
      /// of the workers, and combines them to form the mean/median of
      /// the full dataset. Note that if the median of the workers
      /// data has been provided, the values are treated as estimates
      /// of the mean, and are combined as if they were means (ie. the
      /// overall value is the weighted (by size) average of the
      /// means/medians of the individual images). The value is stored
      /// in the StatsContainer in itsCube.

      if(isMaster()&&isParallel()) {
	// get the means from the workers
        ASKAPLOG_INFO_STR(logger,  "MASTER: Receiving Means and combining" );

	LOFAR::BlobString bs1;
        int size=0;
	double av=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs1);
          LOFAR::BlobIBufString bib(bs1);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("meanW2M");
          ASKAPASSERT(version==1);
	  double newav;
	  int32 newsize;
	  int16 rank;
          in >> rank >> newav >> newsize;
          in.getEnd();
          ASKAPLOG_INFO_STR(logger, "MASTER: Received mean from worker "<< rank);

	  size += newsize;
	  av += newav * newsize;
        }

	if(size>0){
	  av /= double(size);
	}

	ASKAPLOG_INFO_STR(logger, "MASTER: OVERALL SIZE = " << size);
	ASKAPLOG_INFO_STR(logger, "MASTER: OVERALL MEAN = " << av);
	this->itsCube.stats().setMean(av);
      }
      else {
      }
    }
	
    //**************************************************************//

    void DuchampParallel::broadcastMean() 
    {
      /// @details The mean/median value of the full dataset is sent
      /// via LOFAR Blobs to the workers.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	double av = this->itsCube.stats().getMean();
	LOFAR::BlobString bs2;
	bs2.resize(0);
	LOFAR::BlobOBufString bob(bs2);
	LOFAR::BlobOStream out(bob);
	out.putStart("meanM2W",1);
	out << av;
	out.putEnd();
	itsConnectionSet->writeAll(bs2);
	ASKAPLOG_INFO_STR(logger, "MASTER: Broadcast overal mean from master to workers." );
      }
      else {
      }
    }

    //**************************************************************//

    void DuchampParallel::combineRMSs()
    {
      /// @details The master reads the rms/MADFM values from each of
      /// the workers, and combines them to produce an estimate of the
      /// rms for the full cube. Again, if MADFM values have been
      /// calculated on the workers, they are treated as estimates of
      /// the rms and are combined as if they are rms values. The
      /// overall value is stored in the StatsContainer in itsCube.

      if(isMaster()&&isParallel()) {
	// get the means from the workers
        ASKAPLOG_INFO_STR(logger,  "MASTER: Receiving RMS values and combining" );

	LOFAR::BlobString bs;
        int size=0;
	double rms=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("rmsW2M");
          ASKAPASSERT(version==1);
	  double newrms;
	  int32 newsize;
	  int16 rank;
          in >> rank >> newrms >> newsize;
          in.getEnd();
          ASKAPLOG_INFO_STR(logger, "MASTER: Received RMS from worker "<< rank);

	  size += newsize;
	  rms += (newrms * newrms * (newsize-1));
        }

	if(size>0){
	  rms = sqrt( rms / double(size-1) );
	}
	this->itsCube.stats().setStddev(rms);

	this->itsCube.stats().setRobust(false);
	if(!this->itsCube.pars().getFlagUserThreshold()){
	  this->itsCube.stats().setThresholdSNR(this->itsCube.pars().getCut());	
	  this->itsCube.pars().setFlagUserThreshold(true);
	  this->itsCube.pars().setThreshold(this->itsCube.stats().getThreshold());
	}

	ASKAPLOG_INFO_STR(logger, "MASTER: OVERALL RMS = " << rms);

      }
    }

    //**************************************************************//

    void DuchampParallel::broadcastThreshold() 
    {
      /// @details The detection threshold value (which has been
      /// already calculated) is sent to the workers via LOFAR Blobs.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	LOFAR::BlobString bs;
	bs.resize(0);
	LOFAR::BlobOBufString bob(bs);
	LOFAR::BlobOStream out(bob);
	out.putStart("threshM2W",1);
	double threshold = this->itsCube.stats().getThreshold();
	double mean = this->itsCube.stats().getMiddle();
	double rms = this->itsCube.stats().getSpread();
	out << threshold << mean << rms;
	out.putEnd();
	itsConnectionSet->writeAll(bs);
	ASKAPLOG_INFO_STR(logger, "MASTER: Sent threshold (" 
			   << this->itsCube.stats().getThreshold() << ") from the master" );
      }
      else {
      }
    }


    //**************************************************************//

    void DuchampParallel::receiveThreshold()
    {
      /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
      
      if(isWorker()) {

	double threshold, mean, rms;
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  itsConnectionSet->read(0, bs);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("threshM2W");
	  ASKAPASSERT(version==1);
	  in >> threshold >> mean >> rms;
	  in.getEnd();

	  this->itsCube.stats().setRobust(false);
	  this->itsCube.stats().setMean(mean);
	  this->itsCube.stats().setStddev(rms);
	  this->itsCube.pars().setFlagUserThreshold(true);
	  ASKAPLOG_INFO_STR(logger, "#"<<itsRank<<": Setting mean to be " << mean << " and rms " << rms);
	}
	else{
	  if(this->itsCube.pars().getFlagUserThreshold())
	    threshold = this->itsCube.pars().getThreshold();
	  else
	    threshold = this->itsCube.stats().getMiddle() + this->itsCube.stats().getSpread()*this->itsCube.pars().getCut();
	}

	ASKAPLOG_INFO_STR(logger, "Setting threshold on worker  " << itsRank << " to be " << threshold);

	this->itsCube.pars().setThreshold(threshold);

      }
    }


  }
}
