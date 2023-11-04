#include <iostream>

#ifdef _WIN32
#define SCRIPTAPI extern "C" __declspec((dllexport))
#else
#define SCRIPTAPI extern "C"
#endif

SCRIPTAPI void HelloCpp()
{
    std::cout << "Hello, C#!\n";
}