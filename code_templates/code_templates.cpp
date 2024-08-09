// code_templates.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>

#include "bpl.h"

#include "autotelica_core/util/include/cl_parsing.h"
#include "autotelica_core/util/include/asserts.h"

using namespace autotelica;
using namespace autotelica::cl_parsing;

int main(int argc, const char* argv[])
{

	AF_ASSERTS_SHORT_FORM();

	// define DEBUGGING_ARGS when debugging, then you can define
	// debug versions of argv and argc to use, makes debugging a whole lot easier
#define DEBUGGING_ARGS 	
#ifdef DEBUGGING_ARGS
	const char* debug_argv[] = {
		"code_templates",
			"-s", "C:/dev/autotelica/playground/code_templates/examples/template/",
			//"-t", "C:/dev/autotelica/playground/code_templates/examples/target/",
			"-c", "C:/dev/autotelica/playground/code_templates/examples/example_config.ini",
			"-strict",
			"-ignore_files", "*non_parsed*",
			"-ignore_extensions", "xls*,dll,exe",
			"-generate_config" };

	int debug_argc = sizeof(debug_argv) / sizeof(const char*);
#endif
	try {

		// setting up the command line parser
		std::shared_ptr<bpl> _bpl;

		auto generate = [&](std::vector<std::string> const&) { _bpl->generate(); };
		auto generate_config = [&](std::vector<std::string> const&) { _bpl->generate_config_files(); };
		auto describe = [&](std::vector<std::string> const&) { _bpl->describe(); };

		cl_commands commands("Autotelica simple code template generation.");

		commands
			.register_command(
				"Source path",
				"Path to the source template.",
				{ "s", "source", "source_path" },
				1)
			.register_command(
				"Target path",
				"Path to the folder where template will be instantiated.",
				{ "t", "target", "target_path" },
				1)
			.register_command(
				"Config file path",
				"Path to the configuration file.",
				{ "c", "config", "config_path" },
				1)
			.register_command(
				"Strict",
				"Strict running mode.",
				{ "strict" },
				0)
			.register_command(
				"Extensions to ignore",
				"List of file extensions for files that should not parsed, must be quoted (e.g. \"*.xls, *.exe\").",
				{ "e", "ignore_extensions", "extensions_to_ignore" },
				1)
			.register_command(
				"Files to ignore",
				"Files of folder names of files that should not parsed, must be quoted (e.g. \"sheets, bin\").",
				{ "f", "ignore_files", "files_to_ignore" },
				1)
			.register_command(
				"Generate code",
				"Generate code out of the supplied template.",
				{ "generate" },
				0,
				generate)
			.register_command(
				"Generate configuration file",
				"Generate configuration file with no values populated.",
				{ "generate_config" },
				0,
				generate_config)
		.register_command(
			"Describe template",
			"Print the list of all the names and functions used per file in template.",
			{ "describe" },
			0,
			describe);

		// parsing the command line
#ifdef DEBUGGING_ARGS
		commands.parse_command_line(debug_argc, debug_argv);
#else
		commands.parse_command_line(argc, argv);
#endif

		// if there's nothing to do, show help
		if (commands.executors().empty()) {
			commands.help();
			return 0;
		}

		// get the arguments into something we can work with
		std::string source_path;
		std::string target_path;
		std::string config_path;
		bool strict = false;
		std::string extensions_to_ignore;
		std::string files_to_ignore;

		if (commands.has("source_path"))
			source_path = commands.arguments("source_path")[0];
		if (commands.has("target_path"))
			target_path = commands.arguments("target_path")[0];
		if (commands.has("config_path"))
			config_path = commands.arguments("config_path")[0];
		strict = commands.has("strict");
		if (commands.has("extensions_to_ignore"))
			extensions_to_ignore = commands.arguments("extensions_to_ignore")[0];
		if (commands.has("files_to_ignore"))
			files_to_ignore = commands.arguments("files_to_ignore")[0];

		AF_ASSERT(commands.has("generate") || commands.has("generate_config") || commands.has("describe"),
			"Nothing for the application to do, you should supply either 'generate', 'generate_config' or 'describe. as arguments.");

		_bpl = std::shared_ptr<bpl>(new bpl(
			source_path,
			target_path,
			config_path,
			strict,
			extensions_to_ignore,
			files_to_ignore
		));

		if (commands.has("generate")) {
			AF_ASSERT(!commands.has("generate_config") && !commands.has("describe"),
				"It's really hard to generate configuration and code at the same time, just choose one of them please.");
			AF_ASSERT(!source_path.empty(),
				"Can't really generate code without the template, source path must be supplied.");
			AF_ASSERT(!target_path.empty(),
				"Without target path, don't know where to generate code, target path must be supplied.");
		}
		else if (commands.has("generate_config")) {
			AF_ASSERT(!source_path.empty(),
				"Can't really generate configurations without the template, source path must be supplied.");
			AF_ASSERT(target_path.empty(),
				"Target path has no purpose when generating configurations.");
			AF_ASSERT(!config_path.empty(),
				"Can't really generate configurations without the template, source path must be supplied.");
		}
		else if (commands.has("describe")) {
			AF_ASSERT(!source_path.empty(),
				"Can't really describe things without the template, source path must be supplied.");
			AF_ASSERT(target_path.empty(),
				"Target path has no purpose when descirbing templates.");
		}

		commands.execute();

		if (commands.has("generate")) {
			std::cout << "\n\nDone creating project in folder " << target_path << std::endl;
		}
		else if (commands.has("generate_config")) {
			std::cout << "\n\nDone creating " << config_path << std::endl;
		}
		else {
			std::cout << "\n\nDone describing " << source_path << std::endl;
		}
	}
	catch (...) {
		std::cout << "\n\nAn error occured." << std::endl;
		return -1;
	}
}


