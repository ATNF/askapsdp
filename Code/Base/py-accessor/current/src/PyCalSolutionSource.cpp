/// Python Wrapper for CalibAccessFactory
///
/// @copyright (c) 2013 CSIRO
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
/// @author Keith Bannister <keith.bannister@csiro.au>
///
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <casa/aipstype.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <Common/ParameterSet.h>

// own includes
#include <calibaccess/ICalSolutionConstAccessor.h>
#include <calibaccess/TableCalSolutionConstSource.h>
#include <calibaccess/CalibAccessFactory.h>
#include <calibaccess/JonesIndex.h>
#include <calibaccess/JonesJTerm.h>
#include <calibaccess/JonesDTerm.h>

using namespace std;
using namespace boost::python;
using namespace askap::accessors;

class CalSolutionAccessorWrap 
{
private:
  ICalSolutionConstAccessor::ShPtr itsAccessor;
  
public:
  CalSolutionAccessorWrap(ICalSolutionConstAccessor::ShPtr ptr) {
    itsAccessor = ptr;
  }

  casa::Vector<casa::Complex> jones(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) const {
    casa::Vector<casa::Complex> casamat(4);
    casa::SquareMatrix<casa::Complex, 2> squaremat = itsAccessor->jones(ant, beam, chan);
    casamat[0] = squaremat(0, 0);
    casamat[1] = squaremat(0, 1);
    casamat[2] = squaremat(1, 0);
    casamat[3] = squaremat(1, 1);
    return casamat;
  }

  bool jonesValid(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) const  {
    return itsAccessor->jonesValid(ant, beam, chan);
  }

  JonesJTerm gain(const casa::uInt ant, const casa::uInt beam) const {
    return itsAccessor->gain(JonesIndex(ant, beam));
  }

  JonesDTerm leakage(const casa::uInt ant, const casa::uInt beam) const {
    return itsAccessor->leakage(JonesIndex(ant, beam));
  }

  JonesJTerm bandpass(const casa::uInt ant, const casa::uInt beam, const casa::uInt chan) {
    return itsAccessor->bandpass(JonesIndex(ant, beam), chan);
  }

  typedef boost::shared_ptr<CalSolutionAccessorWrap> ShPtr;

};

class CalSourceWrap 
{
private:
  ICalSolutionConstSource::ShPtr itsSource;

public:
  CalSourceWrap(const std::string &s) {
    LOFAR::ParameterSet parset;
    parset.adoptBuffer(s);
    itsSource = CalibAccessFactory::roCalSolutionSource(parset);
  }

  long mostRecentSolution() const {
    return itsSource->mostRecentSolution();
  }

  long solutionID(const double time) const {
    return itsSource->solutionID(time);
  }

  CalSolutionAccessorWrap roSolution(long id) {
    ICalSolutionConstAccessor::ShPtr soln = itsSource->roSolution(id);
    CalSolutionAccessorWrap wrap(soln);
    return wrap;
  }
  
};

void export_CalSolutionSource()
{

  class_<CalSourceWrap>("CalSourceWrap", init<std::string>())
    .def("mostRecentSolution", &CalSourceWrap::mostRecentSolution)
    .def("solutionID", &CalSourceWrap::solutionID)
    .def("roSolution", &CalSourceWrap::roSolution);


  class_<CalSolutionAccessorWrap>("CalSolutionAccessorWrap", no_init)
    .def("jones", &CalSolutionAccessorWrap::jones)
    .def("jonesValid", &CalSolutionAccessorWrap::jonesValid)
    .def("gain", &CalSolutionAccessorWrap::gain)
    .def("leakage", &CalSolutionAccessorWrap::leakage)
    .def("bandpass", &CalSolutionAccessorWrap::bandpass);

  class_<JonesJTerm>("JonesJTerm", no_init)
    .add_property("g1", &JonesJTerm::g1)
    .add_property("g1IsValid", &JonesJTerm::g1IsValid)
    .add_property("g2", &JonesJTerm::g2)
    .add_property("g2IsValid", &JonesJTerm::g2IsValid);

  class_<JonesDTerm>("JonesDTerm", no_init)
    .add_property("d12", &JonesDTerm::d12)
    .add_property("d12IsValid", &JonesDTerm::d12IsValid)
    .add_property("d21", &JonesDTerm::d21)
    .add_property("d21IsValid", &JonesDTerm::d21IsValid); 
}


