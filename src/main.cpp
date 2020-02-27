/*
	*** Dissolve Main
	*** src/main.cpp
	Copyright T. Youngs 2012-2020

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "version.h"
#include "base/messenger.h"
#include "main/dissolve.h"
#include "base/processpool.h"
#include <time.h>
#include <ctime>
#include <stdlib.h>
#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
#ifdef PARALLEL
	// Initialise parallel communication
	ProcessPool::initialiseMPI(&argc, &argv);
#endif

	// Instantiate main classes
	CoreData coreData;
	Dissolve dissolve(coreData);

	// Parse CLI options...
	CharString inputFile, redirectFileName, restartDataFile, outputInputFile;
	int nIterations = 5;
	bool ignoreRestart = false;

	//
	po::options_description desc("Allowed Options");
	desc.add_options()
	  ("check,c", "Check input and set-up only - don't perform any main-loop iterations")
	  ("help,h", "Print what you're reading now")
	  ("output-file,f", po::value<string>()->value_name("OUTPUT_FILE"), "Redirect output from all process to 'OUTPUT_FILE.N', where N is the process rank")
	  ("ignore,i", po::bool_switch(&ignoreRestart), "Ignore restart file")
	  ("master,m", po::bool_switch()->notifier(Messenger::setMasterOnly),
	   "Restrict output to be from the master process alone (parallel code only)")
	  ("iterations,n", po::value<int>(&nIterations)->value_name("LOOPS"), "Run for the specified number of main loop iterations, then stop")
	  ("quiet,q", po::bool_switch()->notifier(Messenger::setQuiet),
	   "Quiet mode - print no output")
	  ("restart,r", po::value<int>()->value_name("FREQUENCY"), "Set restart file frequency (default = 10)")
	  ("single,s", "Perform single main loop iteration and then quit")
	  ("initial-restart-file,t",
	   po::value<string>()->value_name("RESTART_FILE"),
	   "Load restart data from specified file (but still write to standard restart file)")
	  ("verbose,v", po::bool_switch()->notifier(Messenger::setVerbose),
	   "Verbose mode - be a little more descriptive throughout")
	  ("write,w", po::value<string>()->value_name("OUTPUT_FILE"), "Write input to specified file after reading it, and then quit")
	  ("no-write,x", "Don't write restart or heartbeat files (but still read in the restart file if present)");

	po::options_description hidden("Hidden Options");
	hidden.add_options()
	  ("input-file", po::value<string>()->value_name("INPUT_FILE"),
	   "The file with the ");

	po::positional_options_description positional;
	positional.add("input-file", 1);

	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(positional).run(), vm);
	po::notify(vm);

	if(vm.count("help")) {
	  std::cout << argv[0] << " INPUT" << std::endl;
	  std::cout << desc << std::endl;
	  ProcessPool::finalise();
	  Messenger::ceaseRedirect();
	  return 1;
	}
	if(vm.count("check")) {
	  nIterations = 0;
	  Messenger::print("System input and set-up will be checked, then Dissolve will exit.\n");
	}
	if(vm.count("output-file")) {
	  redirectFileName.sprintf("%s.%i", vm["output-file"].as<string>(),
				   ProcessPool::worldRank());
	  Messenger::enableRedirect(redirectFileName.get());
	}
	if(vm.count("iterations")) {
	  Messenger::print("%i main-loop iterations will be performed, then Dissolve will exit.\n", nIterations);
	}
	if(vm.count("restart")) {
	  dissolve.setRestartFileFrequency(vm["restart"].as<int>());
	  if (dissolve.restartFileFrequency() <= 0) Messenger::print("Restart file will not be written.\n");
	  else if (dissolve.restartFileFrequency() == 1) Messenger::print("Restart file will be written after every iteration.\n", dissolve.restartFileFrequency());
	  else Messenger::print("Restart file will be written after every %i iterations.\n", dissolve.restartFileFrequency());
	}
	if(vm.count("single")) {
	  Messenger::print("Single main-loop iteration will be performed, then Dissolve will exit.\n");
	  nIterations = 1;
	}
	if(vm.count("initial-restart-file")) {
	  restartDataFile = vm["initial-restart-file"].as<string>().c_str();
	  Messenger::print("Restart data will be loaded from '%s'.\n", restartDataFile.get());
	}
	if(vm.count("verbose")) {
	  Messenger::printVerbose("Verbose mode enabled.\n");
	}
	if(vm.count("write")) {
	  outputInputFile = vm["write"].as<string>().c_str();
	  Messenger::print("Input file will be written to '%s' once read.\n", outputInputFile.get());
	}
	if(vm.count("no-write")) {
	  dissolve.setRestartFileFrequency(0);
	  dissolve.setWriteHeartBeat(false);
	  Messenger::print("No restart or heartbeat files will be written.\n");
	}
	if(vm.count("input-file")) {
	  inputFile = vm["input-file"].as<string>().c_str();
	}

	// Print GPL license information
#ifdef PARALLEL
	Messenger::print("Dissolve PARALLEL version %s, Copyright (C) 2012-2020 T. Youngs.\n", DISSOLVEVERSION);
#else
	Messenger::print("Dissolve SERIAL version %s, Copyright (C) 2012-2020 T. Youngs.\n", DISSOLVEVERSION);
#endif
	Messenger::print("Source repository: %s.\n", DISSOLVEREPO);
	Messenger::print("Dissolve comes with ABSOLUTELY NO WARRANTY.\n");
	Messenger::print("This is free software, and you are welcome to redistribute it under certain conditions.\n");
	Messenger::print("For more details read the GPL at <http://www.gnu.org/copyleft/gpl.html>.\n");

	// Check module registration 
	Messenger::banner("Available Modules");
	if (!dissolve.registerMasterModules())
	{
		ProcessPool::finalise();
		Messenger::ceaseRedirect();
		return 1;
	}

	// Load input file - if no input file was provided, exit here
	Messenger::banner("Parse Input File");
	if (inputFile.isEmpty())
	{
		Messenger::print("No input file provided. Nothing more to do.\n");
		ProcessPool::finalise();
		Messenger::ceaseRedirect();
		return 1;
	}
	if (!dissolve.loadInput(inputFile))
	{
		ProcessPool::finalise();
		Messenger::ceaseRedirect();
		return 1;
	}

	// Save input file to new output filename and quit?
	if (!outputInputFile.isEmpty())
	{
		Messenger::print("Saving input file to '%s'...\n", outputInputFile.get());
		bool result;
		if (dissolve.worldPool().isMaster())
		{
			result = dissolve.saveInput(outputInputFile);
			if (result) dissolve.worldPool().decideTrue();
			else dissolve.worldPool().decideFalse();
		}
		else result = dissolve.worldPool().decision();
		if (!result) Messenger::error("Failed to save input file to '%s'.\n", outputInputFile.get());
		ProcessPool::finalise();
		Messenger::ceaseRedirect();
		return result ? 0 : 1;
	}

	// Load restart file if it exists
	Messenger::banner("Parse Restart File");
	if (!ignoreRestart)
	{
		// We may have been provided the name of a restart file to read in...
		CharString restartFile;
		if (restartDataFile.isEmpty()) restartFile.sprintf("%s.restart", inputFile.get());
		else restartFile = restartDataFile;
		
		if (DissolveSys::fileExists(restartFile))
		{
			Messenger::print("Restart file '%s' exists and will be loaded.\n", restartFile.get());
			if (!dissolve.loadRestart(restartFile.get()))
			{
				Messenger::error("Restart file contained errors.\n");
				ProcessPool::finalise();
				Messenger::ceaseRedirect();
				return 1;
			}

			// Reset the restart filename to be the standard one
			dissolve.setRestartFilename(CharString("%s.restart", inputFile.get()));
		}
		else Messenger::print("Restart file '%s' does not exist.\n", restartFile.get());
	}
	else Messenger::print("Restart file (if it exists) will be ignored.\n");

	// Prepare for run
	if (!dissolve.prepare())
	{
		ProcessPool::finalise();
		Messenger::ceaseRedirect();
		return 1;
	}

#ifdef PARALLEL
	Messenger::print("This is process rank %i of %i processes total.\n", ProcessPool::worldRank(), ProcessPool::nWorldProcesses());
#endif

	// Run main simulation
	bool result = dissolve.iterate(nIterations);

	// Print timing information
	dissolve.printTiming();

	// Clear all data
	dissolve.clear();

	if (result) Messenger::print("Dissolve is done.\n");
	else Messenger::print("Dissolve is done, but with errors.\n");

	// End parallel communication
	ProcessPool::finalise();

	// Stop redirecting
	Messenger::ceaseRedirect();

	// Done.
	return (result ? 0 : 1);
}
