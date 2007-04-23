#include <measurementequation/MEImageEquation.h>
#include <measurementequation/MESVDSolver.h>
#include <dataaccess/DataAccessorStub.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {
	
class MEImageEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MEImageEquationTest);
    CPPUNIT_TEST_EXCEPTION(testParameters, std::invalid_argument);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testDesignMatrix);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST_EXCEPTION(testFixed, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MEImageEquation *p1, *p2, *p3, *pempty;
	MEParams *params1, *params2, *params3;
    DataAccessorStub *ida;

  public:
    void setUp()
    {
    	/// Antenna locations for MIRANdA 30: Long, Lat, X, Y
	    uint nAnt(30);
		double antloc[30][4]={{117.49202058,-26.605799881,100.958023,559.849243},
			{117.494256673,-26.6060206276,324.567261,584.537170},
			{117.492138828,-26.6048061779,112.782784,448.715179},
			{117.490509466,-26.6064154537,-50.153362,628.693848},
			{117.491968542,-26.6032064197,95.754150,269.800934},
			{117.490679338,-26.6041085045,-33.166206,370.688568},
			{117.493698582,-26.5967173819,268.758209,-455.922058},
			{117.494563491,-26.5982339642,355.249115,-286.310059},
			{117.494296106,-26.5999215991,328.510620,-97.567841},
			{117.496887152,-26.5996276291,587.615234,-130.444946},
			{117.495216406,-26.6020274124,420.540619,137.942749},
			{117.497394175,-26.6012778782,638.317505,54.116112},
			{117.495892042,-26.6031307413,488.104187,261.337189},
			{117.498414438,-26.6041107522,740.343750,370.939941},
			{117.500240665,-26.6055840564,922.966492,535.711792},
			{117.500137190,-26.6059345560,912.619019,574.911072},
			{117.499183013,-26.6062126479,817.201294,606.012390},
			{117.496685358,-26.6061741003,567.435791,601.701294},
			{117.496086521,-26.6064049437,507.552063,627.518433},
			{117.495766946,-26.6057995355,475.594604,559.810608},
			{117.491797669,-26.6098746485,78.666885,1015.564331},
			{117.489620509,-26.6104123521,-139.049057,1075.700195},
			{117.489067099,-26.6064599406,-194.390121,633.669189},
			{117.483440786,-26.6089367492,-757.021423,910.671265},
			{117.483634163,-26.6069021547,-737.683655,683.125671},
			{117.484600363,-26.6042107834,-641.063660,382.127258},
			{117.485728514,-26.6027311538,-528.248596,216.647995},
			{117.484634236,-26.5990365257,-637.676392,-196.552948},
			{117.488195463,-26.5986065217,-281.553711,-244.643860},
			{117.488435693,-26.5949733943,-257.530670,-650.966675}};
		
		vector<casa::MPosition> mPos;
		mPos.resize(nAnt);
		for (uint iant1=0;iant1<nAnt;iant1++) {
			mPos[iant1]=casa::MPosition(casa::MVPosition(casa::Quantity(6400000, "m"), 
											casa::Quantity(antloc[iant1][0], "deg"),
											casa::Quantity(antloc[iant1][1],"deg")),
											casa::MPosition::WGS84);
		}

      ida = new DataAccessorStub;
      uint nRows(nAnt*(nAnt-1)/2);
      uint nChan(4);
      ida->mFrequency.resize(nChan);
      for (uint chan=0;chan<nChan;chan++) ida->mFrequency(chan)=1.400e9-20e6*chan;
      uint nPol(1);
      ida->mVisibility.resize(nRows, nChan, nPol);
      ida->mVisibility.set(casa::Complex(0.0, 0.0));
      ida->mTime.resize(nRows);
      ida->mUVW.resize(nRows);
      ida->mAntenna1.resize(nRows);
      ida->mAntenna2.resize(nRows);
      ida->mFeed1.resize(nRows);
      ida->mFeed2.resize(nRows);
      uint row=0;
      for (uint iant1=0;iant1<nAnt;iant1++) {
	      for (uint iant2=iant1+1;iant2<nAnt;iant2++) {
		      ida->mAntenna1(row)=iant1;
		      ida->mAntenna2(row)=iant2;
		      ida->mFeed1(row)=0;
		      ida->mFeed2(row)=0;
		      ida->mTime(row)=0.0;
			  ida->mUVW(row)=casa::RigidVector<casa::Double, 3>(0.0, 0.0, 0.0);
			  for (uint dim=0;dim<3;dim++) ida->mUVW(row)(dim)=mPos[iant1].get("m").getValue()(dim)-mPos[iant2].get("m").getValue()(dim);
			  row++;
	      }
      }
      
	  uint npix=16;
      MEDomain imageDomain;
	  double arcsec=casa::C::pi/(3600.0*180.0);
      imageDomain.add("RA", -60.0*arcsec, +60.0*arcsec, npix); 
      imageDomain.add("DEC", -600.0*arcsec, +60.0*arcsec, npix); 

	  params1 = new MEParams;
	  casa::Vector<double> imagePixels1(npix*npix);
	  imagePixels1.set(0.0);
	  imagePixels1(npix/2+npix*npix/2)=1.0;
	  imagePixels1(10+npix*5)=0.7;
	  params1->add("image.i.cena", imagePixels1, imageDomain);

      p1 = new MEImageEquation(*params1);

	  params2 = new MEParams;
	  casa::Vector<double> imagePixels2(npix*npix);
	  imagePixels2.set(0.0);
	  imagePixels2(npix/2+npix*npix/2)=0.9;
	  imagePixels2(10+npix*5)=0.75;
	  params2->add("image.i.cena", imagePixels2, imageDomain);
	  	  
      p2 = new MEImageEquation(*params2);
      
      p3 = new MEImageEquation();
      pempty = new MEImageEquation();
      
    }
    
    void tearDown() 
    {
      delete ida;
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
    }
        
	void testPredict()
	{
		p1->predict(*ida);
	}
	
	void testDesignMatrix()
	{
		MEDesignMatrix dm1(*params1);
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(abs(dm1.fit()-0.860064)<0.01);
		p1->predict(*ida);
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(dm1.fit()<0.0001);
		MEDesignMatrix dm2(*params2);
		p2->calcEquations(*ida, dm2);
		CPPUNIT_ASSERT(abs(dm2.fit()-0.0792956)<0.0001);
	}
	
	void testSVD() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MESVDSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		solver1.solveDesignMatrix(q);
		casa::Vector<double> improved=solver1.parameters().value("image.i.cena");
		uint npix=16;
		CPPUNIT_ASSERT(abs(improved(npix/2+npix*npix/2)-1.0)<0.003);
		CPPUNIT_ASSERT(abs(improved(10+npix*5)-0.700)<0.003);
	}
	
	void testFixed() {
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MESVDSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		// Should throw exception: domain_error
	    solver1.parameters().fix("image.i.cena");
		solver1.solveDesignMatrix(q);
	}
	
	void testParameters()
	// Should throw invalid_argument because of the lack of parameters
	{
		pempty->predict(*ida);
	}
  };
  
}
}
