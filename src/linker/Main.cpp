#include "../../inc/linker/Linker.hpp"
#include "../../inc/linker/File.hpp"
#include <iomanip>
#include "../../inc/linker/Error.hpp"


int main(int argc, char const *argv[]) {

    Linker linker;

    if(argc < 2) {
        cerr << "Usage: " << argv[0] << " [[-hex/-relocatable] -o outputFile -place={section}@{address}] outputFiles" << endl;
        return 1;
    }
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        try
        {
           linker.processArgument(arg);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    try {
        linker.start();
    }
    catch(NotDefined& e){
        std::cerr << e.what() << '\n';
    }
    catch(MultipleDefinitions& e){
        std::cerr << e.what() << '\n';
    }
    catch(SectionOverlapping e){
        std::cerr << e.what() << '\n';
    }
    catch(char const* s){
        std::cerr << s << '\n';
    }

      
    return 0;
}
