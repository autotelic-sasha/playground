#pragma once
// bpl is a minimalist code generation framework.
// It's opinionated and insists on being simple. Real simple. 
// It can be extended in ways that will make it complicated, sure, anything can.
// Mostly what it does is just replace strings with other strings, in file and directory names, and within files themselves. 
// 
// It takes three inputs:
//		1) the folder containing the file templates
//		2) a mapping of names to values
//		3) a target folder where the new code project is to be generated
//
// It then traverses the source template, replacing names with values (there's rules, see below) where they are found, 
// and generates new files in the target folder. 
// 
// The replacement rules:
//		1. There is a mapping in the input of names to values. This is just a table of strings.
//		2. Names in the mapping are not case sensitive, values are case sensitive.
//		3. Whenever a name is being replaced by a value:
//			- if the name is spelled all in lowercase in the source, it is replaced by the lowercase version of the value in the target
//			- else if the name is spelled all in uppercase in the source, it is replaced by the uppercase version of the value in the target
//			- otherwise it is replaces by the value as it appears in the input map
//
//		4. In file and directory names:
//			- names to be replaced are delimeted by two underscores either side of it (like __name__)
//			- if a name is not found in the map, nothing happens (no errors are thrown)
//				(there is a strict mode of running that makes this an error, if you really want to)
//			- there are no escape characters for file and directory name replacements
// 
//		5. In the files' content
//			- names to be replaced are delimeted by two curly braces either side of it (like {{name}})
//			- if a name is not found in the map, nothing happens (no errors are thrown)
//				(there is a strict mode of running that makes this an error, if you really want to)
//			- whitespace surrounding names in the braces is eaten
//			- nesting is not allowed (e.g. you can't do silly things like {{name1{{name2}}name3}}, it's rude to expect people to be able to read that)
//			- you can escape replacement by putting it in double braces (e.g. {{{{don't touch this}}}} evaluates to {{don't touch this}})
//          - there's no messing with escaping, like if you want to do it, you need it open and closed with four braces
// 
//		6. There is a (very) small number of functions that can be used to generate special things. 
//			They are hardcoded, to add one you gotta write some c++.
//          They cannot be used in maps, you just specify that they should be used in the source files, but their arguments can come from the map.
//          They cannot be used in file and directory name substitution; the syntax becomes too complicated.
//			Their syntax is name(arg0, arg1, ... , arg5), the name is not case sensitive.
//          They only allow up to five arguments, and arguments can be an integer, a double, a string (quoted), or a name from the map (case rules work the same)
//			Available functions:
//			- GUID(int) - this is because Visual Studio uses guids to link its internal files and configurations. 
//			  The argument is the id of the guid, GUID(0) is always the same GUID during a single run of the template generation, so is GUID(2) etc.
//
//      7. For the times when you want to clone a git repo into a subfolder of a project, there is a special file name: __GITCLONE__. 
//          The file should contain a link to a repository on a single line, and nothing else. 
//          For example: https://github.com/autotelic-sasha/autotelica_core.git 
// 
//          Then, during the template instantiation, the repo will be cloned into the directory containining the file. 
//          This works by simple substitution, git clone REPO_NAME_FROM_FILE PATH_TO_FOLDER_WHERE_FILE_IS .
//          Obviously, you can hack it by adding git parameters to the file, e.g. --brach bname https://github.com/autotelic-sasha/autotelica_core.git .
// 
// 
// Where do the values for replacements come from?
//      When running this from a command line, supply a path to a file containing them. 
//      It can be either a json or ini file.
//
//      The class that does all the work will take a map<string, string> as a constructor argument too, 
//      so you can use that if you are extending the funcionality for your own neffarious purposes.
// 
//      The names can be scoped (it's handy if your templates become big), but only to one level. 
//      You do this by creating "sections", a block with a name that becomes first part of all the names
//      defined in that section before a dot ... 
//      Far easier to see an example:
// 
//      The JSON should look like this:
// 
//      {
//          "sections" : [
//              {
//                  "name" : "", 
//                  "named_values": [
//                      { "name" : "blah", "value" : "blahblah" },
//                      { "name" : "ping", "value" : "pong" },
//                      { "name" : "ok", "value" : "enough" }
//                  ]
//              },
//              {
//                  "name" : "meaningfull", 
//                  "named_values": [
//                      { "name" : "boom", "value" : "bang" },
//                      { "name" : "ping", "value" : "pongpong" }
//                  ]
//              }
//          ]
//      }
//      Hint: if you don't need named sections, you can just use "named_values" block at the top level.
// 
//      now the names get mapped like this:
//          blah -> blahblah
//          ping -> pong
//          ok -> enough
//          meaningfull.boom -> bang
//          meaningfull.ping -> pongpong
//      		
//      The equivalent ini file looks like this:
//      
//      ; Comment about the meaning of it all
//      blah = blahblah
//      ping = pong
//      ok = enough
//      [meaningfull]
//      ; Some other thoughts about it all
//      boom = bang
//      ping = pongpong
// 
// What else?
// 
// 1. You can specify file extensions or file names to ignore.
//      These are simply copied, possibly with name changes, but the content is not parsed. 
//      Super handy when you have binary files, like Excel sheets or whatnot, in your templates.
//      You can do this either on the command line or in the input files, just add values like:
// 
//          "extensions_to_ignore" : "xls,exe,so",
//          "files_to_ignore" : "large_cpp.cpp, large_header.h"
//          (in json)
//
//      or
// 
//          extensions_to_ignore = xls,exe,so
//          files_to_ignore = large_cpp.cpp, large_header.h
//          (in ini)
//  
//      or similarly on the command line.

//    Hints: files_to_ignore and extensions to ignore also work with wildcards (e.g. *part\of\path*)
//
//           if a directory matches one of the listed names, everything in that directory is also 
//           not parsed. 
//           (names of files and directories are always parsed though)
//      
//  2. If a same parameter appears both on a command line and in a config file, command line takes presedence.    

#include <map>
#include <string>
#include <memory>

namespace autotelica {
    class bpl_impl;

    class bpl {
        std::shared_ptr<bpl_impl> _impl;
    public:
        bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            std::string const& config_path_,
            bool strict_ = false,
            std::string const& extensions_to_ignore_ = "",
            std::string const& files_to_ignore_ = "");
        
        bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            bool strict_ = false,
            std::string const& extensions_to_ignore_ = "",
            std::string const& files_to_ignore_ = "",
            std::map<std::string, std::string> const& kvm_ = {});

        // generating projects from templates
        void generate();

        // generating blank configuration files
        // then you just populate them with values,it's nice
        void generate_config_files();

        // got a new template to deal with? 
        // or one that you wrote but forgot all about?
        // use 'describe' to get information about it on screen.
        void describe();

    };
}