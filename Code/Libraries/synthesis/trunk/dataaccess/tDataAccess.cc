#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <iostream.h>

#include <METableDataAccessor.h>

using namespace conrad;
 
// Someone needs these templates - I don't know who!
casa::Matrix<casa::String> c0;

int main() {
	
    try {
		
		
    } catch (casa::AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

