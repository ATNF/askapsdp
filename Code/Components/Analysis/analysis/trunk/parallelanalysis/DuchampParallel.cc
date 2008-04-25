/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
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
#include <sourcefitting/RadioSource.h>

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

    DuchampParallel::DuchampParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : AskapParallel(argc, argv)
    {
      /// @details The constructor reads parameters from the parameter
      /// set parset. This set can include Duchamp parameters, as well
      /// as particular cduchamp parameters such as masterImage and
      /// sectionInfo.

      // First do the setup needed for both workers and master

      itsCube.pars() = parseParset(parset);

      this->itsFlagDoFit = parset.getBool("doFit", false);
      this->itsSummaryFile = parset.getString("summaryFile", "duchamp-Summary.txt");
      this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-Results-Fits.ann");

      // Now read the correct image name according to worker/master state.
      if(isMaster()){
	itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );
	itsCube.pars().setFlagRobustStats(false);
	if(isParallel()) this->itsSectionList = makeSubImages(this->itsNNode-1,parset);
	itsImage = substitute(parset.getString("image"));
	itsCube.pars().setImageFile(itsImage);

      }

      if(isWorker()) {
 	 itsImage = substitute(parset.getString("image"));
	if(isParallel()) itsImage = getSubImageName(itsImage,itsRank-1,itsNNode-1);
	
	itsCube.pars().setImageFile(itsImage);
	//	splitImage(parset);

	// get the list of subsections so that all nodes can access it.
	if(isParallel()){
	  std::vector<duchamp::Section> sectionList = getSectionList(this->itsNNode-1,parset);
	  itsCube.pars().setSubsection(sectionList[itsRank-1].getSection());
	}

	itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );	
	itsCube.pars().setFlagRobustStats( parset.getBool("flagRobust",true) );
      }

      if(itsCube.pars().getFlagLog()){
	ASKAPLOG_INFO_STR(logger, "Setting up logfile " << itsCube.pars().getLogFile() );
	std::ofstream logfile(itsCube.pars().getLogFile().c_str());
	logfile << "New run of the CDuchamp sourcefinder: ";
	time_t now = time(NULL);
	logfile << asctime( localtime(&now) );
	// Write out the command-line statement
	logfile << "Executing statement : ";
	for(int i=0;i<argc;i++) logfile << argv[i] << " ";
	logfile << std::endl;
	logfile << itsCube.pars();
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

	  itsCube.pars().setFlagSubsection(true);
	  itsCube.pars().setSubsection(sectionlist[itsRank-1].getSection());
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<itsRank<<" is using subsection " 
			    << sectionlist[itsRank-1].getSection());

	}

      }
      

    }

    //**************************************************************//

    // Read in the data from the image file (on the workers)
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
	  ASKAPLOG_INFO_STR(logger,  "#"<<itsRank<<": About to read data from image " << itsCube.pars().getFullImageFile());
	  if(itsCube.getCube()==duchamp::FAILURE){
	    ASKAPLOG_ERROR_STR(logger, "#"<<itsRank<<": Could not read in data from image " << itsImage);
	  }
	  else {
	    ASKAPLOG_INFO_STR(logger,  "#"<<itsRank<<": Read data from image " << itsImage);
	    std::stringstream ss;
	    ss << itsCube.getDimX() << " " << itsCube.getDimY() << " " << itsCube.getDimZ();
	    ASKAPLOG_INFO_STR(logger, "#"<<itsRank<<": Dimensions are " << ss.str() );
	    if(itsCube.getDimZ()==1) itsCube.pars().setMinChannels(0);  
	  }
	}
	else{
	  ASKAPLOG_ERROR_STR(logger, "#"<<itsRank<<": Could not read data from image " << itsImage << " as it's not ready.");
	}
      }
      else {

	if(itsCube.getMetadata()==duchamp::FAILURE){
	  ASKAPLOG_ERROR_STR(logger, "MASTER: Could not read in metadata from image " << itsImage << ".");
	}
	else {
	  ASKAPLOG_INFO_STR(logger,  "MASTER: Read metadata from image " << itsImage);
	}
	itsCube.header().defineWCS(itsCube.pars().getImageFile(), itsCube.pars());
	itsCube.header().readHeaderInfo(itsCube.pars().getImageFile(), itsCube.pars());
	if(itsCube.getDimZ()==1) itsCube.pars().setMinChannels(0);  
	
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
      ///@details Searches the image/cube for objects, using the
      ///duchamp::Cube::CubicSearch function. The detected objects are
      ///then sent to the master using LOFAR Blobs. This is done
      ///pixel-by-pixel, sending the pixel flux as well.
      ///
      /// This is only done on the workers.

      if(isWorker()) {
        ASKAPLOG_INFO_STR(logger,  "Finding lists from image " << itsImage);

	if(itsCube.getSize()>0){

	  itsCube.setCubeStats();
	  if(itsCube.pars().getFlagATrous()){
	    ASKAPLOG_INFO_STR(logger,  "Searching with reconstruction first");
	    itsCube.ReconSearch();
	  }
	  else if(itsCube.pars().getFlagSmooth()) {
	    ASKAPLOG_INFO_STR(logger,  "Searching with smoothing first");	  
	    itsCube.SmoothSearch();
	  }
	  else {
	    ASKAPLOG_INFO_STR(logger,  "Searching, no smoothing or reconstruction done.");
	    itsCube.CubicSearch();
	  }
	}

	// merge the objects, and grow them if necessary.
	itsCube.ObjectMerger();

	itsCube.calcObjectWCSparams();

	int16 num = itsCube.getNumObj(), rank=this->itsRank;
        ASKAPLOG_INFO_STR(logger,  "Found " << num << " objects in worker " << this->itsRank);
	
      }
    }

    //**************************************************************//

    void DuchampParallel::fitSources()
    {

      if(isWorker()){
	
	ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<": Fitting source profiles.");
	duchamp::FitsHeader head = itsCube.getHead();
	float noise;
	if(itsCube.pars().getFlagUserThreshold()) noise = 1.;
	else noise = itsCube.stats().getStddev();
	float threshold;
	if(itsCube.pars().getFlagUserThreshold()) threshold = itsCube.pars().getThreshold();
	else threshold = itsCube.stats().getThreshold();

 	for(int i=0;i<itsCube.getNumObj();i++){
	  ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<": Fitting source #"<<i+1<<".");
	  sourcefitting::RadioSource src(itsCube.getObject(i));
// 	  src.setNoiseLevel( noise );
	  src.setDetectionThreshold( threshold );
	  src.setHeader( head );

	  // Only do fit if object is not next to boundary
	  bool flagBoundary = false;
	  bool flagAdj = itsCube.pars().getFlagAdjacent();
	  int threshS = itsCube.pars().getThreshS();
	  int threshV = itsCube.pars().getThreshV();
	  if(flagAdj){
	    flagBoundary = flagBoundary || ( src.getXmin()==0 );
	    flagBoundary = flagBoundary || ( src.getXmax()==itsCube.getDimX()-1 ); 
	    flagBoundary = flagBoundary || ( src.getYmin()==0 );
	    flagBoundary = flagBoundary || ( src.getYmax()==itsCube.getDimY()-1 ); 
	    if(itsCube.getDimZ()>1){
	      flagBoundary = flagBoundary || ( src.getZmin()==0 );
	      flagBoundary = flagBoundary || ( src.getZmax()==itsCube.getDimZ()-1 ); 
	    }
	  }
	  else{
	    flagBoundary = flagBoundary || ( src.getXmin()<threshS );
	    flagBoundary = flagBoundary || ( (itsCube.getDimX()-src.getXmax())<threshS ); 
	    flagBoundary = flagBoundary || ( src.getYmin()<threshS );
	    flagBoundary = flagBoundary || ( (itsCube.getDimY()-src.getYmax())<threshS ); 
	    flagBoundary = flagBoundary || ( src.getZmin()<threshV );
	    flagBoundary = flagBoundary || ( (itsCube.getDimZ()-src.getZmax())<threshV ); 
	  }
	  ASKAPLOG_INFO_STR(logger, "#"<<this->itsRank<<"("<<i+1
			    <<") bdry="<<flagBoundary<<", doFit="<<itsFlagDoFit
			    <<", xmin="<<src.getXmin()<<", xmax="<<src.getXmax()
			    <<", ymin="<<src.getYmin()<<", ymax="<<src.getYmax()
			    <<", zmin="<<src.getZmin()<<", zmax="<<src.getZmax()
			    );
	  src.setAtEdge(flagBoundary);
	  if(!flagBoundary && itsFlagDoFit){
	    //	    if( src.setFluxArray(&(this->itsVoxelList)) ){
// 	    if( src.setFluxArray(itsCube.getArray()) ){
// 	      src.fitGauss();
// 	    }
	    src.fitGauss(itsCube.getArray(),itsCube.getDimArray());
	  }
	  itsSourceList.push_back(src);
	}
	
      }

    }

    //**************************************************************//

    void DuchampParallel::sendObjects() 
    {

      if(isWorker()){

	int16 num = itsCube.getNumObj(), rank=this->itsRank;

	if(isParallel()){

	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("detW2M",1);
	  out << rank << num;
	  for(int i=0;i<itsSourceList.size();i++){
	    // for each RadioSource object, send to master
	    out << itsSourceList[i];

	    int border = sourcefitting::detectionBorder;
	    /// @todo -- abstract this getting-the-surrounding-area into a class?
	    int xmin,xmax,ymin,ymax,zmin,zmax;
	    xmin = std::max(0 , int(itsSourceList[i].getXmin()-border));
	    xmax = std::min(itsCube.getDimX()-1, itsSourceList[i].getXmax()+border);
	    ymin = std::max(0 , int(itsSourceList[i].getYmin()-border));
	    ymax = std::min(itsCube.getDimY()-1, itsSourceList[i].getYmax()+border);
	    zmin = std::max(0 , int(itsSourceList[i].getZmin()-border));
	    zmax = std::min(itsCube.getDimZ()-1, itsSourceList[i].getZmax()+border);
	  
	    int numVox = (xmax-xmin+1)*(ymax-ymin+1)*(zmax-zmin+1);
	    out << numVox;
	  
	    for(int32 x=xmin; x<=xmax; x++){
	      for(int32 y=ymin; y<=ymax; y++){
		for(int32 z=zmin; z<=zmax; z++){
		
		  bool inObject = itsSourceList[i].pixels().isInObject(x,y,z);
		  float flux = itsCube.getPixValue(x,y,z);
		
		  out << inObject << x << y << z << flux;
		
		}
	      }
	    }
	  }

	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  ASKAPLOG_INFO_STR(logger, "Sent detection list to the master from worker " << this->itsRank );
	}
        else{ // if not parallel, still want to make the voxelList
	  
	  for(int i=0;i<itsSourceList.size();i++){
	    for(int32 x=this->itsSourceList[i].boxXmin(); x<=this->itsSourceList[i].boxXmax(); x++){
	      for(int32 y=this->itsSourceList[i].boxYmin(); y<=this->itsSourceList[i].boxYmax(); y++){
		for(int32 z=this->itsSourceList[i].boxZmin(); z<=this->itsSourceList[i].boxZmax(); z++){
	                 
		  float flux = 0.;
		  if( (x>=0 && x<itsCube.getDimX()) &&
		      (y>=0 && y<itsCube.getDimY()) &&
		      (z>=0 && z<itsCube.getDimZ()) )  flux = itsCube.getPixValue(x,y,z);
	
		  PixelInfo::Voxel vox(x,y,z,flux);
		  itsVoxelList.push_back(vox);
	                 
		}
	      }
	    }
	  }

	}     

      }    
    }

    //**************************************************************//

    void DuchampParallel::receiveObjects()
    {
      
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
			      << numObj << " objects from worker #"<< rank);
	    for(int obj=0;obj<numObj;obj++){
	      sourcefitting::RadioSource src;
	      in >> src;
	      ASKAPLOG_INFO_STR(logger, "MASTER: Read Source " << obj+1 << " from worker#"<<rank);
	      src.setXOffset(itsSectionList[i-1].getStart(0));
	      src.setYOffset(itsSectionList[i-1].getStart(1));
	      src.setZOffset(itsSectionList[i-1].getStart(2));
	      //	      src.addOffsets();
	      itsSourceList.push_back(src);
	      int numVox;
	      in >> numVox;
	      for(int p=0;p<numVox;p++){
		int32 x,y,z; 
		float flux;
		bool inObj;
		in >> inObj >> x >> y >> z >> flux; 
		x += itsSectionList[i-1].getStart(0);
		y += itsSectionList[i-1].getStart(1);
		z += itsSectionList[i-1].getStart(2);
		//		if(inObj) object->addPixel(x,y,z);
		PixelInfo::Voxel vox(x,y,z,flux);
		itsVoxelList.push_back(vox);
	      }

	    }
	    ASKAPLOG_INFO_STR(logger, "MASTER: Received list of size " 
			      << numObj << " from worker #"<< rank);
	    ASKAPLOG_INFO_STR(logger, "MASTER: Now have " 
			      << itsSourceList.size() << " objects");
	    in.getEnd();

	  }
	      
      }

    }

    //**************************************************************//

    void DuchampParallel::cleanup()
    {

      if(isMaster()){
	
	ASKAPLOG_INFO_STR(logger, "MASTER: Beginning the cleanup");

	std::vector<sourcefitting::RadioSource> backuplist = itsSourceList;
	std::vector<sourcefitting::RadioSource> edgeSources, goodSources;
	for(int i=0;i<itsSourceList.size();i++){
	  if(itsSourceList[i].isAtEdge()) edgeSources.push_back(itsSourceList[i]);
	  else goodSources.push_back(itsSourceList[i]);
	}
	ASKAPLOG_INFO_STR(logger, "MASTER: edgeSources.size="<<edgeSources.size()<<
			  " goodSources.size="<<goodSources.size());
	itsSourceList.clear();

	duchamp::FitsHeader head = itsCube.getHead();
	float noise;
	if(itsCube.pars().getFlagUserThreshold()) noise = 1.;
	else noise = itsCube.stats().getStddev();
	float threshold;
	if(itsCube.pars().getFlagUserThreshold()) threshold = itsCube.pars().getThreshold();
	else threshold = itsCube.stats().getThreshold();

	if(edgeSources.size()>0){ // if there are edge sources

	  for(int i=0;i<edgeSources.size();i++){
	    duchamp::Detection obj = edgeSources[i];
	    obj.addOffsets();
	    itsCube.addObject(obj);
	  }
	  
	  ASKAPLOG_INFO_STR(logger, "MASTER: num sources in cube = "<<itsCube.getNumObj());
	  itsCube.ObjectMerger();
	  this->calcObjectParams();
	  ASKAPLOG_INFO_STR(logger, "MASTER: num sources in cube = "<<itsCube.getNumObj());

	  for(int i=0;i<itsCube.getNumObj();i++){
	    ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source #"<<i+1<<".");
	    sourcefitting::RadioSource src(itsCube.getObject(i));
	    //	    src.setNoiseLevel( noise );
	    src.setDetectionThreshold( threshold );
	    src.setHeader( head );
	    
	    if(itsFlagDoFit){
// 	      if( src.setFluxArray(&(this->itsVoxelList)) ){
// 		src.fitGauss();
// 	      }
	      src.fitGauss(&this->itsVoxelList);
	    }
	    itsSourceList.push_back(src);
	  }
	  
	}

	for(int i=0;i<goodSources.size();i++){
	  itsSourceList.push_back(goodSources[i]);
	}

	std::stable_sort(this->itsSourceList.begin(), this->itsSourceList.end());

	itsCube.clearDetectionList();
	for(int i=0;i<this->itsSourceList.size();i++){
	  this->itsSourceList[i].setID(i+1);
	  itsCube.addObject(duchamp::Detection(itsSourceList[i]));
	}

      }
      

    }



    //**************************************************************//

    void DuchampParallel::calcObjectParams()
    {

      int numVox = itsVoxelList.size();
      int numObj = itsCube.getNumObj();
      std::vector<PixelInfo::Voxel> templist[numObj];
      for(int i=0;i<itsCube.getNumObj();i++){ 
	// for each object, make a vector list of voxels that appear in it.
	  
	std::vector<PixelInfo::Voxel> 
	  objVoxList=itsCube.getObject(i).getPixelSet();
	std::vector<PixelInfo::Voxel>::iterator vox;
	// get the fluxes of each voxel
	for(vox=objVoxList.begin();vox<objVoxList.end();vox++){
	  int ct=0;
	  while(ct<numVox && !vox->match(itsVoxelList[ct])){
	    ct++;
	  }
	  if(numVox!=0 && ct==numVox){ // there has been no match -- problem!
	    ASKAPLOG_ERROR(logger, "MASTER: Found a voxel in the object lists that doesn't appear in the base list.");
	  }
	  else vox->setF( itsVoxelList[ct].getF() );
	}
	
	templist[i] = objVoxList;
	
      }
      std::vector< std::vector<PixelInfo::Voxel> > 
	bigVoxSet (templist, templist + numObj);
      itsCube.calcObjectWCSparams(bigVoxSet);
      
    }

    //**************************************************************//

    void DuchampParallel::printResults()
    {
      /// @details The final list of detected objects is written to
      /// the terminal and to the results file in the standard Duchamp
      /// manner.

      if(isMaster()) {
	ASKAPLOG_INFO_STR(logger, "MASTER: Found " << itsCube.getNumObj() << " sources.");
	
	itsCube.prepareOutputFile();
// 	if(itsCube.getNumObj()>0){
// 	  // no flag-setting, as it's hard to do when we don't have
// 	  // all the pixels. Particularly the negative flux flags
// 	  itsCube.sortDetections();
// 	}
	itsCube.outputDetectionList();

	if(itsCube.pars().getFlagKarma()){
	  std::ofstream karmafile(itsCube.pars().getKarmaFile().c_str());
	  for(int i=0;i<itsCube.getNumObj();i++) itsCube.pObject(i)->addOffsets();
	  itsCube.outputDetectionsKarma(karmafile);
	  karmafile.close();
	}

	if(itsFlagDoFit){
	  std::ofstream summaryFile(this->itsSummaryFile.c_str());
	  std::vector<duchamp::Column::Col> columns = itsCube.getFullCols();
	  for(int i=0;i<itsSourceList.size();i++)
	    itsSourceList[i].printSummary(summaryFile, columns, i==0);
	  summaryFile.close();
	}

      }
      else{
      }
    }

    //**************************************************************//

    
    // Condense the lists (on the master)
    void DuchampParallel::condenseLists() 
    {
      /// @details Done only on the master. Receive the list of
      /// detected pixels from the workers and store the objects in
      /// itsCube's objectList. Also store the detected voxels in
      /// itsVoxelList so that the fluxes are available for later use.
      ///
      /// Once all the objects are read, merge the object list to
      /// combine adjacent/overlapping objects and remove unacceptable
      /// ones.

      if(isMaster()) {
        // Get the lists from the workers here
        ASKAPLOG_INFO_STR(logger,  "MASTER: Retrieving lists from workers" );

	if(isParallel()) {
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
				 << numObj << " objects from worker #"<< rank);
	      for(int obj=0;obj<numObj;obj++){
		duchamp::Detection *object = new duchamp::Detection;
		int32 objsize;
		in >> objsize;
		for(int p=0;p<objsize;p++){
		  int32 x,y,z; 
		  float flux;
		  bool inObj;
		  /*  in >> x >> y >> z >> flux; */
		  in >> inObj >> x >> y >> z >> flux; 
		  x += itsSectionList[i-1].getStart(0);
		  y += itsSectionList[i-1].getStart(1);
		  z += itsSectionList[i-1].getStart(2);
		  if(inObj) object->addPixel(x,y,z);
		  PixelInfo::Voxel vox(x,y,z,flux);
		  itsVoxelList.push_back(vox);
		}
		itsCube.addObject(*object);
		delete object;
	      }
	      in.getEnd();

	      ASKAPLOG_INFO_STR(logger, "MASTER: Received list of size " 
				 << numObj << " from worker #"<< rank);
	      ASKAPLOG_INFO_STR(logger, "MASTER: Now have " 
				 << itsCube.getNumObj() << " objects");
	    }
	}
	// Now process the lists
        ASKAPLOG_INFO_STR(logger,  "MASTER: Condensing lists..." );
	if(itsCube.getNumObj()>1) itsCube.ObjectMerger(); 
        ASKAPLOG_INFO_STR(logger,  "MASTER: Condensing lists done" );

     }
      else {
      }
    }

    void DuchampParallel::calcFluxes()
    {
      /// @details Done on the master. Calculate the fluxes for each
      /// of the objects by constructing voxelLists. The WCS
      /// parameters for each object are also calculated here, so the
      /// WCS header information stored by the master must be
      /// correct. The objects are then ordered by velocity. 
      ///
      /// In the parallel case, we do not set any object flags, as we
      /// don't have all the flux information. This may be possible if
      /// we pass all surrounding pixels as well as detected pixels,
      /// but at the moment, no flags are set. In the serial case,
      /// they are.

      if(isMaster()){

	if(isParallel()) {

	  int numVox = itsVoxelList.size();
	  int numObj = itsCube.getNumObj();
	  std::vector<PixelInfo::Voxel> templist[numObj];

	  ASKAPLOG_INFO_STR(logger, "MASTER: Calculating fluxes of " 
			     << numObj << " detected objects.");

	  for(int i=0;i<itsCube.getNumObj();i++){ // for each object
	  
	    std::vector<PixelInfo::Voxel> 
	      objVoxList=itsCube.getObject(i).getPixelSet();
	    std::vector<PixelInfo::Voxel>::iterator vox;
	    // get the fluxes of each voxel
	    for(vox=objVoxList.begin();vox<objVoxList.end();vox++){
	      int ct=0;
	      while(ct<numVox && !vox->match(itsVoxelList[ct])){
		ct++;
	      }
	      if(numVox!=0 && ct==numVox){ // there has been no match -- problem!
		ASKAPLOG_ERROR(logger, "MASTER: Found a voxel in the object lists that doesn't appear in the base list.");
	      }
	      else vox->setF( itsVoxelList[ct].getF() );
	    }

	    templist[i] = objVoxList;

	  }
	  std::vector< std::vector<PixelInfo::Voxel> > 
	    bigVoxSet (templist, templist + numObj);

	  itsCube.prepareOutputFile();
	  if(itsCube.getNumObj()>0){
	    // no flag-setting, as it's hard to do when we don't have
	    // all the pixels. Particularly the negative flux flags
	    itsCube.calcObjectWCSparams(bigVoxSet);
	    itsCube.sortDetections();
	  }

	}
	else {
	  
	  itsCube.prepareOutputFile();
	  if(itsCube.getNumObj()>0){
	    itsCube.calcObjectWCSparams();
	    itsCube.setObjectFlags();
	    itsCube.sortDetections();
	  }
	}
      }      
    }

