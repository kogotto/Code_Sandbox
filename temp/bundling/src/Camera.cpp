#include "Camera.hpp"
#include "TACID.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;


/** 
 * Default constructor for the TimeID class. 
*/
TimeID::TimeID(){
    dirs.clear();
    pathnames.clear();
}

/**
 * Parameterized constructor for the TimeID Class. 
*/
TimeID::TimeID( const string& dirname, const string& camname ){

    // add directory to pathname list
    pathnames.push_back(dirname);
    
    //get the path elements in the form of a deque
    dirs = file_decompose_path( dirname );
        
    //look for the camname
    deque<string>::iterator pos = find( dirs.begin(), dirs.end(), camname );
        
    
    //if it is there, then delete everthing up to and including it for the dirname
    string pathStr = "";
    vector<string> pathstack;
    if( pos != dirs.end()){
        for( deque<string>::iterator it = dirs.begin(); it != pos; it++ ){
            
            //pull from the dir stack
            dirs.pop_front();
        }
        //remove the first item, as that is our starting directory
        dirs.pop_front();
    }

}


/**
 * Decompose the path string and add it to the pathnames list
*/
void TimeID::decompose_and_add_path( const string& pathStr ){

    //add to path string list
    pathnames.push_back( pathStr );
}



bool TimeID::operator <(  TimeID const& rh )const{
    
    //iterate through the host directories
    for( size_t i=0; i<dirs.size(); i++){

        //make sure the guest still has directories
        //  if not, then the host is smaller
        if( rh.dirs.size() <= i ){
            return true;
        }

        //compare directories
        if( dirs[i] < rh.dirs[i] )
            return true;
        else if( dirs[i] > rh.dirs[i] )
            return false;
        else 
            continue;

    }

    //if they are equal and the guest has more directories, then the host is smaller
    if( dirs.size() < rh.dirs.size() )
        return true;
    
    return false;
}

bool TimeID::operator >(  TimeID const& rh )const{

    //iterate through the host directories
    for( size_t i=0; i<dirs.size(); i++){

        //make sure the guest still has directories
        //  if not, the host is larger
        if( rh.dirs.size() <= i ){
            return true;
        }

        //compare directories
        if( dirs[i] > rh.dirs[i] )
            return true;
        else if( dirs[i] < rh.dirs[i] )
            return false;
        else 
            continue;

    }

    //if they are equal and the guest has more directories, then the host is smaller
    if( dirs.size() < rh.dirs.size() )
        return false;

    //otherwise they are equal
    return false;
}

bool TimeID::operator ==( TimeID const& rh )const{

    //make sure they have the same size
    if( dirs.size() != rh.dirs.size() )
        return false;
    
    //check to make sure the contents are equal
    for( size_t i=0; i<dirs.size(); i++ ){
        
        if( dirs[i] != rh.dirs[i] )
            return false;
    }

    return true;

}
bool TimeID::operator !=( TimeID const& rh )const{
    
    //make sure they have the same size
    if( dirs.size() != rh.dirs.size() )
        return true;

    //check to make sure the contents are equal
    for( size_t i=0; i<dirs.size(); i++ ){
        
        if( dirs[i] != rh.dirs[i] )
            return true;
    }

    return false;

}

ostream& operator << ( ostream& ostr, const TimeID& id ){
    ostr << "DIRS: ";
    for( size_t i=0; i<id.dirs.size(); i++) 
        cout << id.dirs[i] << " / ";
    ostr << endl;
    ostr << "PATHS: " << endl;
    for( size_t i=0; i<id.pathnames.size(); i++ )
        ostr << "   " << id.pathnames[i] << endl;

    return ostr;
}

SceneID::SceneID( ): m_major("0"), m_minor(0){}

SceneID::SceneID( string const& filename ){

    m_major = "0";
    
    //pull out scene number
    bool isValid;
    m_minor = TACID::scene_number( filename, isValid );

}


bool SceneID::operator < ( SceneID const& rh )const{
    if( m_major == rh.m_major ){
        if( m_minor < rh.m_minor ) return true;
    }
    else if( m_major < rh.m_major )
        return true;
        
    return false;
}

bool SceneID::operator > ( SceneID const& rh )const{
    if( m_major == rh.m_major ){
        if( m_minor > rh.m_minor ) 
            return true;
    }
    else if( m_major > rh.m_major )
        return true;
        
    return false;
}

bool SceneID::operator == ( SceneID const& rh )const{
    if( m_major == rh.m_major )
        if( m_minor == rh.m_minor ) return true;
    return false;
}

bool SceneID::operator != ( SceneID const& rh )const{

    if( m_major == rh.m_major )
        if( m_minor == rh.m_minor ) return false;
    return true;

}

void Camera::add_directory( string const& dir_name ){

    root_directories.push_back(dir_name);

}

