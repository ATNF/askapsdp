/// @file
/// 
/// @brief Singular value decomposition algorithm
/// @details To provide a replacement of GSL's version of 
/// the singular value decomposition, I quickly adapted this file
/// (which I wrote long time ago for another project). The algorithm
/// originates initially from the Numerical Recipies in C.
/// The code doesn't conform with the CONRAD coding standards, because
/// it wasn't written originally for CONRAD. If necessary, it will be 
/// fully converted later.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef __SVDECOMPOSE_H
#define __SVDECOMPOSE_H

#include <cmath>
#include <vector>
#include <string>
#include <iostream>

namespace svd {

  using namespace std;

  double inline sqr(double x) {return x==0? 0 :x*x;}

  double inline sign(double a,double b)
  {
    return b>=0?fabs(a):-fabs(a);
  }

  int inline imin(int a,int b) { return a>b?b:a;}

  double inline fmax(double a,double b) { return a>b?a:b;}

  // supplementary function to calculate hypothenuse using the
  // Pythagor formula sqrt(a^2+b^2) without destructive underflow or
  // overflow.
  double inline pythag(double a,double b) {
     double absa=fabs(a),absb=fabs(b);
     if (absa>absb) return absa*sqrt(1.+sqr(absb/absa));
     else return (absb==0.?0.:absb*sqrt(1.+sqr(absa/absb)));
  }

  // adapter to be used with any one dimensional container, which
  // supports
  //           1) typedef value_type exists 
  //           2) operator [index]  (index (0..size-1)) returning value_type&
  //           3) size_t size() const; returning a number of elements
  //           4) void resize(size_t newsize); resize the array
  template<class Cont> class Matrix2D {
      size_t m,n; // number of rows and columns;
      Cont &cont; // reference to the container
    public:
        Matrix2D(Cont &icont,size_t im=0,size_t in=0) : 
	          m(im),n(in), cont(icont) {cont.resize(m*n);};
	void resize(size_t newnrow, size_t newncol) {
	     m=newnrow;
	     n=newncol;
	     if (cont.size()!=m*n)
	         cont.resize(m*n);
	}
	
	typename Cont::value_type& operator()(size_t row, size_t col) {
	     if (row>=m || col>=n) {
	          std::cout<<m<<" "<<n<<" "<<row<<" "<<col<<std::endl;
	          throw string("Out of range in non-const method");
	     }
	     return cont[row*n+col];
	}

	const typename Cont::value_type& operator()(size_t row,
	                                           size_t col) const {
	     if (row>=m || col>=n) throw string("Out of range in const method");					   
	     return cont[row*n+col];
	}
	size_t nrow() const { return m;}
	size_t ncol() const { return n;}
  };

