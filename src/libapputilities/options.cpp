/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "options.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

#include <ciso646>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace // unnamed
{
// Utility function to enable the syntax "@filename" to load a file containing command-line options (a so-called response file)
// Code taken from Boost 1.60 program_options documentation, file "example/response_file.cpp"
// Additional command line parser which interprets '@something' as a
// option "option-file" with the value "something"
std::pair<std::string, std::string> at_option_parser(std::string const&s)
{
    if ('@' == s[0])
        return std::make_pair(std::string("option-file"), s.substr(1));
    else
        return std::pair<std::string, std::string>();
}

} // unnamed namespace

namespace visr
{
namespace apputilities
{

Options::Options()
{
  registerOption<bool>( "help,h", "Show help and usage information." );
  registerOption<bool>( "version,v", "Display version information." );

  registerOption<std::string>( "option-file", "Load options from a file. Can also be used with syntax \"@<filename>\"." );
}

Options::~Options()
{
}

Options::ParseResult Options::parse( int argc, char const * const * argv, std::ostream & errMsg )
{
  try
  {
    boost::program_options::store( boost::program_options::command_line_parser( argc, argv).options( mDescription )
                                   .extra_parser(at_option_parser).run(), // Register transformation @filename->--option-file=filename syntax
								   // (taken from Boost documentation example.
								   mVariablesMap );
    notify( mVariablesMap );
  }
  catch( std::exception const & ex )
  {
    errMsg << ex.what();
    return ParseResult::Failure;
  }
  // Register the options in a options file as ordinary options.
  // Taken and adapted from Boost program option documentation, file example/response.file.cpp
  if( hasOption("option-file"))
  {
    // Load the file and tokenize it
    boost::filesystem::path const optionFilePath( absolute(boost::filesystem::path( getOption<std::string>( "option-file"))) );
    if( not boost::filesystem::exists(optionFilePath) or is_directory(optionFilePath) )
    {
      errMsg << "The file specified by the \"--option-file\" option does not exist.";
      return ParseResult::Failure;
    }
    std::ifstream ifd ( optionFilePath.string() );
    // Read the whole file into a string
    std::stringstream fileContent;
    fileContent << ifd.rdbuf();
    // Split the file content
    // Differing from the boost example, we use an escaped_list_separator to allow quoted strings with spaces.
    // Also added '=' to the separator chars to allow options as '-i=8' or '--input-channels=8' in the file.
    boost::escaped_list_separator<char> sep( "\\", " =\n\r", "\"" );
    std::string sstr = fileContent.str();
    boost::tokenizer<boost::escaped_list_separator<char> > tok(sstr, sep);
    std::vector<std::string> args;
    std::copy(tok.begin(), tok.end(), std::back_inserter(args));
    // Parse the file and store the options
    store(boost::program_options::command_line_parser(args).options(mDescription).run(), mVariablesMap);
  }

  if( getOption<bool>( "help" ) )
  {
    return ParseResult::Help;
  }
  if( getOption<bool>( "version" ) )
  {
    return ParseResult::Version;
  }
  return ParseResult::Success;
}
  
bool Options::hasOption( char const * optionName ) const
{
  return (mVariablesMap.count( optionName ) >= 1);
}

void Options::printDescription( std::ostream & out )
{
  out << mDescription << std::endl;
}

} // namespace apputilities
} // namespace visr
