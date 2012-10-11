/// @file
///
/// Implementation of WALLABY's recommended algorithms for optimising
/// the mask of a detected source.
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
#include <parametrisation/OptimisedGrower.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <duchamp/Detection/ObjectGrower.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/PixelMap/Object3D.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <math.h>

#include <Common/ParameterSet.h>

ASKAP_LOGGER(logger, ".optimisedgrower");

using namespace duchamp;

namespace askap {

  namespace analysis {

    OptimisedGrower::OptimisedGrower():
      duchamp::ObjectGrower()
    {
      this->clobberPrevious=true;
    }

    OptimisedGrower::OptimisedGrower(const LOFAR::ParameterSet &parset)
    {
      this->clobberPrevious = parset.getBool("clobberPrevious",true);
      this->maxIterations = parset.getInt16("maxIter",10);
    }

    void OptimisedGrower::setFlag(int x, int y, int z, duchamp::STATE newstate)
    {
      size_t pos = x + this->itsArrayDim[0]*y + this->itsArrayDim[0]*this->itsArrayDim[1]*z;
      this->setFlag(pos,newstate);
    }

    void OptimisedGrower::grow(duchamp::Detection *object)
    {
      this->itsObj = object;
      // this->xObj = this->itsObj->getXPeak();
      // this->yObj = this->itsObj->getYPeak();
      this->xObj = int(object->getXCentroid());
      this->yObj = int(object->getYCentroid());
      int   zObj = int(object->getZCentroid());
      //   std::cout << "Central position = ("<<this->xObj << ","<<this->yObj <<")\n";

      this->findEllipse();

      size_t spatsize=this->itsArrayDim[0]*this->itsArrayDim[1];
      size_t fullsize=spatsize*this->itsArrayDim[2];
      bool keepGoing = true;

      if(this->clobberPrevious){			
	// If we are clobbering the previous object, we start with only
	// the central pixel and then grow out to the ellipse
	this->itsObj = new duchamp::Detection;
	this->itsObj->addPixel(this->xObj,this->yObj,zObj);
	ASKAPLOG_DEBUG_STR(logger, "Starting with single pixel at ("<<xObj<<","<<yObj<<","<<zObj<<") and ellipse of size " << ell_a<<"x"<<ell_b<<"x"<<ell_theta);
	// reset the current objects STATE array pixels to AVAILABLE
	std::vector<Voxel> oldvoxlist=object->getPixelSet();
	for(std::vector<Voxel>::iterator vox=oldvoxlist.begin();vox<oldvoxlist.end();vox++) this->setFlag(*vox,duchamp::AVAILABLE);
      }
      else ASKAPLOG_DEBUG_STR(logger, "Initial object size = " << this->itsObj->getSize());

      for(int iter=0; iter<this->maxIterations && keepGoing; iter++){

	// after growMask, check if flux of newObj is positive.
	// If yes:
	//    - add newObj to current object 
	//    - update the STATE array (each of newObj's pixels -> DETECTED), 
	//    - increment ell_a, ell_b, itercounter
	//    - continue loop.
	// If no, exit loop.
    
	duchamp::Detection newObj = this->growMask();
	newObj.calcFluxes(this->itsFluxArray, &(this->itsArrayDim[0]));
	ASKAPLOG_DEBUG_STR(logger,"Iter#"<<iter<<", flux of new object = "<< newObj.getTotalFlux());
	//	  std::cout << "Adding object:\n"<<newObj;
	keepGoing = (newObj.getTotalFlux() > 0.);
	if(keepGoing){
	  this->itsObj->addDetection(newObj);
	  ASKAPLOG_DEBUG_STR(logger,  "Object size now " << this->itsObj->getSize() );

	  std::vector<Voxel> newlist=newObj.getPixelSet();
	  for(size_t i=0;i<newlist.size();i++) this->setFlag(newlist[i],duchamp::DETECTED);
      
	  this->ell_b += (this->ell_b / this->ell_a);
	  this->ell_a += 1.;
	}
      }

      for(size_t i=0;i<fullsize;i++) 
	if(this->itsFlagArray[i]==NEW) this->itsFlagArray[i]=duchamp::AVAILABLE;  // the pixels in newObj were rejected, so set them back to AVAILABLE.

      this->itsObj->calcFluxes(this->itsFluxArray, &(this->itsArrayDim[0]));
     
  
      if(this->clobberPrevious) *object = *this->itsObj;


    }