/**
 * Takes the base pathnames provided to the camera object and builds a tree
 * structure consisting of the internal directories.  Since the camera has multiple
 * base directories, each subdirectory may exist in each base. 
*/
void Camera::build_scene_space(){
    
    // create our directory stack which we will perform a Depth-First-Search on.
    deque<string> image_stack;

    // load the directory stack with the contents of each base directory
    for( size_t i=0; i<root_directories.size(); i++ )
        directory_append_internal( root_directories[i], image_stack, IO_ALL);
    
    // some useful variables
    string topStr;
    SceneID id;
    
    // keep iterating until the image stack is empty
    while( image_stack.size() > 0 ){
        
        //pop off the first item
        topStr = image_stack.front();
        image_stack.pop_front();

        //if item is a directory, then add it to the directory tree and keep digging
        if( is_dir( topStr ) == true ){

            //create timeid
            TimeID id( topStr, camera_name );

            //check if the element already exists
            set<TimeID>::iterator it = time_space.find(id);

            if( it != time_space.end() ){

                TimeID actual = (*it);
                actual.decompose_and_add_path(topStr);
                time_space.erase(id);
                time_space.insert(actual);
            }
            else{

                //only add if the depth is less than the max
                //compute the depth
                int depth = TimeID( topStr, camera_name).dirs.size();
                if( depth == 3 ){
                    
                    //add to directory space
                    time_space.insert( TimeID( topStr, camera_name ) );
                }
                
                if( depth < 3 ){
                    //keep adding
                    directory_append_internal( topStr, image_stack, IO_ALL );  
                }
            }
        }

    }


}

/** 
 * Pop the top entry off of the time space and retrieve all 
 * images. Place them onto the current image list.
 * Make sure to sort the list. 
*/
void Camera::decompose_top_directories( ){

    // pop the top time space entry
    TimeID timeEntry = *time_space.begin(); 
    time_space.erase( time_space.begin());
   
    // clear the current image list
    current_image_list.clear();

    // add the directory contents to the list
    for( size_t i=0; i<timeEntry.pathnames.size(); i++ ){
        directory_append_internal( timeEntry.pathnames[i], current_image_list, IO_FILES_ONLY );
    }

    // sort the list by scene number
    sort_TACID_list( current_image_list );

}


/** 
 * Compares the current image list against the input
 * test case.  
*/
void Camera::union_image_list( deque<ImageBundle>& image_list )const{
    
    //compare each list side-by-side until image list has been iterated
    deque<ImageBundle>::iterator itA = image_list.begin();
    deque<string>::const_iterator itB = current_image_list.begin();
    int result;
    while( true ){
        
        //check if we have reached the end of the image list, if yes, then exit
        if( itA == image_list.end() )
            break;

        //if we reached the end of the current image list, 
        //   then remove the rest of the image list
        if( itB == current_image_list.end() ){
            while(itA != image_list.end() ){
                itA = image_list.erase(itA);
            }
        }

        //compare each tacid
        result = TACID_scene_func()( itA->data[0], *itB );

        //if they are equal, add the image to the bundle and increment both pointers
        if( result == 0 ){
            
            itA->data.push_back(*itB);
            itA++;
            itB++;
            continue;
        }

        //if itA is smaller, delete it
        if( result < 0 ){
            itA = image_list.erase( itA );
            continue;
        }

        //if itB is smaller, than increment it
        else{
            itB++;
            continue;
        }

    }
}


/**
 * Return true if the camera is out of directories to search.
*/
bool Camera::empty_time_space()const{
    
    if( time_space.size() <= 0 )
        return true;
    
    return false;
}


/** 
 * Output stream operator 
 */
ostream& operator << ( ostream& ostr, Camera const& camera ){

    ///print header
    ostr << "Camera: " << endl;

    ///print root directories
    ostr << "   root directories: " << endl;
    if( camera.root_directories.size() > 0 ){ 
        for( size_t i=0; i<camera.root_directories.size(); i++)
            ostr << "   " << camera.root_directories[i] << endl;
    }
    ostr << endl;


    return ostr;
}


/**
 *  Convert the cam### string into an actual 
 *  number which can be referenced in an array
*/
int camera2int( const string& dirname ){

    //pull out the last three
    string substr = dirname.substr(dirname.size()-3,3);

    int output;
    stringstream sin;
    sin << substr;
    sin >> output;
    return output;

}

/**
 * Returns true if the character being evaluated
 * is either a number or an upper-case letter between
 * A and F inclusive. 
*/
bool isCharAlphaNumeric( char const& c ){
    
    // check if a number
    if( c >= '0' && c <= '9' ){
        return true;
    }
    //otherwise check if a letter
    if( c >= 'A' && c <= 'F' )
        return true;

    return false;

}


/**
 * checks to make sure that the string matches the requirements
 * to be a camera directory.  The main requirement is that it follows
 * the general template  cam### where ### is a 3 digit hex number.
*/
bool Camera::isValid( string const& name ){

    //pull out the filename from the full path
    string fpath = file_basename(name);

    //we first need to ensure there is a cam in the first three letters
    if( fpath.substr(0,3) != "cam" )
        return false;

    //we need to make sure that the length is 6 characters
    if( fpath.size() != 6 )
        return false;

    //we need to make sure the last three values are alpha numeric
    if( isCharAlphaNumeric( fpath[3] ) == false ) return false;
    if( isCharAlphaNumeric( fpath[4] ) == false ) return false;
    if( isCharAlphaNumeric( fpath[5] ) == false ) return false;

    return true;
}

