#include <gridding/TableVisGridder.h>
#include <measurementequation/MEParams.h>
#include <measurementequation/MEComponentEquation.h>
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
	
class TableVisGridderTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TableVisGridderTest);
    CPPUNIT_TEST(testForward);
    CPPUNIT_TEST(testReverse);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    TableVisGridder *p1, *p2, *p3, *pempty;
    DataAccessorStub *ida;
	casa::Vector<double>* cellSize;
	casa::Cube<casa::Complex>* grid;
	casa::Vector<float>* weights;
		

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
      uint nChan(16);
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
      
	  MEParams ip;
	  ip.add("flux.i.cena", 100.0);
	  ip.add("direction.ra.cena", 0.5);
	  ip.add("direction.dec.cena", -0.3);
	  
	  MEComponentEquation ce(ip);
	  ce.predict(*ida);

      p1 = new TableVisGridder();

      p2 = new TableVisGridder();
      
      p3 = new TableVisGridder();
      
      pempty = new TableVisGridder();

      cellSize=new casa::Vector<double>(2);

      (*cellSize)(0)=10.0*casa::C::arcsec;
      (*cellSize)(1)=10.0*casa::C::arcsec;
      
      grid=new casa::Cube<casa::Complex>(512,512,1);
      grid->set(0.0);
      
      weights=new casa::Vector<float>(1);
      weights->set(0.0);
      
    }
    
    void tearDown() 
    {
      delete ida;
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
      delete cellSize;
      delete grid;
      delete weights;
    }

	void testForward()
	{
		p1->forward(*ida, *cellSize, *grid, *weights);
	}    
	void testReverse()
	{
		p1->reverse(*ida, *grid, *cellSize);
	}    
  };
  
}
}
