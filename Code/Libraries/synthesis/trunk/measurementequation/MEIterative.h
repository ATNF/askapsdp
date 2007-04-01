#ifndef MEITERATIVE_H_
#define MEITERATIVE_H_

namespace conrad
{

class MEIterative
{
public:
	MEIterative(const double gain=0.1, const int niter=100, const double tol=1e-6) : itsGain(gain),
		itsNiter(niter), itsTol(tol) {};
	virtual ~MEIterative() {};
	double gain() {return itsGain;}
	void setGain(const double gain=0.1) {itsGain=gain;};
	
	int niter() {return itsNiter;}
	void setNiter(const int niter=100) {itsNiter=niter;};

	double tol() {return itsTol;};
	void setTol(const double tol=1e-6) {itsTol=tol;};
	
private:
	double itsGain;
	int itsNiter;
	double itsTol;
};

}

#endif /*ITERATIVE_H_*/
