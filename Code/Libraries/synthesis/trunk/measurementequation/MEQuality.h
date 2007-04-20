#ifndef MEQUALITY_H_
#define MEQUALITY_H_

namespace conrad
{
namespace synthesis
{

class MEQuality
{
public:
	MEQuality();
	virtual ~MEQuality();
	void setRank(const unsigned int rank) {itsRank=rank;};
	const unsigned int rank() const {return itsRank;};
	void setCond(const double cond) {itsCond=cond;};
	const double cond() const {return itsCond;};
private:
	double itsCond;
	unsigned int itsRank;
};

}
}
#endif /*MEQUALITY_H_*/
