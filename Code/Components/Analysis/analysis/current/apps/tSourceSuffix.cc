#include <iostream>
#include <outputs/CataloguePreparation.h>
int main()
{

    for (int i = 0; i < 100; i++) {
        std::cout << "source " << i << " has suffix '" << askap::analysis::getSuffix(i) << "'\n";
    }

    std::cout << "----------\n";

    for (int i = 700; i < 710; i++) {
        std::cout << "source " << i << " has suffix '" << askap::analysis::getSuffix(i) << "'\n";
    }
}
