#include "util.h"

#include <fstream>

#include <sstream>
using std::stringstream;

#include <chrono>
#include <thread>

#include <unistd.h> // for terminal size detection
#include <sys/ioctl.h> // for terminal size detection

// change to toggle debug statements on and off
#define debug debug_off

std::chrono::high_resolution_clock::time_point programStartTime = std::chrono::high_resolution_clock::now();

int getTermWidth()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

void logMessage(string source, MessageType type, string message)
{
	//const string indent =		" |  ";
	//const string lastIndent =	" |_ ";
	
	int termWidth = getTermWidth() - 1;
	
	std::ostream * stream = &std::cerr;
	if (type == MESSAGE_DEBUG)
		stream = &std::cout;
	
	string typeStr;
	switch (type)
	{
	case MESSAGE_DEBUG:
		typeStr = "debug";
		break;
	case MESSAGE_WARNING:
		typeStr = "WARNING";
		break;
	case MESSAGE_ASSERTION_FAILED:
		typeStr = "FATAL ASSERTION";
		break;
	case MESSAGE_FATAL_ERROR:
		typeStr = "FATAL ERROR";
		break;
	default:
		typeStr = "UNKNOWN MESSAGE TYPE";
	}
	string msg = "[" + source + "]: " + message;
	vector<string> lines;
	int messageWidth = std::max(termWidth - (int)typeStr.size(), 12);
	//string indent = [&]() -> string { string out = ""; for (int i = 0; i < (int)typeStr.size() - 1; i++) { out += " "; } return out; } ();
	string indent = [&]() -> string { string out = ""; for (int i = 0; i < 2; i++) { out += " "; } return out; } ();
	//string lastIndent = indent + "|_";
	string lastIndent = indent + "  ";
	//indent += "| ";
	indent += "  ";
	
	int start = 0;
	int end = (int)msg.size();
	
	while (start < end)
	{
		//int lineEnd = start + messageMaxWidth - (start == 0 ? 0 : (int)typeStr.size());
		int lineEnd = start + messageWidth;
		int splitPoint;
		if (lineEnd < end)
		{
			splitPoint = lineEnd - 1;
			while (splitPoint > start && msg[splitPoint] != ' ' && msg[splitPoint] != '\n') { splitPoint--; }
			if (splitPoint == start)
				splitPoint = lineEnd;
		}
		else
		{
			splitPoint = lineEnd = end;
		}
		string line;
		if (start == 0)
			line += typeStr + " ";
		else if (lineEnd == end)
			line += lastIndent;
		else
			line += indent;
		line += msg.substr(start, splitPoint - start);
		lines.push_back(line);
		start = splitPoint;
		if (start < end && (msg[start] == ' '))
			start++;
	}
	
	for (auto i: lines)
	{
		*stream << i << std::endl;
	}
	
	if (type == MESSAGE_FATAL_ERROR || type == MESSAGE_ASSERTION_FAILED)
		exit(1);
}

bool loadFile(string filename, string& contents)
{
	std::fstream inFile;
	
	debug("attempting to open '" + filename + "'...");
	
	inFile.open(filename);
	
	if (!inFile.is_open())
	{
		debug("'" + filename + "' failed to open :(");
		return false;
	}
	else
	{
		debug("file opended, reading file...");
		
		stringstream strStream;
		strStream << inFile.rdbuf();	// read the file
		contents = strStream.str();	// str holds the content of the file
		inFile.close();
		
		debug("file reading done, '" + filename + "' closed");
		
		return true;
	}
}

void sleepForSeconds(double seconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
}

double timeSinceStart()
{
	return (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - programStartTime)).count();
}

int timeSinceStartMili()
{
	return timeSinceStart() * 1000;
}

/*
void MessageLogger::status(string msg)
{
	if (verbose)
	{
		important(msg);
	}
}

void MessageLogger::important(string msg)
{
	show(tag + ": " + msg);
}

void MessageLogger::show(string msg)
{
	cout << msg << endl;
}
*/