//     void DuchampParallel::printResults()
//     {
//       /// @details The final list of detected objects is written to
//       /// the terminal and to the results file in the standard Duchamp
//       /// manner.

//       if(isMaster()) {
// 	ASKAPLOG_INFO_STR(logger, "MASTER: Found " << itsCube.getNumObj() << " sources.");
	
// 	itsCube.outputDetectionList();

// 	if(itsCube.pars().getFlagKarma()){
// 	  std::ofstream karmafile(itsCube.pars().getKarmaFile().c_str());
// 	  itsCube.outputDetectionsKarma(karmafile);
// 	  karmafile.close();
// 	}

//       }
//       else{
//       }
//     }
    
//     void DuchampParallel::fitSources()
//     {
//       /// @details The final list of detected objects is fitted with
//       /// Gaussians (or other functions) using the RadioSource class.

//       if(isMaster() && itsFlagDoFit) {
// 	ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source profiles.");

// 	duchamp::FitsHeader head = itsCube.getHead();
// 	float noise;
// 	if(itsCube.pars().getFlagUserThreshold()) noise = 1.;
// 	else noise = itsCube.stats().getStddev();
// 	float threshold;
// 	if(itsCube.pars().getFlagUserThreshold()) threshold = itsCube.pars().getThreshold();
// 	else threshold = itsCube.stats().getThreshold();

