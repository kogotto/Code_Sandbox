/**
 * @file    A_CLI_Command_Parser_Factory.cpp
 * @author  Marvin Smith
 * @date    5/18/2015
*/
#include "A_CLI_Command_Parser_Factory.hpp"

// C++ Standard Libraries
#include <iostream>
#include <sstream>

// PUGI Xml
#include "../../thirdparty/pugixml.hpp"

namespace CLI{
namespace CMD{


/*********************************************/
/*          Create a Command Parser          */
/*********************************************/
A_CLI_Command_Parser::ptr_t  A_CLI_Command_Parser_Factory::Initialize( const std::string& config_path )
{
    // Create XML Document
    pugi::xml_document xmldoc;
    pugi::xml_parse_result result = xmldoc.load_file( config_path.c_str() );

    if( result == false ){
        std::stringstream sin;
        sin << "error: " << __FILE__ << ", Line: " << __LINE__ << ". CLI Command Configuration File parsed with errors. Details: " << result.description();
        std::cerr << sin.str() << std::endl;
        return nullptr;
    }

    
    // Get the root node
    pugi::xml_node root_node = xmldoc.child("command-configuration");

    // Check the node
    if( root_node == pugi::xml_node() ){ return nullptr; }


    // Get the regex split pattern
    std::string regex_split_pattern = root_node.child("regex-split-pattern").attribute("value").as_string();

    
    // Get the Parser Command Nodes
    pugi::xml_node parser_commands_node = root_node.child("parser-commands");
    std::vector<CMD::A_CLI_Parser_Command> parser_command_list;

    // Iterate over each command
    CMD::A_CLI_Parser_Command parser_command( CMD::CLICommandParseStatus::UNKNOWN );
    for( pugi::xml_node_iterator pit = parser_commands_node.begin(); pit != parser_commands_node.end(); pit++ )
    {

        // Get the node
        pugi::xml_node parser_command_node = (*pit);

        // Get the mode
        std::string mode_str = parser_command_node.attribute("mode").as_string();

        // set the mode
        if( mode_str == "shutdown" ){
            parser_command = CMD::A_CLI_Parser_Command( CMD::CLICommandParseStatus::CLI_SHUTDOWN );
        }
        else if( mode_str == "help" ){
            parser_command = CMD::A_CLI_Parser_Command( CMD::CLICommandParseStatus::CLI_HELP );
        }
        else{
            std::cerr << "error: Unknown parser command mode (" << mode_str << ")" << std::endl;
            return nullptr;
        }

        // Iterate over names
        for( pugi::xml_node_iterator ait = parser_command_node.begin(); ait != parser_command_node.end(); ait++ )
        {
            parser_command.Add_Name( (*ait).attribute("value").as_string());
        }

        // Add parser command
        parser_command_list.push_back(parser_command);

    }


    // Get the Commands Node
    pugi::xml_node commands_node = root_node.child("commands");

    // Command List
    std::vector<A_CLI_Command> command_list;
    std::vector<A_CLI_Command_Argument> argument_list;

    // Parse the Commands Node
    for( pugi::xml_node_iterator it = commands_node.begin(); it != commands_node.end(); it++ )
    {
        
        // Clear the argument list
        argument_list.clear();

        // Convert to node
        pugi::xml_node command_node = (*it);
        
        // Get the command name
        std::string command_name  = command_node.child("name").attribute("value").as_string();
        
        // Get the command description
        std::string command_description = command_node.child("description").attribute("value").as_string();
        
        // Get the arguments node
        pugi::xml_node arguments_node = command_node.child("arguments");
        for( pugi::xml_node_iterator ait = arguments_node.begin(); ait != arguments_node.end(); ait++ )
        {
            // Get the node
            pugi::xml_node arg_node = (*ait);

            // Add the next argument
            argument_list.push_back( A_CLI_Command_Argument( arg_node.attribute("name").as_string(),
                                                             StringToCLICommandArgumentType( arg_node.attribute("type").as_string()),
                                                             arg_node.attribute("description").as_string()));

        }

        // Add the command
        command_list.push_back(A_CLI_Command( command_name, command_description, argument_list ));

    }

    // Create the parser
    A_CLI_Command_Parser::ptr_t parser = std::make_shared<A_CLI_Command_Parser>( regex_split_pattern, parser_command_list, command_list );

    // Return new parser
    return parser;
}

} // End of CMD Namespace
} // End of CLI Namespace

