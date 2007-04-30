#include <measurementequation/MEComponentEquation.h>
#include <measurementequation/MELinearSolver.h>
#include <dataaccess/DataAccessorStub.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {
	
class MEComponentEquationTest : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(MEComponentEquationTest);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_EXCEPTION(testParameters, std::invalid_argument);
    CPPUNIT_TEST(testPredict);
    CPPUNIT_TEST(testDesignMatrix);
    CPPUNIT_TEST(testAssembly);
    CPPUNIT_TEST(testSVD);
    CPPUNIT_TEST(testConstructNormalEquations);
//	CPPUNIT_TEST(testSolveNormalEquations);
	CPPUNIT_TEST_EXCEPTION(testNoFree, std::domain_error);
    CPPUNIT_TEST_SUITE_END();
	
  private:
    MEComponentEquation *p1, *p2, *p3, *pempty;
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
      
	  params1 = new MEParams;
	  params1->add("flux.i.cena", 100.0);
	  params1->add("direction.ra.cena", 0.5);
	  params1->add("direction.dec.cena", -0.3);

      p1 = new MEComponentEquation(*params1);

	  params2 = new MEParams;
	  params2->add("flux.i.cena", 100.0);
	  params2->add("direction.ra.cena", 0.500005);
	  params2->add("direction.dec.cena", -0.300003);
	  	  
      p2 = new MEComponentEquation(*params2);
      
      p3 = new MEComponentEquation();
      pempty = new MEComponentEquation();
    }
    
    void tearDown() 
    {
      delete ida;
      delete p1;
      delete p2;
      delete p3;
      delete pempty;
    }
    
    void testCopy() 
    {
		MEParams ip;
		ip.add("Value0");
		ip.add("Value1");
		ip.add("Value2");
		delete p1;
		p1 = new MEComponentEquation(ip);
		delete p2;
		p2 = new MEComponentEquation(*p1);
		CPPUNIT_ASSERT(p2->parameters().names().size()==3);
		CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
		CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
		CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
    }
    
	void testPredict()
	{
		p1->predict(*ida);
	}
	
	void testDesignMatrix()
	{
		MEDesignMatrix dm1(*params1);
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(abs(dm1.fit()-100.0)<0.01);
		p1->predict(*ida);
		dm1.reset();
		p1->calcEquations(*ida, dm1);
		CPPUNIT_ASSERT(dm1.fit()<0.03);
		MEDesignMatrix dm2(*params2);
		p2->calcEquations(*ida, dm2);
		CPPUNIT_ASSERT(abs(dm2.fit()-6.61609)<0.0001);
	}
	
	void testAssembly() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
	}

	void testSVD() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==3);
		CPPUNIT_ASSERT(abs(q.cond()-1.8419e+06)<100.0);
  	    solver1.parameters().fix("flux.i.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==2);
		CPPUNIT_ASSERT(abs(q.cond()-2.58063)<0.0001);
	    solver1.parameters().fix("direction.ra.cena");
		solver1.solveDesignMatrix(q);
		CPPUNIT_ASSERT(q.rank()==1);
		CPPUNIT_ASSERT(abs(q.cond()-1.000000)<0.0001);
	}

	void testConstructNormalEquations() {
		MEDesignMatrix dm1(*params1);
		p2->calcEquations(*ida, dm1);
		MENormalEquations normeq(dm1, MENormalEquations::COMPLETE);
		std::map<string, std::map<string, casa::Matrix<double> > > nm(normeq.normalMatrix());
		vector<string> names(params1->names());
		for (uint row=0;row<names.size();row++) {
			for (uint col=0;col<names.size();col++) {
				casa::IPosition ip(nm[names[row]][names[col]].shape());
				CPPUNIT_ASSERT(ip(0)==1);
				CPPUNIT_ASSERT(ip(1)==1);
			}
		}
	}

	void testSolveNormalEquations() {
		// Predict with the "perfect" parameters"
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		// Calculate gradients using "imperfect" parameters" 
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
		MENormalEquations normeq(dm1, MENormalEquations::COMPLETE);
		solver1.addNormalEquations(normeq);
		solver1.solveNormalEquations(q);
	}

	void testNoFree() {
		MEDesignMatrix dm1(*params1);
		p1->predict(*ida);
		p2->calcEquations(*ida, dm1);
		MEQuality q;
		MELinearSolver solver1(*params2);
		solver1.addDesignMatrix(dm1);
  	    solver1.parameters().fix("flux.i.cena");
	    solver1.parameters().fix("direction.ra.cena");
	    solver1.parameters().fix("direction.dec.cena");
		// Should throw exception: domain_error
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