//  	for(int i=0;i<itsCube.getNumObj();i++){
// 	  ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source #"<<i+1<<".");

// 	  sourcefitting::RadioSource src(itsCube.getObject(i));
// 	  src.setNoiseLevel( noise );
// 	  src.setDetectionThreshold( threshold );
// 	  src.setHeader( &head );
// 	  if( src.setFluxArray(&(this->itsVoxelList)) ){
// 	    src.fitGauss();
// 	    itsSourceList.push_back(src);
// 	  }
// 	}

// 	std::vector<sourcefitting::RadioSource>::iterator src;
// 	std::ofstream summaryFile(this->itsSummaryFile.c_str());
// 	std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();
// 	int prec=columns[duchamp::Column::FINT].getPrecision();
// 	if(prec < 6)
// 	  for(int i=prec;i<6;i++) columns[duchamp::Column::FINT].upPrec();
// 	prec=columns[duchamp::Column::FPEAK].getPrecision();
// 	if(prec < 6)
// 	  for(int i=prec;i<6;i++) columns[duchamp::Column::FPEAK].upPrec();
// 	columns[duchamp::Column::NUM].printTitle(summaryFile);
// 	columns[duchamp::Column::RA].printTitle(summaryFile);
// 	columns[duchamp::Column::DEC].printTitle(summaryFile);
// 	columns[duchamp::Column::VEL].printTitle(summaryFile);
// 	columns[duchamp::Column::FINT].printTitle(summaryFile);
// 	columns[duchamp::Column::FPEAK].printTitle(summaryFile);
// 	int width = columns[duchamp::Column::NUM].getWidth() + 
// 	  columns[duchamp::Column::RA].getWidth() + 
// 	  columns[duchamp::Column::DEC].getWidth() +
// 	  columns[duchamp::Column::VEL].getWidth() +
// 	  columns[duchamp::Column::FINT].getWidth() +
// 	  columns[duchamp::Column::FPEAK].getWidth();
// 	summaryFile << " #Fit  F_int (fit)   F_pk (fit)\n";
// 	summaryFile << std::setfill('-') << std::setw(width) << '-'
// 		    << "-------------------------------\n";
// 	for(src=itsSourceList.begin();src<itsSourceList.end();src++){
//  	  src->printSummary(summaryFile, columns);
// 	}

