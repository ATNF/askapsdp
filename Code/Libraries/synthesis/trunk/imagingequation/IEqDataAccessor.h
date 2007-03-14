#ifndef IEQDATAACCESSOR_H_
#define IEQDATAACCESSOR_H_

namespace conrad
{

class IEqDataAccessor
{
public:
	IEqDataAccessor();
	virtual ~IEqDataAccessor();
	virtual void initmodel();
};

}

#endif /*IEQDATAACCESSOR_H_*/
