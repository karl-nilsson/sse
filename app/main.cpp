#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

// #include <sse/version.hpp>
#include <sse/Importer.hpp>
#include <sse/Object.hpp>

namespace fs = std::filesystem;
using namespace std;

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    //  cout << "StepSlicerEngine " + VERSION + " build: " + GIT_SHA<< endl;

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << "file1.step" << endl;
        return 1;
    }

    auto imp = Importer{};

    auto objects = vector<TopoDS_Shape>();
    //vector<sse::Object>();

    for (int i = 1; i < argc; i++) {
        cout << "Loading file: " << argv[i] << endl;

        // check if file exists
        if (!fs::exists(argv[i])) {
            cerr << "file " << argv[i] << " does not exist" << endl;
            continue;
        }
        // import the object, then add it to the list
        auto v = imp.importSTEP(argv[i]);
        if(v == std::nullopt){
            cerr << "Error processing file " << argv[i] << endl;
        } else {
            objects.push_back(v.value());
        }
    }


    // slice the body into layers
    //auto a = splitter(objects, makeTools(1, 10));

    return 0;
}

