#include <iostream>
#include <sourcefitting/RadioSource.h>
int main()
{
    
    for(int i=0;i<100;i++){
	std::cout << "source " << i << " has suffix '"<<askap::analysis::sourcefitting::getSuffix(i) << "'\n";
    }

    std::cout << "----------\n";

    for(int i=700;i<710;i++){
	std::cout << "source " << i << " has suffix '"<<askap::analysis::sourcefitting::getSuffix(i) << "'\n";
    }
}
