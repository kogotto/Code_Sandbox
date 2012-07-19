#ifndef __SRC_TOOLS_GEO_META_PARSER_UTILS_H__
#define __SRC_TOOLS_GEO_META_PARSER_UTILS_H__

#include <iostream>
#include <string>

#include <GeoImage.h>

class geoTuple{
    public:
        std::string id;
        std::string val;
        std::string type;
};

class Options{
    public:
        
        Options();

        std::string filename;
        bool file_set;

        int debug_level;

        void print( );
        
        GEO::GeoImage current_image;
};


std::string int2str( int const& val );
int         str2int( std::string const& val );

void get_console_size( int& x, int& y );

#endif