// 	this->writeFitAnnotation();

//       }
//       else {
//       }
      
//     }


    void DuchampParallel::writeFitAnnotation()
    {
      /// @details This function writes a Karma annotation file
      /// showing the location and shape of the fitted 2D Gaussian
      /// components. It makes use of the
      /// RadioSource::writeFitToAnnotationFile() function. The file
      /// written to is given by the input parameter
      /// fitAnnotationFile.

      if(itsSourceList.size()>0){
	
	std::ofstream outfile(this->itsFitAnnotationFile.c_str());
	ASKAPLOG_INFO_STR(logger, "Writing to annotation file: " << this->itsFitAnnotationFile );
	outfile << "COLOR BLUE\n";
	outfile << "COORD W\n";
	outfile << "PA SKY\n";
	std::vector<sourcefitting::RadioSource>::iterator src;
	for(src=itsSourceList.begin();src<itsSourceList.end();src++){
	  outfile << "# Source " << int(src-itsSourceList.begin())+1 << ":\n";
	  src->writeFitToAnnotationFile(outfile);
	}
	
	outfile.close();
	
      }

    }

    
    void DuchampParallel::gatherStats()
    {
      /// @details A front-end function that calls all the statistics
      /// functions. Net effect is to find the mean/median and
      /// rms/MADFM for the entire dataset and store these values in
      /// the master's itsCube statsContainer.

      if(!itsCube.pars().getFlagUserThreshold()){
	findMeans();
	combineMeans();
	broadcastMean();
	findRMSs();
	combineRMSs();
      }
      else itsCube.stats().setThreshold( itsCube.pars().getThreshold() );
    }


    void DuchampParallel::findMeans()
    {
      /// @details Find the mean or median (according to the
      /// flagRobustStats parameter) of the worker's image/cube, then
      /// send to the master via LOFAR Blobs.

      if(isWorker()) {
	ASKAPLOG_INFO_STR(logger, "Finding mean: worker " << this->itsRank);

	if(itsCube.pars().getFlagATrous()) itsCube.ReconCube();
	else if(itsCube.pars().getFlagSmooth()) itsCube.SmoothCube();

	int32 size = itsCube.getSize();
	float mean = 0.,rms;
	float *array;
	// make a mask in case there are blank pixels.
	bool *mask = itsCube.pars().makeBlankMask(itsCube.getArray(), itsCube.getSize());

	if(size>0){
	  
	  if(itsCube.pars().getFlagATrous())       array = itsCube.getArray();
	  else if (itsCube.pars().getFlagSmooth()) array = itsCube.getRecon();
	  else                                     array = itsCube.getArray();
	  
	  // calculate both mean & rms, but ignore rms for the moment.
	  if(itsCube.pars().getFlagRobustStats()) findMedianStats(array,size,mask,mean,rms);
	  else                                    findNormalStats(array,size,mask,mean,rms);

	}
	double dmean = mean;

	ASKAPLOG_INFO_STR(logger, "#" << this->itsRank << ": Mean = " << mean );
	
	if(isParallel()) {
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
	  // serial case
	  if(itsCube.pars().getFlagRobustStats()) itsCube.stats().setMedian(mean);
	  else itsCube.stats().setMean(mean);
	  itsCube.stats().setRobust(itsCube.pars().getFlagRobustStats());
	}

      }
      else {
      }
    }

    void DuchampParallel::findRMSs() 
    {
      /// @details Find the rms or the median absolute deviation from
      /// the median (MADFM) (dictated by the flagRobustStats
      /// parameter) of the worker's image/cube, then send to the
      /// master via LOFAR Blobs. To calculate the rms/MADFM, the mean
      /// of the full dataset must be read from the master (again
      /// passed via LOFAR Blobs). The calculation uses the
      /// findSpread() function.

      if(isWorker()) {
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
	  mean = itsCube.stats().getMiddle();
	}
	// use it to calculate the rms for this section
 	int32 size = itsCube.getSize();
	double rms = 0.;
	float *array;

	if(size > 0){
	  
	  if(itsCube.pars().getFlagATrous()){
	    array = new float[size];
	    for(int i=0;i<size;i++) array[i] = itsCube.getPixValue(i) - itsCube.getReconValue(i);
	  }
	  else if (itsCube.pars().getFlagSmooth()) array = itsCube.getRecon();
	  else array = itsCube.getArray();
	  if(itsCube.pars().getFlagBlankPix()){
	    bool *mask = itsCube.pars().makeBlankMask(array, itsCube.getSize());
	    rms = findSpread(itsCube.pars().getFlagRobustStats(),mean,size,array,mask);
	  }
	  else rms = findSpread(itsCube.pars().getFlagRobustStats(),mean,size,array);

	}

	ASKAPLOG_INFO_STR(logger, "#" << this->itsRank << ": rms = " << rms );

	// return it to the master
	if(isParallel()) {
	  LOFAR::BlobString bs2;
	  bs2.resize(0);
	  LOFAR::BlobOBufString bob(bs2);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("rmsW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << rms << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs2);
	  //	  itsConnectionSet->write(this->itsRank,bs2);
	  ASKAPLOG_INFO_STR(logger, "Sent local rms to the master from worker " << this->itsRank );
	}
	else{
	  //serial case
	  if(itsCube.pars().getFlagRobustStats()) itsCube.stats().setMadfm(rms);
	  else itsCube.stats().setStddev(rms);
	}

      }
      else {
      }

    }

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
	itsCube.stats().setMean(av);
      }
      else {
      }
    }
	
    void DuchampParallel::broadcastMean() 
    {
      /// @details The mean/median value of the full dataset is sent
      /// via LOFAR Blobs to the workers.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	double av = itsCube.stats().getMean();
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
	itsCube.stats().setStddev(rms);

	double av = itsCube.stats().getMean();
	double threshold = av + rms * itsCube.pars().getCut();
	itsCube.stats().setThreshold(threshold);
	itsCube.pars().setFlagUserThreshold(true);
	itsCube.pars().setThreshold(threshold);

	ASKAPLOG_INFO_STR(logger, "MASTER: OVERALL RMS = " << rms);

      }
    }

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
	double threshold = itsCube.stats().getThreshold();
	out << threshold;
	out.putEnd();
	itsConnectionSet->writeAll(bs);
	ASKAPLOG_INFO_STR(logger, "MASTER: Sent threshold (" 
			   << itsCube.stats().getThreshold() << ") from the master" );
      }
      else {
      }
    }


    void DuchampParallel::receiveThreshold()
    {
      /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
      
     if(isWorker()) {

	double threshold;
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  itsConnectionSet->read(0, bs);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("threshM2W");
	  ASKAPASSERT(version==1);
	  in >> threshold;
	  in.getEnd();

	  itsCube.pars().setFlagUserThreshold(true);
	}
	else{
	  if(itsCube.pars().getFlagUserThreshold())
	    threshold = itsCube.pars().getThreshold();
	  else
	    threshold = itsCube.stats().getMiddle() + itsCube.stats().getSpread()*itsCube.pars().getCut();
	}

	ASKAPLOG_INFO_STR(logger, "Setting threshold on worker " << itsRank << " to be " << threshold);

	itsCube.pars().setThreshold(threshold);
     }
    }


  }
}
