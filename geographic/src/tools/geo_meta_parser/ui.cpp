#include "ui.h"
#include "geo_forms.h"

#include <string>

#include <ncurses.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <GeoImage.h>

using namespace cv;
using namespace std;

const int program_header_row = 0;
const int menu_header_row    = 2;
const int meta_header_row    = 3;

const int footer_height      = 4;

const int MAIN_MENU_TIMEOUT  = 100;


vector<geo_header_item> pull_header_metadata( GEO::GeoImage const& img );

void init_gui( Options const& configuration ){

    //take the console
    initscr();

    //disable line buffering
    raw();
    
    //enable the keypad
    keypad( stdscr, true);
    
    //disable echo
    noecho();

    //enable color console
    start_color();

}

void stop_gui( Options const& configuration ){

    //return the console back to bash
    endwin();

}

void resolve_window_range( int& top_win, int& bot_win, int const& cur_pos , const int& size ){

    //if the cursor is within the top and bottom range, then simply exit
    if( cur_pos >= top_win && cur_pos <= bot_win )
        return;

    //compute the difference between the cursor and each marker
    int distTopCurs = cur_pos - top_win;
    int distBotCurs = cur_pos - bot_win;
    
    int diff;
    if( fabs(distTopCurs) < fabs(distBotCurs) )
        diff = distTopCurs;
    else
        diff = distBotCurs;


    //apply results
    top_win += diff;
    bot_win += diff;

}

void main_menu( Options& configuration ){
    
    int con_size_x, con_size_y;
    int input = 0;
    int skey = 0;
    bool exit_program = false;
    int window_height;
    bool lock_screen_modify = false;

    //initialize GeoImage object
    configuration.current_image.set_init( true );
    if( configuration.current_image.get_init() == false ){
        throw string(string("Error: image ")+configuration.filename+string(" did not load properly"));
    }

    /* Pull Header Metadata */
    vector<geo_header_item>  header_metadata = pull_header_metadata( configuration.current_image );
    
    //set timeout function
    timeout( MAIN_MENU_TIMEOUT );
    fflush(stdout);

    while( exit_program == false ){

        //retrieve window size
        get_console_size( con_size_x, con_size_y);

        //fix the window_top and cursor position variables
        window_height = (con_size_y-footer_height) - meta_header_row;

        //we already have the window top, now lets find the window bottom
        int win_bot = configuration.window_top + window_height - 1;
        resolve_window_range( configuration.window_top, win_bot, configuration.cursor_pos, header_metadata.size() );


        //clear console screen
        clear( );
        attron(COLOR_PAIR(backg_window_pair));

        //print program header
        print_header( "Geographic Meta Parser",  program_header_row, con_size_x, con_size_y); 

        //print menu header
        print_header( "Main Menu", menu_header_row, con_size_x, con_size_y);
        wnoutrefresh(stdscr);

        //print information
        print_metadata( header_metadata, meta_header_row, con_size_x, con_size_y-footer_height, configuration.cursor_pos, configuration.window_top );
        wnoutrefresh(stdscr);

        //print footer
        print_footer( con_size_x, con_size_y );

        //draw screen
        wnoutrefresh(stdscr);
        doupdate();

        //wait on user input and take action
        input  = getch();
        
        switch (input){

            case 'c':
            case 'C':

                //we should lock the screen and allow for modification of the field
                lock_screen_modify = true;

                break;

            case 27: // ALT OR ESCAPE

                skey = getch();
                if( skey != ERR){  // You have alt key

                }
                else{ // You have escape key
                    exit_program = true;
                }
                break;

            case 'q':
            case 'Q':
                exit_program = true;
                break;

                /**
                  KEY VALUES
                 */
            case 10:

                if( lock_screen_modify == true )
                    lock_screen_modify = false;
                break;

            case KEY_UP:

                //make sure that the screen isn't locked for various parameters
                if( lock_screen_modify == false ){

                    //decrement cursor position
                    configuration.cursor_pos--;
                    if( configuration.cursor_pos < 0 )
                        configuration.cursor_pos = ((int)header_metadata.size()) + configuration.cursor_pos;
                }
                break;

            case KEY_DOWN:

                //make sure that the screen isn't locked for various parameters
                if( lock_screen_modify == false ){

                    //decrement cursor position
                    configuration.cursor_pos++;
                    if( configuration.cursor_pos >= header_metadata.size())
                        configuration.cursor_pos = ((int)header_metadata.size()) - configuration.cursor_pos;
                }
                break;

            default:;

        }
        fflush(stdout);

    }

}

/**
 *  Process command-line arguments into tangible flags and options.
 * 
 * @param[in] args Command-Line arguments
 * @param[in/out] configuration Config flag container
 */
void process_arguments( std::vector<pair<std::string,std::string> >const& args, Options& configuration ){

    //iterate through arguments evaluating for flags
    for( size_t i=1; i<args.size(); i++){

        //Process File
        if( args[i].first.substr(0, 6) == "-file=" ){
            configuration.filename = args[i].second.substr(6);
            configuration.file_set = true;
            configuration.current_image.set_filename(configuration.filename);

        }

        //Process Debug Level
        else if( args[i].first.substr(0, 5) == "-dbg=" )
            configuration.debug_level = str2int(args[i].second.substr(5));

        //Process Help Flags
        else if( args[i].first.substr(0,5) == "-help" ){
            usage();
        }

        //throw exception
        else
            throw string(string("Error: unknown flag -> ") + args[i].second );
    }

    if( configuration.file_set == false )
        throw string("Error: no filename set on image");

}

void usage(){
    cout << " usage:  " << endl;
    throw string("");
}


vector<geo_header_item>  pull_header_metadata( GEO::GeoImage const& img ){


    //pull the GeoHeader
    vector<pair<string,string> >  hdr_dat = img.get_header()->get_header_data(); 

    vector<geo_header_item>  output;

    for( size_t i=0; i<hdr_dat.size(); i++ ){
        output.push_back( geo_header_item( 1, hdr_dat[i].first, hdr_dat[i].second));
    }

    return output;
}

