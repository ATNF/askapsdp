/// @file
///
/// @brief Utility functions to support use of LOFAR Blobs in the analysis software
/// @details
/// These functions are not part of any classes, but provide
/// ways for existing objects to be passed over LOFAR Blobs.
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
#include <analysisutilities/BlobRelated.h>
#include <sourcefitting/RadioSource.h>

#include <string>
#include <vector>
#include <utility>

#include <duchamp/Detection/detection.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <APS/Exceptions.h>

#include <scimath/Functionals/Gaussian2D.h>
#include <casa/namespace.h>

namespace askap
{
  namespace analysis
  {

    LOFAR::BlobOStream& sourcefitting::operator<<(LOFAR::BlobOStream& blob, RadioSource& src)
    {
      int32 l;
      int i;
      float f;
      std::string s;
      bool b;
      Double d;
      int size = src.getSize();
      blob << size;
      std::vector<PixelInfo::Voxel> pixelSet = src.getPixelSet();
      for(i=0;i<size;i++){
	l = pixelSet[i].getX(); blob << l; 
	l = pixelSet[i].getY(); blob << l;
	l = pixelSet[i].getZ(); blob << l;
      }
      l = src.xSubOffset; blob << l;
      l = src.ySubOffset; blob << l;
      l = src.zSubOffset; blob << l;
      f = src.totalFlux;  blob << f;
      f = src.intFlux;    blob << f;
      f = src.peakFlux;   blob << f;
      f = src.peakSNR;    blob << f;
      l = src.xpeak;	  blob << l;
      l = src.ypeak;      blob << l;
      l = src.zpeak;      blob << l;
      f = src.xCentroid;  blob << f;
      f = src.yCentroid;  blob << f;
      f = src.zCentroid;  blob << f;
      s = src.centreType; blob << s;
      s = src.flagText;   blob << s;
      b = src.negSource;  blob << b;
      b = src.flagWCS;    blob << b;
      b = src.specOK;     blob << b;
      i = src.id;         blob << i;
      s = src.name;       blob << s;
      s = src.raS;        blob << s;
      s = src.decS;       blob << s;
      f = src.ra;         blob << f;
      f = src.dec;        blob << f;
      f = src.raWidth;    blob << f;
      f = src.decWidth;   blob << f;
      f = src.vel;        blob << f;
      f = src.velWidth;   blob << f;
      f = src.velMin;     blob << f;
      f = src.velMax;     blob << f;
      i = src.posPrec;    blob << i;
      i = src.xyzPrec;    blob << i;
      i = src.fintPrec;   blob << i;
      i = src.fpeakPrec;  blob << i;
      i = src.velPrec;    blob << i;
      i = src.snrPrec;    blob << i;

      b = src.hasFit;     blob << b;
      b = src.atEdge;     blob << b;
      f = src.itsDetectionThreshold; blob << f;
      f = src.itsNoiseLevel; blob << f;

      i = src.itsGaussFitSet.size(); blob << i;
      std::vector<casa::Gaussian2D<Double> >::iterator fit = src.itsGaussFitSet.begin();
      for(; fit<src.itsGaussFitSet.end(); fit++){
	d = fit->height();     blob << d;
	d = fit->xCenter();    blob << d;
	d = fit->yCenter();    blob << d;
	d = fit->majorAxis();  blob << d;
	d = fit->axialRatio(); blob << d;
	d = fit->PA();         blob << d;
      }

//       l = src.itsBoxMargins[0].first;    blob << l;
//       l = src.itsBoxMargins[0].second;   blob << l;
//       l = src.itsBoxMargins[1].first;    blob << l;
//       l = src.itsBoxMargins[1].second;   blob << l;
//       l = src.itsBoxMargins[2].first;    blob << l;
//       l = src.itsBoxMargins[2].second;   blob << l;

      return blob;
    }

    LOFAR::BlobIStream& sourcefitting::operator>>(LOFAR::BlobIStream &blob, RadioSource& src)
    {

      int i;
      int32 l;
      bool b;
      float f;
      Double d;
      std::string s;
      int32 size;
      blob >> size;
      for(i=0;i<size;i++){
	int32 x,y,z;
	blob >> x;
	blob >> y;
	blob >> z;
	src.addPixel(x,y,z);
      }
      blob >> l; src.xSubOffset = l;
      blob >> l; src.ySubOffset = l;
      blob >> l; src.zSubOffset = l;
      blob >> f; src.totalFlux = f;
      blob >> f; src.intFlux = f;
      blob >> f; src.peakFlux = f;
      blob >> f; src.peakSNR = f;
      blob >> l; src.xpeak = l;
      blob >> l; src.ypeak = l;
      blob >> l; src.zpeak = l;
      blob >> f; src.xCentroid = f;
      blob >> f; src.yCentroid = f;
      blob >> f; src.zCentroid = f;
      blob >> s; src.centreType = s;
      blob >> b; src.negSource = b;
      blob >> s; src.flagText = s;
      blob >> b; src.flagWCS = b;
      blob >> b; src.specOK = b;
      blob >> i; src.id = i;
      blob >> s; src.name = s;
      blob >> s; src.raS = s;
      blob >> s; src.decS = s;
      blob >> f; src.ra = f;
      blob >> f; src.dec = f;
      blob >> f; src.raWidth = f;
      blob >> f; src.decWidth = f;
      blob >> f; src.vel = f;
      blob >> f; src.velWidth = f;
      blob >> f; src.velMin = f;
      blob >> f; src.velMax = f;
      blob >> i; src.posPrec = i;
      blob >> i; src.xyzPrec = i;
      blob >> i; src.fintPrec = i;
      blob >> i; src.fpeakPrec = i;
      blob >> i; src.velPrec = i;
      blob >> i; src.snrPrec = i;

      blob >> b; src.hasFit = b;
      blob >> b; src.atEdge = b;
      blob >> f; src.itsDetectionThreshold = f;
      blob >> f; src.itsNoiseLevel = f;

      blob >> size;
      src.itsGaussFitSet.clear();
      for(i=0;i<size;i++){
	casa::Gaussian2D<Double> fit;
	blob >> d; fit.setHeight(d);
	blob >> d; fit.setXcenter(d);
	blob >> d; fit.setYcenter(d);
	blob >> d; fit.setMajorAxis(d);
	blob >> d; fit.setAxialRatio(d);
	blob >> d; fit.setPA(d);
	src.itsGaussFitSet.push_back(fit);
      }

//       std::vector<std::pair<long,long> > box;
//       int32 x,y;
//       blob >> x; blob >> y; box[0] = std::pair<long,long>(x,y);
//       blob >> x; blob >> y; box[1] = std::pair<long,long>(x,y);
//       blob >> x; blob >> y; box[2] = std::pair<long,long>(x,y);
//       src.itsBoxMargins = box;

      return blob;
    }

  }

}
