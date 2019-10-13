#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run();

    if(context.shouldExit()) {
        return res;
    }
}
