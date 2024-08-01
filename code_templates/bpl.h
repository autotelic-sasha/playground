#pragma once
#include "string_util.h"
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
//			- you can escape replacement by a single '\' before the double braces (e.g. \{{don't touch this}})
// 
//		6. There is a small number of functions that can be used to generate special things. 
//			They cannot be used in maps, you just specify that they should be used in the source files, but their arguments can come from the map.
//          They cannot be used in file and directory name substitution; the syntax becomes too complicated.
//			Their syntax is name(arg0, arg1, ... , arg5), the name is not case sensitive.
//          They only allow up to five arguments, and arguments can be an integer, a double, a string (quoted), or a name from the map (case rules work the same)
//			Available functions:
//			- GUID(int) - this is because Visual Studio uses guids to link its internal files and configurations. 
//			  The argument is the id of the guid, GUID(0) is always the same GUID during a single run of the template generation, so is GUID(2) etc.
//		
#include <map>
#include <string>
#include <memory>

namespace autotelica {
    class bpl_impl;

    class bpl {
        std::shared_ptr<bpl_impl> _impl;
    public:
        bpl(
            std::string source_path_,
            std::string target_path_,
            std::map<std::string, std::string> kvm_) {
        }
    };
}