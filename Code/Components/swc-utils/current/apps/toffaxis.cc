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

void doTest() {
  const casa::MVDirection tangent(convertQuantity("15h56m58.871","rad"),
                                  convertQuantity("-79.14.04.28","rad"));
  //const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
  const casa::MVDirection dir(convertQuantity("16h17m49.278","rad"),
                                  convertQuantity("-77.17.18.46","rad"));

  //casa::MVDirection testDir = tangent;
  // Virgo
  //casa::MVDirection testDir(convertQuantity("12h30m49.43","rad"),convertQuantity("12.23.29.1","rad"));
  // 1934-638
  casa::MVDirection testDir(convertQuantity("19h39m25.03","rad"),convertQuantity("-63.42.45.6","rad"));

  std::cout<<"tangent point: "<<printDirection(tangent)<<std::endl;
  std::cout<<"dir: "<<printDirection(dir)<<std::endl;
  std::cout<<"test direction: "<<printDirection(testDir)<<std::endl;
  double offset1 = 0., offset2 = 0.;

  offset1 = sin(dir.getLong() - tangent.getLong()) * cos(dir.getLat());
  offset2 = sin(dir.getLat()) * cos(tangent.getLat()) - cos(dir.getLat()) * sin(tangent.getLat())
                                                  * cos(dir.getLong() - tangent.getLong());
 

  std::cout<<"separation (dir vs. tangent): "<<dir.separation(tangent)*180./casa::C::pi<<" deg, offsets (deg): "
     <<offset1*180./casa::C::pi<<" "<<offset2*180./casa::C::pi<<std::endl;
  testDir.shift(offset1,offset2, casa::True);
  std::cout<<"offset applied to test direction: "<<printDirection(testDir)<<std::endl;
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
