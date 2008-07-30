/// @file
/// 
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <casa/aips.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Complex.h>
#include <calweightsolver/ImplCalWeightSolver.h>
#include <calweightsolver/EigenSolve.h>
#include <fstream>

using namespace casa;
using namespace std;

int main() {
try {
   /*
   //
   ifstream is("mtr.dat");
   uInt x,y;
   is>>x>>y;
   if (!is) throw AipsError("Unable to read mtr.dat");
   Matrix<Complex> mtr(x,y);
   for (uInt row=0;row<x;++row)
        for (uInt col=0;col<y;++col) {
	     double buf;
	     is>>buf;
	     if (!is) throw AipsError("Unable to read mtr.dat");
	     mtr(row,col)=Complex(buf,0.);
	}
   cout<<"Matrix is loaded successfully"<<endl;
   EigenSolver es;
   es.solveEigen(mtr);

   Matrix<Complex> tmpbuf(mtr.nrow(),mtr.ncolumn());
   double pkval;
   for (uInt row=0;row<tmpbuf.nrow();++row) 
	for (uInt col=0;col<tmpbuf.ncolumn();++col) {
	     tmpbuf(row,col)=-mtr(row,col);
	     for (uInt i=0;i<es.getEigenVectors().ncolumn();++i)
		  tmpbuf(row,col)+=es.getEigenVectors()(row,i)*
		       es.getEigenValues()[i]*es.getV()(col,i);
             if ((!row && !col) || pkval<abs(tmpbuf(row,col)))
	          pkval=abs(tmpbuf(row,col));             
	}
   cout<<"Peak ampl. is "<<pkval<<endl;
   return 0;
   */
   // old main.cc
   ImplCalWeightSolver solver;
   MDirection pc(Quantity(0.,"deg"),Quantity(-50.,"deg"),
                 MDirection::J2000);
   solver.setSky(pc,"mysrc.cl");
   //solver.setSky(pc,"singlesrc.cl");
   solver.setVP("xntd.element.vbeam.real","xntd.element.vbeam.imag");
   Matrix<Double> feed_offsets(5,2,0.);
   feed_offsets(1,0)=-M_PI/180./3.; feed_offsets(1,1)=-M_PI/180./3.;
   feed_offsets(2,0)=M_PI/180./3.; feed_offsets(2,1)=-M_PI/180./3.;
   feed_offsets(3,0)=M_PI/180./3.; feed_offsets(3,1)=M_PI/180./3.;
   feed_offsets(4,0)=-M_PI/180./3.; feed_offsets(4,1)=M_PI/180./3.;

   /*
   Vector<Double> uvw(3,0.);
   uvw[0]=1e3; uvw[1]=0.; uvw[2]=0.;
   Matrix<Complex> res=solver.solveWeights(feed_offsets,uvw);
   */
   Matrix<Complex> res=solver.eigenWeights(feed_offsets);

   for (uInt j=0;j<res.ncolumn();++j) {
        cout<<"Weight configuration number "<<j+1<<endl;
        for (uInt f=0;f<res.nrow();++f)
             cout<<res(f,j)<<endl;
   }
}
catch (const AipsError &ae) {
   cerr<<ae.what()<<endl;
   return -1;
}
catch (...) {
   cerr<<"An unexpected exception has been caught"<<endl;
   return -2;
}

return 0;
}
