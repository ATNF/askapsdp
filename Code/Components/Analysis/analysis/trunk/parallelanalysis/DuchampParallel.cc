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

      itsFlagDoFit = parset.getBool("doFit", false);
      this->itsSummaryFile = parset.getString("summaryFile", "duchamp-Summary.txt");
      this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-Results-Fits.ann");

      itsCube.pars().setVerbosity(false);
      itsCube.pars().setFlagLog(true);

      // Now read the correct image name according to worker/master state.
      if(isMaster()){
	itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );
	itsCube.pars().setFlagRobustStats(false);
	if(isParallel()) itsSectionList = makeSubImages(itsNNode-1,parset);
// 	itsImage = substitute(parset.getString("masterImage"));
	itsImage = substitute(parset.getString("image"));
	itsCube.pars().setImageFile(itsImage);

	/// The sectionInfo, read by the master, is interpreted by the
	/// function readSectionInfo(). See its description for a
	/// description of the format of the sectionInfo file, and how
	/// the separated data is interpreted.
// 	string sectionInfo = substitute(parset.getString("sectionInfo"));
// 	itsSectionList = readSectionInfo(sectionInfo);
// 	ASKAPLOG_INFO_STR(logger, "Read in the sectionInfo.");
// 	if(itsSectionList.size() == 0){
// 	  ASKAPLOG_ERROR_STR(logger, "No SectionInfo file found. Exiting.");
// 	  exit(0);
// 	}
// 	else if( (isParallel() && (int(itsSectionList.size()) != (itsNNode-1) ))
// 		 || (!isParallel() && (int(itsSectionList.size()) != (itsNNode))) )
// 	  ASKAPLOG_ERROR_STR(logger, "Number of sections provided by " 
// 			      << sectionInfo 
// 			      << " does not match the number of images being processed.");

      }

      if(isWorker()) {
 	 itsImage = substitute(parset.getString("image"));
	if(isParallel()) itsImage = getSubImageName(itsImage,itsRank-1,itsNNode-1);
	
	itsCube.pars().setImageFile(itsImage);
	//	splitImage(parset);

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

    void DuchampParallel::splitImage(const LOFAR::ACC::APS::ParameterSet& parset)
    {
      /// @details Generates a subsection string for the current
      /// worker based on the number of nodes and the requested
      /// distribution of subimages.
      ///
      /// @todo Enable the overlap to be based on the beam size?

      if(isParallel() && isWorker()){

	int nsubx = parset.getInt16("nsubx",1);
	int nsuby = parset.getInt16("nsuby",1);
	int nsubz = parset.getInt16("nsubz",1);
      
	int overlapx = parset.getInt16("overlapx",0);
	int overlapy = parset.getInt16("overlapy",0);
	int overlapz = parset.getInt16("overlapz",0);

	int numRequestedSubs = nsubx * nsuby * nsubz;
	int numWorkers = itsNNode - 1;

	if( numWorkers != numRequestedSubs )
	  ASKAPLOG_INFO_STR(logger, "Requested number of subsections ("<<numRequestedSubs
			    <<") doesn't match number of workers (" << numWorkers<<"). Not doing splitting.");

	else {

	  long *dimAxes = getFITSdimensions(itsImage);
	  /// @todo Note that we are assuming a particular axis setup here. Make this more robust!
	  long start = 0;
	  std::stringstream section;
	  section << "[";
	  if(nsubx>1){
	    int x1 = std::max( start , (itsRank-1)*dimAxes[0]/nsubx - overlapx/2 );
	    int x2 = std::min( dimAxes[0] , itsRank*dimAxes[0]/nsubx + overlapx/2 );
	    section << x1+1 << ":" << x2 << ",";
	  }
	  else section << "*,";

	  if(nsuby>1){
	    int y1 = std::max( start , (itsRank-1)*dimAxes[1]/nsuby - overlapy/2 );
	    int y2 = std::min( dimAxes[1] , itsRank*dimAxes[1]/nsuby + overlapy/2 );
	    section << y1+1 << ":" << y2 << "," ;
	  }
	  else section << "*,";

	  if(nsubz>1){
	    int z1 = std::max( start , (itsRank-1)*dimAxes[2]/nsubz - overlapz/2 );
	    int z2 = std::min( dimAxes[2] , itsRank*dimAxes[2]/nsubz + overlapz/2 );
	    section << z1+1 << ":" << z2 << "]";
	  }
	  else section << "*]";

	  itsCube.pars().setFlagSubsection(true);
	  itsCube.pars().setSubsection(section.str());
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<itsRank<<" is using subsection " << section.str());

	}

      }
      

    }


    // Read in the data from the image file (on the workers)
    void DuchampParallel::readData()
    {
      /// @details Reads in the data using duchamp functionality and
      /// the image name defined in the constructor.

      if(isWorker()) {
	
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
	  }
	}

