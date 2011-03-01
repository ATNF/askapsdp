
#include <dataaccess/TableDataSource.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/ParsetInterface.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>
#include <utils/EigenDecompose.h>


// casa
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>
#include <measurementequation/SynthesisParamsHelper.h>


// std
#include <stdexcept>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace synthesis;

/// @brief analyse the uvw
/// @details
/// @param[in] acc accessor 
/// @param[in] beam if positive, only this beam will be taken into account
/// @return largest residual w-term (negative value means the fit has failed)
double analyseUVW(const IConstDataAccessor& acc, int beam = -1)
{
  const casa::MVDirection tangent(SynthesisParamsHelper::convertQuantity("12h30m00.000","rad"),
                                  SynthesisParamsHelper::convertQuantity("-45.00.00.000","rad"));
  const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
  const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw = acc.rotatedUVW(tangentDir);
  
  casa::Matrix<double> normalMatr(3,3,0.);
  const casa::uInt nRow = acc.nRow();
  for (casa::uInt row = 0; row<nRow; ++row) {
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
  if (fabs(normalVector[2]) > 1e-6) {
      normalVector[0] /= normalVector[2];
      normalVector[1] /= normalVector[2];
      normalVector[0] *= -1.;
      normalVector[1] *= -1.;
      std::cout<<"Best fit plane w = u * "<<normalVector[0]<<" + v * "<<normalVector[1]<<std::endl;
      
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
      std::cout<<"Largest residual w-term  is "<<maxDeviation<<" metres"<<std::endl;
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
  const casa::MVDirection tangent(SynthesisParamsHelper::convertQuantity("12h30m00.000","rad"),
                                  SynthesisParamsHelper::convertQuantity("-45.00.00.000","rad"));
  const casa::MDirection tangentDir(tangent, casa::MDirection::J2000);
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       analyseUVW(*it);
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

int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     casa::Timer timer;

     timer.mark();
     //TableDataSource ds(argv[1],TableDataSource::REMOVE_BUFFERS |
     //                           TableDataSource::MEMORY_BUFFERS);     
     //TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS | TableDataSource::WRITE_PERMITTED);     
     TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     //timeDependentSubtableTest(argv[1],ds);
     timer.mark();
     doReadOnlyTest(ds);
     //doReadWriteTest(ds);    
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
