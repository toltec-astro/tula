#include <tula_boilerplate/boilerplate.h>

#include <iostream>

int main()
{
    std::cout << "tula_downstream -> tula_boilerplate "
              << tula_boilerplate::version << '\n';
    std::cout << "packaged logging provider: "
              << tula_boilerplate::logging_provider << '\n';
    return 0;
}
