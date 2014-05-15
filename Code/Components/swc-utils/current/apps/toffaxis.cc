// test code to help with off-axis direction calculation

#include <dataaccess/TableDataSource.h>
#include <askap_swcutils.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/ParsetInterface.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>
#include <utils/EigenDecompose.h>
#include <askap/AskapUtil.h>


// casa
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>


// std
#include <stdexcept>
#include <iostream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace askap::accessors;

// the following code was copied from SynthesisParamsHelper. It probably needs to go to Base
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. 8.0arcsec. This method allows
    /// to parse a single string corresponding to such a parameter and return
    /// a double value converted to the requested units.
    /// @param[in] strval input string
    /// @param[in] unit required units (given as a string)
    /// @return converted value
    double convertQuantity(const std::string &strval,
                       const std::string &unit)
    {
       casa::Quantity q;
      
       casa::Quantity::read(q, strval);
       return q.getValue(casa::Unit(unit));
    }
        
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. [8.0arcsec,8.0arcsec]. This method allows
    /// to parse vector of strings corresponding to such parameter and return
    /// a vector of double values in the required units.
    /// @param[in] strval input vector of strings
    /// @param[in] unit required units (given as a string)
    /// @return vector of doubles with converted values
    std::vector<double> convertQuantity(const std::vector<std::string> &strval,
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
//    


/// @brief analyse the uvw
/// @details
/// @param[in] acc accessor 
/// @param[out] nv normalised vector orthogonal to the fitted plane in uvw-coordinates
/// @param[in] beam if positive, only this beam will be taken into account
/// @return largest residual w-term (negative value means the fit has failed)
double analyseUVW(const IConstDataAccessor& acc, casa::Vector<double> &nv, int beam = -1)
{
  nv.resize(3);
  const casa::MVDirection tangent(convertQuantity("12h30m00.000","rad"),
                                  convertQuantity("-45.00.00.000","rad"));
  const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
  const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw = acc.rotatedUVW(tangentDir);
  //const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw = acc.uvw();
  
  casa::Matrix<double> normalMatr(3,3,0.);
  const casa::uInt nRow = acc.nRow();
  for (casa::uInt row = 0; row<nRow; ++row) {
       if ((int(acc.feed1()[row]) != beam) && (beam>=0)) {
           continue;
       }
       normalMatr(0,0) += casa::square(uvw[row](0));
       normalMatr(1,1) += casa::square(uvw[row](1));
       normalMatr(2,2) += casa::square(uvw[row](2));
       normalMatr(0,1) += uvw[row](0)*uvw[row](1);
       normalMatr(0,2) += uvw[row](0)*uvw[row](2);
       normalMatr(1,2) += uvw[row](1)*uvw[row](2);
  }
  normalMatr(1,0) = normalMatr(0,1);
  normalMatr(2,0) = normalMatr(0,2);
  normalMatr(2,1) = normalMatr(1,2);
  
  casa::Vector<double> eVal;
  casa::Matrix<double> eVect;
  scimath::symEigenDecompose(normalMatr,eVal,eVect);
  
  casa::Vector<double> normalVector(eVect.column(2).copy());
  const double norm = casa::sum(casa::square(normalVector));
  normalVector /= norm;
  nv = normalVector;
  if (fabs(normalVector[2]) > 1e-6) {
      normalVector[0] /= normalVector[2];
      normalVector[1] /= normalVector[2];
      normalVector[0] *= -1.;
      normalVector[1] *= -1.;
      //std::cout<<"Best fit plane w = u * "<<normalVector[0]<<" + v * "<<normalVector[1]<<std::endl;
      
      double maxDeviation = -1;
      for (casa::uInt row = 0; row<nRow; ++row) {
           if ((int(acc.feed1()[row]) != beam) && (beam>=0)) {
               continue;
           }
           // residual w-term
           const double resW = uvw[row](2) - uvw[row](0)*normalVector[0] - uvw[row](1) * normalVector[1];
           if (fabs(resW) > maxDeviation) {
               maxDeviation = fabs(resW);
           }
      }
      std::cout<<normalVector[0]<<" "<<normalVector[1]<<" "<<maxDeviation<<" "<<beam<<std::endl;
      return maxDeviation;      
  }
  return -1.;
}


void doReadOnlyTest(const IConstDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseFeed(1);
  //sel<<LOFAR::ParameterSet("test.in").makeSubset("TestSelection.");
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(53635.5,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  const casa::MVDirection tangent(convertQuantity("12h30m00.000","rad"),
                                  convertQuantity("-45.00.00.000","rad"));
  const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       
       casa::Vector<double> nvAll;
       analyseUVW(*it,nvAll,0);
       ASKAPDEBUGASSERT(nvAll.nelements() == 3);
       for (int beam=0;beam<5;++beam) {
            casa::Vector<double> nvCurrentBeam;
            analyseUVW(*it,nvCurrentBeam,beam);
            ASKAPDEBUGASSERT(nvCurrentBeam.nelements() == 3);
            const double dotproduct = nvAll[0]*nvCurrentBeam[0] + nvAll[1]*nvCurrentBeam[1] + nvAll[2]*nvCurrentBeam[2];
            std::cout<<"angle="<<acos(dotproduct)/casa::C::pi*180.<<std::endl;
       }
       /*
       for (casa::uInt row = 0; row<it->nRow(); ++row) {
            std::cout<<"beam="<<it->feed1()[row]<<" direction: "<<printDirection(it->pointingDir1()[row])<<std::endl;
       }
       */

       /*
       const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw = it->rotatedUVW(tangentDir);
       const casa::Vector<casa::uInt>& beamIDs = it->feed1();
       for (casa::uInt row = 0; row<uvw.nelements(); ++row) {
            ASKAPASSERT(beamIDs[row] == it->feed2()[row]);
            std::cout<<beamIDs[row]<<" "<<uvw[row](0)<<" "<<uvw[row](1)<<" "<<uvw[row](2)<<std::endl;
       }
       */
  }
}

// helper method to add extra offset to the list of offsets to form equilateral triangle with two specified points
// there are two possible points (raAdd true and false)
void add3rdOffset(std::vector<double> &xOffsets, std::vector<double> &yOffsets, size_t pt1, size_t pt2, bool raAdd)
{
   ASKAPDEBUGASSERT(pt1 < xOffsets.size());
   ASKAPDEBUGASSERT(pt2 < xOffsets.size());
   ASKAPDEBUGASSERT(xOffsets.size() == yOffsets.size());
   const double xNew = xOffsets[pt1] + 0.5 * (xOffsets[pt2] - xOffsets[pt1]) + (raAdd ? +1. : -1.) * 
                       (yOffsets[pt2] - yOffsets[pt1]);
   const double yNew = yOffsets[pt1] + 0.5 * (yOffsets[pt2] - yOffsets[pt1]) + (raAdd ? -1. : +1.) * 
                       (xOffsets[pt2] - xOffsets[pt1]);
   xOffsets.push_back(xNew);
   yOffsets.push_back(yNew);
}

void makeRaster() {
  // Virgo
  //casa::MVDirection tangent(convertQuantity("12h30m49.43","rad"),convertQuantity("12.23.29.1","rad"));
  // 0407-658
  casa::MVDirection tangent(convertQuantity("04h08m20.38","rad"),convertQuantity("-65.45.09.1","rad"));

  //const double size = 4.9; // in degrees
  const double size = 8.4; // in degrees

  std::cout<<"Making a raster file for "<<size<<" by "<<size<<" deg about tangent: "<<printDirection(tangent)<<std::endl;
  const double resolution = 0.5; // in degrees
  // we always include 0 offset, so the number of points each side will always be odd
  const int halfNOffsets = int(size/2./resolution);
  const int nOffsets = 2*halfNOffsets+1;
  std::cout<<"Will include "<<nOffsets<<" points each side"<<std::endl;
  std::ofstream os("rasterpointings.dat");
  os<<"# pointings for the "<<nOffsets*resolution<<" by "<<nOffsets*resolution<<" deg raster around "<<printDirection(tangent)<<std::endl;
  os<<"# resolution = "<<resolution<<" degrees, "<<nOffsets<<" pointings each side"<<std::endl;
  os<<"# columns are RA2000 DEC2000 SequenceNum x y"<<std::endl;
  size_t counter = 0;
  const double resolutionInRad = resolution / 180. * casa::C::pi;
  for (int x = -halfNOffsets; x <= halfNOffsets; ++x) {
       const int dir = (x + halfNOffsets) % 2 == 0 ? 1 : -1; // the first scan is in the increasing order
       for (int y = -halfNOffsets; y <= halfNOffsets; ++y) {
            const double xOffset = double(x) * resolutionInRad;
            const double yOffset = double(y * dir) * resolutionInRad;
            casa::MVDirection testDir(tangent);
            testDir.shift(xOffset,yOffset, casa::True);
            ++counter; // effectively a 1-based counter
            os<<testDir.getLong()/casa::C::pi*180.<<" "<<testDir.getLat()/casa::C::pi*180.<<" "<<counter<<" "<<x<<" "<<y * dir<<std::endl;
       }
  }
}

void doTest() {
  // 1549-790
  const casa::MVDirection tangent(convertQuantity("15h56m58.871","rad"), convertQuantity("-79.14.04.28","rad"));
  // 1934-638
  //const casa::MVDirection tangent(convertQuantity("19h39m25.03","rad"),convertQuantity("-63.42.45.6","rad"));
  //const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
  
  // 1610-771
  const casa::MVDirection dir(convertQuantity("16h17m49.278","rad"), convertQuantity("-77.17.18.46","rad"));

  // 1936-623
  //const casa::MVDirection dir(convertQuantity("19h41m21.77","rad"),convertQuantity("-62.11.21.06","rad"));
  
  // 1547-795
  //const casa::MVDirection dir(convertQuantity("15h55m21.65","rad"), convertQuantity("-79.40.36.3","rad"));
  

  //casa::MVDirection testDir = tangent;
  // Virgo
  //casa::MVDirection testDir(convertQuantity("12h30m49.43","rad"),convertQuantity("12.23.28.01","rad"));
  // 1934-638
  //casa::MVDirection testDir(convertQuantity("19h39m25.03","rad"),convertQuantity("-63.42.45.6","rad"));

  //casa::MVDirection testDir(convertQuantity("07h12m43.98","rad"),convertQuantity("18.57.52.4","rad"));
  //casa::MVDirection testDir(convertQuantity("15h39m16.97","rad"),convertQuantity("-78.42.06.17","rad"));

  /*
  // Sun @ MRO
  //casa::MPosition mroPos(casa::MVPosition(casa::Quantity(370.81, "m"), casa::Quantity(116.6310372795, "deg"),
  //                       casa::Quantity(-26.6991531922, "deg")), casa::MPosition::Ref(casa::MPosition::WGS84));
  casa::MPosition mroPos(casa::MVPosition(casa::Quantity(370.81, "m"), casa::Quantity(-26.6991531922, "deg"), 
           casa::Quantity(116.6310372795, "deg")),casa::MPosition::Ref(casa::MPosition::WGS84));
  casa::Quantity q;
  ASKAPCHECK(casa::MVTime::read(q, "today"), "MVTime::read failed");
  std::cout<<"Current UT MJD: "<<q<<std::endl;
  casa::MEpoch when(casa::MVEpoch(q), casa::MEpoch::Ref(casa::MEpoch::UTC));
  casa::MeasFrame frame(mroPos, when);
  casa::MVDirection testDir = casa::MDirection::Convert(casa::MDirection(casa::MDirection::SUN), casa::MDirection::Ref(casa::MDirection::J2000,frame))().getValue();
 
   */
  // put Sun position here for beamforming
  casa::MVDirection testDir(convertQuantity("20h54m47.18","rad"),convertQuantity("-17.24.01.5","rad"));

  std::cout<<"tangent point: "<<printDirection(tangent)<<std::endl;
  std::cout<<"dir: "<<printDirection(dir)<<std::endl;
  std::cout<<"test direction: "<<printDirection(testDir)<<std::endl;

  double offset1 = 0., offset2 = 0.;
  const double factor = -1;

  offset1 = sin(dir.getLong() - tangent.getLong()) * cos(dir.getLat());
  offset2 = sin(dir.getLat()) * cos(tangent.getLat()) - cos(dir.getLat()) * sin(tangent.getLat())
                                                  * cos(dir.getLong() - tangent.getLong());
  
  /*
  // for explicit offsets
  const double ofq = sqrt(3.)/2.;
  offset1 = ofq / 180. * casa::C::pi;
  offset2 = -0.5 / 180. * casa::C::pi;
  */
  
  std::cout<<"separation (dir vs. tangent): "<<dir.separation(tangent)*180./casa::C::pi<<" deg, offsets (deg): "
     <<offset1*180./casa::C::pi<<" "<<offset2*180./casa::C::pi<<std::endl;
  
  const casa::MVDirection backupTestDir(testDir);

  testDir.shift(offset1*factor,offset2*factor, casa::True);
  std::cout<<"offset applied to single test direction: "<<printDirection(testDir)<<std::endl;

  // for multiple offsets
  std::vector<double> xOffsets, yOffsets;

  /*
   // first 9-beam config
  xOffsets.push_back(0.); yOffsets.push_back(0.);
  xOffsets.push_back(-ofq); yOffsets.push_back(-0.5);
  xOffsets.push_back(-ofq); yOffsets.push_back(0.5);
  xOffsets.push_back(-ofq); yOffsets.push_back(1.5);
  xOffsets.push_back(0.); yOffsets.push_back(1.);
  xOffsets.push_back(ofq); yOffsets.push_back(1.5);
  xOffsets.push_back(ofq); yOffsets.push_back(0.5);
  xOffsets.push_back(ofq); yOffsets.push_back(-0.5);
  //xOffsets.push_back(0.); yOffsets.push_back(-1.5);
  xOffsets.push_back(0.); yOffsets.push_back(-1.);
  */

  /*
  // line of beams in HA
  // off-position as the first one
  xOffsets.push_back(20.); yOffsets.push_back(0.);
  const double fullOffset = 2.7;
  for (size_t pt = 0; pt<9; ++pt) {
       xOffsets.push_back(fullOffset / 9. * double(pt));
       yOffsets.push_back(0.);
  }
  */

  const double offset1inDeg = offset1 / casa::C::pi * 180.;
  const double offset2inDeg = offset2 / casa::C::pi * 180.;

  
  // for our standard cluster field

  xOffsets.push_back(0.); yOffsets.push_back(0.);
  xOffsets.push_back(offset1inDeg*0.5); yOffsets.push_back(offset2inDeg*0.5);
  xOffsets.push_back(offset1inDeg); yOffsets.push_back(offset2inDeg);
  xOffsets.push_back(-offset1inDeg*0.5); yOffsets.push_back(-offset2inDeg*0.5);
  add3rdOffset(xOffsets, yOffsets, 0, 1, true);
  add3rdOffset(xOffsets, yOffsets, 1, 2, true);
  add3rdOffset(xOffsets, yOffsets, 0, 1, false);
  add3rdOffset(xOffsets, yOffsets, 1, 2, false);
  add3rdOffset(xOffsets, yOffsets, 3, 0, false);
  
   

  ASKAPDEBUGASSERT(xOffsets.size() == yOffsets.size());
  for (size_t i=0; i<xOffsets.size(); ++i) {
      offset1 = xOffsets[i] / 180. * casa::C::pi;
      offset2 = yOffsets[i] / 180. * casa::C::pi;
      testDir = backupTestDir;
      testDir.shift(offset1*factor,offset2*factor, casa::True);
      std::cout<<"offset ("<<xOffsets[i]<<","<<yOffsets[i]<<") applied to test direction: "<<printDirection(testDir)<<std::endl;
      //std::cout<<360+testDir.getLong()/casa::C::pi*180.<<" "<<testDir.getLat()/casa::C::pi*180.<<std::endl;
  }

}

int main(int argc, char **argv) {
  try {
     if (argc!=1) {
         cerr<<"Usage "<<argv[0]<<" - no arguments"<<endl;
         //cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     casa::Timer timer;

     timer.mark();
     //TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     makeRaster();
     doTest();
     //doReadOnlyTest(ds);
     std::cerr<<"Job: "<<timer.real()<<std::endl;
     
  }
  catch(const AskapError &ce) {
     cerr<<"AskapError has been caught. "<<ce.what()<<endl;
     return -1;
  }
  catch(const std::exception &ex) {
     cerr<<"std::exception has been caught. "<<ex.what()<<endl;
     return -1;
  }
  catch(...) {
     cerr<<"An unexpected exception has been caught"<<endl;
     return -1;
  }
  return 0;
}
