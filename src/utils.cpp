#include "../h/utils.h"

#include <fstream>

#include <sstream>
using std::stringstream;

#include <chrono>
#include <thread>

void logError(string msg)
{
	std::cerr << "error: " << msg << std::endl;
}

bool loadFile(string filename, string& contents, bool debug)
{
	std::fstream inFile;
	
	if (debug)
		cout << "attempting to open '" << filename << "'..." << endl;
	
	inFile.open(filename);
	
	if (!inFile.is_open())
	{
		if (debug)
			cout << "'" << filename << "' failed to open :(" << endl;
		return false;
	}
	else
	{
		if (debug)
			cout << "file opended, reading file..." << endl;
		
		stringstream strStream;
		strStream << inFile.rdbuf();	// read the file
		contents = strStream.str();	// str holds the content of the file
		inFile.close();
		
		if (debug)
			cout << "file reading done, '" << filename << "' closed" << endl;
		
		return true;
	}
}

void sleepForSeconds(double seconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
}

MessageLogger::MessageLogger(string tagIn, VerboseToggle verboseIn, MessageLogger * parentIn = nullptr)
{
	verbose = verboseIn;
	tag = tagIn;
	if (tag == "")
		tag = "[unknown]";
	parent = parentIn;
}

void MessageLogger::status(string msg)
{
	if (verbose)
	{
		if (parent)
		cout << tag << ": " << 
	}
}

void MessageLogger::log(string msg)
{
	if (parent)
		parent->log(tag + ": " + msg);
	else
		cout << tag << ": " << msg;
}