  // given a matrix A[0..m-1][0..n-1] this routine computes its singular
  // value decomposition, A=UWV^T. The matrix U replaces A on output,
  // the diagonal matrix of singular values W is output as a vector
  // W[0..n-1]. The matrix V (not the transpose V^T) is output as
  // V[0..n-1][0..n-1]. See Numerical Recipies in C for detail.
  // Note for C++ implementation:
  //      W is a class of any type, which has the following
  //        1) operator [index]   (index (0..size-1)), returning double&
  //        2) size_t size() const; returning a number of elements
  //        3) void resize(size_t newsize);  resize the array (doesn't
  //           matter whether the elements are preserved or not).
  //      V and M are classes of any type, which have the following
  //        1) operator (row,col) (row (0..nrow-1), col (0..ncol-1)),
  //           returning double&
  //        2) size_t nrow() const; returning a number of rows
  //        3) size_t ncol() const; returning a number of columns
  //        4) void resize(size_t newnrow, size_t newncol);
  //           resize the matrix (doesn't
  //           matter whether the elements are preserved or not).
  // The function may throw an exception (string) if number of
  // internal iteration has exceeded the limit
  template<class Array, class Matrix>
  void computeSVD(Matrix &A, Array &W, Matrix &V) {
       // setup the sizes of the output arrays
       W.resize(A.ncol());
       V.resize(A.ncol(),A.ncol());
  
       // Householder reduction to bidiagonal form
       double g=0., anorm=0., scale=0.;
       vector<double> rv1(A.ncol());
       int l;
       for (size_t i=0;i<A.ncol();++i) {
            l=i+1;
	    rv1[i]=scale*g;
	    double s=0.;
	    g=scale=0.;
	    if (i<A.nrow()) {
	        for (size_t k=i;k<A.nrow();++k) scale+=fabs(A(k,i));
		if (scale) {
		    for (size_t k=i;k<A.nrow();++k) {
		         A(k,i)/=scale;
			 s+=sqr(A(k,i));
		    }
		    double f=A(i,i);
		    g=-sign(sqrt(s),f);
		    double h=f*g-s;
		    A(i,i)=f-g;
		    for (size_t j=l;j<A.ncol();++j) {
		         s=0.;
		         for (size_t k=i;k<A.nrow();++k) s+=A(k,i)*A(k,j);
			 f=s/h;
			 for (size_t k=i;k<A.nrow();++k) A(k,j)+=f*A(k,i);
		    }
		    for (size_t k=i;k<A.nrow();++k) A(k,i)*=scale;
		}
	    }
	    W[i]=scale*g;
	    g=s=scale=0.;
	    if (i<A.nrow() && i!=(A.ncol()-1)) {
	        for (size_t k=l;k<A.ncol();++k) scale+=fabs(A(i,k));
		if (scale) {
		    for (size_t k=l;k<A.ncol();++k) {
		         A(i,k)/=scale;
			 s+=sqr(A(i,k));
		    }
		    double f=A(i,l);
		    g=-sign(sqrt(s),f);
		    double h=f*g-s;
		    A(i,l)=f-g;
		    for (size_t k=l;k<A.ncol();++k) rv1[k]=A(i,k)/h;
		    for (size_t j=l;j<A.nrow();++j) {
		         s=0.;
		         for (size_t k=l;k<A.ncol();++k) s+=A(j,k)*A(i,k);
			 for (size_t k=l;k<A.ncol();++k) A(j,k)+=s*rv1[k];
		    }
		    for (size_t k=l;k<A.ncol();++k) A(i,k)*=scale;
		}
	    }
	    anorm=fmax(anorm,(fabs(W[i])+fabs(rv1[i])));
       }       
       // Accumulation of right-hand transformations.
       for (int i=int(A.ncol())-1;i>=0;--i) {
            if (i<int(A.ncol())-1) {
	        if (g) {
		        for (size_t j=l;j<A.ncol();++j)  // Double division to avoid overflow
		             V(j,i)=(A(i,j)/A(i,l))/g;
	            for (size_t j=l;j<A.ncol();++j) {
		             double s=0.;
		             for (size_t k=l;k<A.ncol();++k) s+=A(i,k)*V(k,j);
			         for (size_t k=l;k<A.ncol();++k) V(k,j)+=s*V(k,i);
		        }
		    }
		    for (size_t j=l;j<A.ncol();++j) V(i,j)=V(j,i)=0.;
	   }
	    V(i,i)=1.;
	    g=rv1[i];
	    l=i;
       }
       
       // Accumulation of left-hand transformations
       for (int i=imin(int(A.nrow()),int(A.ncol()))-1;i>=0;--i) {
            l=i+1;
	    g=W[i];
	    for (size_t j=l;j<A.ncol();++j) A(i,j)=0.;
	    if (g) {
	        g=1./g;
		for (size_t j=l;j<A.ncol();++j) {
		     double s=0.;
		     for (size_t k=l;k<A.nrow();++k) s+=A(k,i)*A(k,j);
		     double f=(s/A(i,i))*g;
		     for (size_t k=i;k<A.nrow();++k) A(k,j)+=f*A(k,i);
		}
		for (size_t j=i;j<A.nrow();++j) A(j,i)*=g;
	    } else for (size_t j=i;j<A.nrow();++j) A(j,i)=0.;
	    ++A(i,i);
       }
       
       // Diagonalization of the bidiagonal form: Loop over
       // singular values, and over allowed iterations
       for (int k=int(A.ncol())-1;k>=0;--k) {
            for (size_t its=0;its<30;++its) {
	         bool flag=true;
		 int nm;		 
		 for (l=k;l>=0;--l) {  
		      // Test for splitting. Note that rv1[0] is always zero
		      nm=l-1;
		      if ((double)(fabs(rv1[l])+anorm)==anorm || !l) {
		          flag=false;
			  break;
		      }
		      if ((double)(fabs(W[nm])+anorm)==anorm) break;
		 }  
		 if (flag) { 
		     double c=0.; // cancellation of rv1[l], if l>0
		     double s=1.;
		     for (int i=l;i<=k;++i) {
		          double f=s*rv1[i];
			  rv1[i]*=c;
			  if ((double)(fabs(f)+anorm)==anorm) break;
			  g=W[i];
			  double h=pythag(f,g);
			  W[i]=h;
			  h=1./h;
			  c=g*h;
			  s=-f*h;
			  for (size_t j=0;j<A.nrow();++j) {
			       double y=A(j,nm);
			       double z=A(j,i);
			       A(j,nm)=y*c+z*s;
			       A(j,i)=z*c-y*s;
			  }
		    }
		 }
		 double z=W[k];
		 if (l==k) {  // Convergence
		     if (z<0.) { // Singular value is made nonnegative
		         W[k]=-z;
			 for (size_t j=0;j<A.ncol();++j)
			      V(j,k)=-V(j,k);		         
		     } 
		     break;
		 }
		 if (its==30) throw string("no convergence in 30 svdcmp iterations");
		 double x=W[l]; 
		 nm=k-1;  
		 double y=W[nm];
		 g=rv1[nm];
		 double h=rv1[k];
		 double f=((y-z)*(y+z)+(g-h)*(g+h))/(2.*h*y);
		 g=pythag(f,1.);
		 f=((x-z)*(x+z)+h*((y/(f+sign(g,f)))-h))/x;		 
		 // next QR transformation
		 double c=1.;
		 double s=1.;		 
		 for (size_t j=l;j<=size_t(nm);++j) {
		      size_t i=j+1;
		      g=rv1[i];
		      y=W[i];
		      h=s*g;
		      g=c*g;
		      z=pythag(f,h);
		      rv1[j]=z;
		      c=f/z;
		      s=h/z;
		      f=x*c+g*s;
		      g=g*c-x*s;
		      h=y*s;
		      y*=c;
		      for (size_t jj=0;jj<A.ncol();++jj) {
		           x=V(jj,j);
			   z=V(jj,i);
			   V(jj,j)=x*c+z*s;
			   V(jj,i)=z*c-x*s;			   
		      }
		      z=pythag(f,h);
		      W[j]=z; // rotation can be arbitrary if z=0
		      if (z) {
		         z=1.0/z;
			 c=f*z;
			 s=h*z;
		      }
		      f=c*g+s*y;
		      x=c*y-s*g;
		      for (size_t jj=0;jj<A.nrow();++jj) {
                           y=A(jj,j);
			   z=A(jj,i);
			   A(jj,j)=y*c+z*s;
			   A(jj,i)=z*c-y*s;		      
		      }		      
		 }
		 rv1[l]=0.;
		 rv1[k]=f;
		 W[k]=x;
	    }
       }
  }
}
#endif // #ifndef __SVDECOMPOSE_H
