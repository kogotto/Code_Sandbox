/**
   Unit test showing how to use and implement the Geographic Utilities
 */

#include <iostream>
#include <string>

#include <Logger.hpp>

#include "coordinate/TEST_coordinate.hpp"
#include "dem/TEST_dem.hpp"
#include "image/TEST_image.hpp"


using namespace std;

/** Main Driver 
 *
 * @brief Entrance to the Unit Test
 */
int main(int argc, char* argv[]) {

    try {
        // print header
        print_module_header("GeoImage Unit Test Framework");
        
        // run the coordinate module
        TEST_coordinate_module();
        
        // run the image module
        TEST_image_module();

        // run the dem module
        TEST_dem_module();

        // print footer
        print_module_footer("GeoImage Unit Test Framework");
    
    
    }    catch (string e) {
        cout << e << endl;
    }
    
    //end of unit tests
    return 0;
}

