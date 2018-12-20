#include <stdexcept>
#include <iostream>
#include <string>
#include <PdBase.hpp>

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
          throw std::runtime_error("Invalid arguments. Provide the patch path and filename");
        }
        pd::PdBase pdEngine;
        if(!pdEngine.init(0, 2, 44.1))
        {
            throw std::runtime_error("Could not initialize PD engine.");
        }
        pd::Patch handle = pdEngine.openPatch(argv[2], argv[1]);
        if (!handle.isValid())
        {
            throw std::runtime_error(std::string("Unable to open patch file: ") + argv[2]);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught. Details: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught." << std::endl;
        return 1;
    }
    return 0;
}