    void OptimisedGrower::findEllipse()
    {
      // make the moment map for the object and find mom_x, mom_y, mom_xy 

      size_t mapxsize=this->itsObj->getXmax()-itsObj->getXmin()+1;
      size_t mapysize=this->itsObj->getYmax()-itsObj->getYmin()+1;
      float *mom0map = new float[mapxsize*mapysize];
      // std::cout << "Size of moment map = " << mapxsize << "x"<<mapysize<<"\n";
      double mom_x=0., mom_y=0., mom_xy=0., sum=0.;
      size_t mapPos=0;
      for(size_t i=0;i<mapxsize*mapysize;i++) mom0map[i]=0.;
      for (int y=this->itsObj->getYmin(); y<=this->itsObj->getYmax(); y++){
	for (int x=this->itsObj->getXmin(); x<=this->itsObj->getXmax(); x++){
	  for (int z=this->itsObj->getZmin();z<=this->itsObj->getZmax(); z++){
	    if(this->itsObj->isInObject(x,y,z)) 
	      mom0map[mapPos] += this->itsFluxArray[x +
						    y*this->itsArrayDim[0] + 
						    z*this->itsArrayDim[0]*this->itsArrayDim[1]];
	  }
	  int offx=int(x)-this->xObj;
	  int offy=int(y)-this->yObj;
	  if(mom0map[mapPos]>0.){
	    mom_x += offx * offx * mom0map[mapPos];
	    mom_y += offy * offy * mom0map[mapPos];
	    mom_xy += offx * offy * mom0map[mapPos];
	    sum += mom0map[mapPos];	
	  }
	  mapPos++;
	}    
      }
  
      mom_x /= sum;
      mom_y /= sum;
      mom_xy /= sum;

      ASKAPLOG_DEBUG_STR(logger, "Moments: " << mom_x << " " << mom_y << " " << mom_xy << " and sum = " << sum);

      // find ellipse parameters
      this->ell_theta = 0.5 * atan2(2.0 * mom_xy, mom_x - mom_y);
      this->ell_a = sqrt(2.0 * (mom_x + mom_y + sqrt(((mom_x - mom_y) * (mom_x - mom_y)) + (4.0 * mom_xy * mom_xy))));
      this->ell_b = sqrt(2.0 * (mom_x + mom_y - sqrt(((mom_x - mom_y) * (mom_x - mom_y)) + (4.0 * mom_xy * mom_xy))));

      this->ell_b = std::max(this->ell_b,0.1);

      ASKAPLOG_DEBUG_STR(logger, "Ellipse : " << this->ell_a << " x " << this->ell_b << " , " << this->ell_theta <<" (" << this->ell_theta*180./M_PI << ")");

    }

    duchamp::Detection OptimisedGrower::growMask()
    {
      // --loop over FULL x- and y-range of image--
      // Test each pixel (and each channel for it over object's range, suitably modified):
      //   - is pixel AVAILABLE?
      //   - is pixel within ellipse?
      // If so, add to new object
      // When finished, return object
      // Potential optimisation - copy growing functionality from ObjectGrower - just looking at neighbours of current object's pixels and adding if within ellipse.

      Object2D spatmap = this->itsObj->getSpatialMap();
      Object3D temp3d;
      temp3d.addChannel(0,spatmap);
      std::vector<Voxel> pixlist = temp3d.getPixelSet();
      long zero = 0;
      long xpt,ypt;
      long xmin,xmax,ymin,ymax,x,y,z;
      size_t pos;
      size_t spatsize=this->itsArrayDim[0]*this->itsArrayDim[1];
      duchamp::Detection newObj;
      for(size_t i=0; i<pixlist.size(); i++){
    
	xpt=pixlist[i].getX();
	ypt=pixlist[i].getY();
      
	xmin = size_t(std::max(xpt - this->itsSpatialThresh, zero));
	xmax = size_t(std::min(xpt + this->itsSpatialThresh, long(this->itsArrayDim[0])-1));
	ymin = size_t(std::max(ypt - this->itsSpatialThresh, zero));
	ymax = size_t(std::min(ypt + this->itsSpatialThresh, long(this->itsArrayDim[1])-1));
      
	//loop over surrounding pixels.
	for(x=xmin; x<=xmax; x++){
	  for(y=ymin; y<=ymax; y++){

	    int offx=int(x)-this->xObj;
	    int offy=int(y)-this->yObj;
	    double phi = atan2(offy, offx) - this->ell_theta;
	    double radius = this->ell_a * this->ell_b / 
	      sqrt((this->ell_a * this->ell_a * sin(phi) * sin(phi)) + 
		   (this->ell_b * this->ell_b * cos(phi) * cos(phi))   );

	    for(z=this->zmin; z<=this->zmax; z++){

	      pos=x+y*this->itsArrayDim[0]+z*spatsize;

	      if( this->itsFlagArray[pos] == duchamp::AVAILABLE
		  &&  ((offx*offx+offy*offy)<radius*radius) ){
		this->itsFlagArray[pos] = NEW;
		newObj.addPixel(x,y,z);
		pixlist.push_back(Voxel(x,y,0));
	      }
	    }
	  }
	}
      }

      // std::cout << newObj <<"\n";
  
      return newObj;

    }


  }
}
