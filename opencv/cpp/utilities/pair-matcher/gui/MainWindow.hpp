/**
 * @file    MainWindow.hpp
 * @author  Marvin Smith
 * @date    2/17/2016
*/
#ifndef __PAIR_MATCHER_MAIN_WINDOW_HPP__
#define __PAIR_MATCHER_MAIN_WINDOW_HPP__

// Qt Libraries
#include <QDockWidget>
#include <QMainWindow>


// OpenCV Libraries
#include <opencv2/core.hpp>


// Pair Matcher Libraries
#include "../Options.hpp"
#include "TestImageWidget.hpp"


/**
 * @class MainWindow
*/
class MainWindow : public QMainWindow
{
    public:
    
        /**
         * @brief Constructor
        */
        MainWindow( Options const& options );


    private:
        
        /**
         * @brief Build Dock Widgets
        */
        void Build_Dock_Windows();


        /// Class Name
        std::string m_class_name;


        /// Configuration Options
        Options m_options;
        

        /// Dock Widgets
        QDockWidget* m_ref_image_dock;
        QDockWidget* m_tst_image_dock;
        
        
        /// Test Image Data
        TestImageWidget* m_test_image_widget;

}; // End of MainWindow Class

#endif