/**  
 * Find Camera Directories
 * 
 * Searches and loads all camera directories found in system
 */
deque<Camera> find_camera_directories( Options const& options ){

    //create a list of cameras
    deque<Camera> output( options.number_eo_frames );

    //create a list of directories to search
    deque<string> dir_tree(0);

    string cdir;

    /**
      start at the base directory and begin building a list of camera directories.  When 
      we discover one, add it to the respective camera.
      */
    dir_tree.push_back( options.input_base );
    while( true ){

        //pop the first item off of the stack
        cdir = dir_tree.front();
        dir_tree.pop_front();

        //check to see if it is a valid camera directory
        if( Camera::isValid(cdir)){

            //extract the camera number
            int camNum = camera2int( cdir );

            //add camera to the camera list
            output[camNum].root_directories.push_back(cdir);
            output[camNum].camera_name = file_basename( cdir );

        }
        else{

            //query the directory for all internal directories
            directory_append_internal( cdir, dir_tree, IO_DIRECTORIES_ONLY  ); 
        }

        //exit if we are out of directories
        if( dir_tree.size() <= 0 )
            break;
    }

    return output;

}


/**
 * Make sure each camera in the list has the same directory 
 * at the top of its time space. 
*/
void normalize_cameras( deque<Camera>& cameras ){

    bool inSync;
    
    //search until we have a level set of cameras
    while( true ){

        inSync = true;
        for( size_t i=1; i<cameras.size(); i++){

            //if the camera list is empty, then exit
            if( cameras[0].time_space.size() <= 0 || cameras[i].time_space.size() <= 0 ){
                return;
            }

            //keep moving if the elements are equal
            if( *cameras[0].time_space.begin() == *cameras[i].time_space.begin() )
                continue;

            //otherwise, we are out of sync and must correct
            inSync = false;

            //find the smaller element and remove it.
            if(  (*cameras[0].time_space.begin()) < (*cameras[i].time_space.begin()) )
                cameras[0].time_space.erase(cameras[0].time_space.begin());
            else
                cameras[i].time_space.erase(cameras[i].time_space.begin());

            break;
        }

        if( inSync == true )
            break;

    }

}


/**
 * Decompose and pull all image sets for the selected directory
*/
deque<ImageBundle> decompose_top_camera_directories( deque<Camera>& cameras ){

    //create output
    deque<ImageBundle> output;
    
    //iterate through each camera and decompose each directory
    for( size_t i=0; i<cameras.size(); i++ )
        cameras[i].decompose_top_directories();
    
    //using the first image list as the base, union each image list set based
    // on scene number. 
    deque<string> imgList = cameras[0].current_image_list;
    
    
    for( deque<string>::iterator it=imgList.begin(); it != imgList.end(); it++ )
        output.push_back( ImageBundle(*it));

    
    for( size_t i=1; i<cameras.size(); i++ )
        cameras[i].union_image_list( output );
    

    //return output
    return output;
}


/** 
 * Grab all matching and complete image bundles
*/
deque<ImageBundle> compute_image_bundles( deque<Camera>& cameras, Options const& options ){

    
    deque<ImageBundle> bundle_output;
    deque<ImageBundle> bundles;

    // For each camera, initialize the time space
    for( size_t i=0; i<cameras.size(); i++ ){
        cameras[i].build_scene_space();
    }

    int cnt = 0;
    /** Begin comparing directories */
    bool run_loop = true;
    while( run_loop == true ){
        
        cout << cnt++ << endl;
        //stop processing if any camera node is empty
        for( size_t i=0; i<cameras.size(); i++ ){
            if( cameras[i].empty_time_space() == true ){
                run_loop = false; 
                break;
            }
        }
        if( run_loop == false )
            break;
        
        //first check to make sure all cameras have the same top element
        normalize_cameras( cameras );
        
        //stop processing if any camera node is empty
        for( size_t i=0; i<cameras.size(); i++ ){
            if( cameras[i].empty_time_space() == true ){
                run_loop = false;
                break;
            }
        }
        if( run_loop == false )
            break;
        
        cout << "C" << endl;
        //now decompose each directory and search for matching image pairs
        bundles.clear();
        bundles = decompose_top_camera_directories( cameras );
        
        cout << "E" << endl;
        //add the image bundles to the bundle list
        bundle_output.insert( bundle_output.end(), bundles.begin(), bundles.end() );
        
        cout << "Current Size: " << bundle_output.size() << endl;
        if( bundle_output.size() > options.max_bundle_limit )
        {
            cout << "hard limit reached" << endl;
            break;
        }

    }
    
    
    return bundle_output;
}

