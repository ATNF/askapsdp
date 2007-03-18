/// @file
///
/// IEqDataAccessor: Access to buffered visibility data
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef IEQDATAACCESSOR_H_
#define IEQDATAACCESSOR_H_

namespace conrad
{

class IEqDataAccessor
{
public:
	/// Construct
	IEqDataAccessor();
	
	/// Destruct
	virtual ~IEqDataAccessor();
	
	/// Initialize model column
	virtual void initmodel();
};

}

#endif /*IEQDATAACCESSOR_H_*/
