#ifndef MEITERATIVE_H_
#define MEITERATIVE_H_

namespace conrad
{
namespace synthesis
{
	

class MEIterative
{
public:
	MEIterative(const double gain=0.1, const int niter=100, const double tol=1e-6, const string algorithm=string(""),
		const string subalgorithm=string("")) : itsGain(gain), itsNiter(niter), itsTol(tol),
		itsAlgorithm(algorithm), itsSubAlgorithm(subalgorithm) 
		{
		};
	virtual ~MEIterative() {};
	double gain() {return itsGain;}
	void setGain(const double gain=0.1) {itsGain=gain;};
	
	int niter() {return itsNiter;}
	void setNiter(const int niter=100) {itsNiter=niter;};

	double tol() {return itsTol;};
	void setTol(const double tol=1e-6) {itsTol=tol;};
	
	string algorithm() {return itsAlgorithm;};
	void setAlgorithm(const string algorithm=string("")) {itsAlgorithm=algorithm;};
	
	string subAlgorithm() {return itsSubAlgorithm;};
	void setSubAlgorithm(const string subalgorithm=string("")) {itsSubAlgorithm=subalgorithm;};
	
private:
	string itsAlgorithm;
	string itsSubAlgorithm;
	double itsGain;
	int itsNiter;
	double itsTol;
};

}
}

#endif 