// 	if(isParallel()){
// 	  LOFAR::BlobString bs;
// 	  bs.resize(0);
// 	  LOFAR::BlobOBufString bob(bs);
// 	  LOFAR::BlobOStream out(bob);
// 	  out.putStart("inputDone",1);
// 	  out << true;
// 	  out.putEnd();
// 	  itsConnectionSet->write(0,bs);
// 	}
      }
      else {
	//	if(itsCube.getCube()==duchamp::FAILURE){
	if(itsCube.getMetadata()==duchamp::FAILURE){
	  ASKAPLOG_ERROR_STR(logger, "MASTER: Could not read in metadata from image " << itsImage << ".");
	}
	else {
	  ASKAPLOG_INFO_STR(logger,  "MASTER: Read metadata from image " << itsImage);
	}
	itsCube.header().defineWCS(itsCube.pars().getImageFile(), itsCube.pars());
	itsCube.header().readHeaderInfo(itsCube.pars().getImageFile(), itsCube.pars());

	LOFAR::BlobString bs;
	bool OK=true;
// 	for(int i=1;i<itsNNode && OK;i++){

	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("goInput",1);
	  out << true;
	  out.putEnd();
// 	  itsConnectionSet->write(i-1,bs);
	  itsConnectionSet->writeAll(bs);

// 	  bs.resize(0);
//           itsConnectionSet->read(i-1, bs);
//           LOFAR::BlobIBufString bib(bs);
//           LOFAR::BlobIStream in(bib);
//           int version=in.getStart("inputDone");
//           ASKAPASSERT(version==1);
//           in >> OK;
//           in.getEnd();
	  
// 	}

	if(!OK) ASKAPLOG_ERROR(logger, "MASTER: Error in splitting image.");

      }

    }
      


    // Find the lists (on the workers)
    void DuchampParallel::findLists()
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

	int16 num = itsCube.getNumObj(), rank=this->itsRank;
        ASKAPLOG_INFO_STR(logger,  "Found " << num << " objects in worker " << this->itsRank);
	
        // Send the lists to the master here
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("detW2M",1);
	  out << rank << num;
	  for(int i=0;i<itsCube.getNumObj();i++){
	    /*
	    std::vector<PixelInfo::Voxel> voxlist = itsCube.getObject(i).getPixelSet();
	    std::vector<PixelInfo::Voxel>::iterator vox;
	    int32 size = voxlist.size();
	    out << size;
	    for(vox=voxlist.begin();vox<voxlist.end();vox++){
	      int32 x,y,z;  // should be long but problem with Blobs
	      x = vox->getX(); 
	      y = vox->getY(); 
	      z = vox->getZ();
 	      out << x << y << z << itsCube.getPixValue(x,y,z);
	    }
	    */

	    int border = sourcefitting::detectionBorder;

	    /// TODO -- abstract this getting-the-surrounding-area into a class?
	    int xmin,xmax,ymin,ymax,zmin,zmax;
	    xmin = std::max(0 , int(itsCube.getObject(i).getXmin()-border));
	    xmax = std::min(itsCube.getDimX()-1, itsCube.getObject(i).getXmax()+border);
	    ymin = std::max(0 , int(itsCube.getObject(i).getYmin()-border));
	    ymax = std::min(itsCube.getDimY()-1, itsCube.getObject(i).getYmax()+border);
	    zmin = std::max(0 , int(itsCube.getObject(i).getZmin()-border));
	    zmax = std::min(itsCube.getDimZ()-1, itsCube.getObject(i).getZmax()+border);

	    int numVox = (xmax-xmin+1)*(ymax-ymin+1)*(zmax-zmin+1);
	    out << numVox;
	    
	    for(int32 x=xmin; x<=xmax; x++){
	      for(int32 y=ymin; y<=ymax; y++){
		for(int32 z=zmin; z<=zmax; z++){
		  
		  bool inObject = itsCube.getObject(i).pixels().isInObject(x,y,z);
		  float flux = itsCube.getPixValue(x,y,z);

		  out << inObject << x << y << z << flux;

		}
	      }
	    }

	  }
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  //	  itsConnectionSet->write(this->itsRank,bs);
	  ASKAPLOG_INFO_STR(logger, "Sent detection list to the master from worker " << this->itsRank );
	}
	else{ // if not parallel, still want to make the voxelList

	  for(int i=0;i<itsCube.getNumObj();i++){
	    int32 border = sourcefitting::detectionBorder;

	    for(int32 x=itsCube.getObject(i).getXmin()-border; x<=itsCube.getObject(i).getXmax()+border; x++){
	      for(int32 y=itsCube.getObject(i).getYmin()-border; y<=itsCube.getObject(i).getYmax()+border; y++){
		for(int32 z=itsCube.getObject(i).getZmin()-border; z<=itsCube.getObject(i).getZmax()+border; z++){
		  
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
      else {
      }
      
    }
    
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

    void DuchampParallel::printResults()
    {
      /// @details The final list of detected objects is written to
      /// the terminal and to the results file in the standard Duchamp
      /// manner.

      if(isMaster()) {
	ASKAPLOG_INFO_STR(logger, "MASTER: Found " << itsCube.getNumObj() << " sources.");
	
	itsCube.outputDetectionList();

	if(itsCube.pars().getFlagKarma()){
	  std::ofstream karmafile(itsCube.pars().getKarmaFile().c_str());
	  itsCube.outputDetectionsKarma(karmafile);
	  karmafile.close();
	}

      }
      else{
      }
    }
    
    void DuchampParallel::fitSources()
    {
      /// @details The final list of detected objects is fitted with
      /// Gaussians (or other functions) using the RadioSource class.

      if(isMaster() && itsFlagDoFit) {
	ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source profiles.");

	duchamp::FitsHeader head = itsCube.getHead();
	float noise;
	if(itsCube.pars().getFlagUserThreshold()) noise = 1.;
	else noise = itsCube.stats().getStddev();
	float threshold;
	if(itsCube.pars().getFlagUserThreshold()) threshold = itsCube.pars().getThreshold();
	else threshold = itsCube.stats().getThreshold();

 	for(int i=0;i<itsCube.getNumObj();i++){
	  ASKAPLOG_INFO_STR(logger, "MASTER: Fitting source #"<<i+1<<".");

	  sourcefitting::RadioSource src;
	  src.setDetection( itsCube.pObject(i) );
	  src.setNoiseLevel( noise );
	  src.setDetectionThreshold( threshold );
	  src.setHeader( &head );
	  if( src.setFluxArray(&(this->itsVoxelList)) ){
	    src.fitGauss();
	    itsSourceList.push_back(src);
	  }
	}

	std::vector<sourcefitting::RadioSource>::iterator src;
	std::ofstream summaryFile(this->itsSummaryFile.c_str());
	std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();
	int prec=columns[duchamp::Column::FINT].getPrecision();
	if(prec < 6)
	  for(int i=prec;i<6;i++) columns[duchamp::Column::FINT].upPrec();
	prec=columns[duchamp::Column::FPEAK].getPrecision();
	if(prec < 6)
	  for(int i=prec;i<6;i++) columns[duchamp::Column::FPEAK].upPrec();
	columns[duchamp::Column::NUM].printTitle(summaryFile);
	columns[duchamp::Column::RA].printTitle(summaryFile);
	columns[duchamp::Column::DEC].printTitle(summaryFile);
	columns[duchamp::Column::VEL].printTitle(summaryFile);
	columns[duchamp::Column::FINT].printTitle(summaryFile);
	columns[duchamp::Column::FPEAK].printTitle(summaryFile);
	int width = columns[duchamp::Column::NUM].getWidth() + 
	  columns[duchamp::Column::RA].getWidth() + 
	  columns[duchamp::Column::DEC].getWidth() +
	  columns[duchamp::Column::VEL].getWidth() +
	  columns[duchamp::Column::FINT].getWidth() +
	  columns[duchamp::Column::FPEAK].getWidth();
	summaryFile << " #Fit  F_int (fit)   F_pk (fit)\n";
	summaryFile << std::setfill('-') << std::setw(width) << '-'
		    << "-------------------------------\n";
	for(src=itsSourceList.begin();src<itsSourceList.end();src++){
 	  src->printSummary(summaryFile, columns);
	}

	this->writeFitAnnotation();

      }
      else {
      }
      
    }


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
	double mean = 0.;
	float *array;

	if(size>0){
	  
	  if(itsCube.pars().getFlagATrous())       array = itsCube.getArray();
	  else if (itsCube.pars().getFlagSmooth()) array = itsCube.getRecon();
	  else                                     array = itsCube.getArray();
	  
	  if(itsCube.pars().getFlagRobustStats()) mean = findMedian(array,size);
	  else                                    mean = findMean(array,size);

	}

	ASKAPLOG_INFO_STR(logger, "#" << this->itsRank << ": Mean = " << mean );
	
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("meanW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << mean << size;
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
	  rms = findSpread(itsCube.pars().getFlagRobustStats(),mean,size,array);

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
