#include "utils.h"

#include <fstream>

#include <sstream>
using std::stringstream;

#include <chrono>
#include <thread>

#include <unistd.h> // for terminal size detection
#include <sys/ioctl.h> // for terminal size detection

int getTermWidth()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

void logMessage(string source, MessageType type, string message)
{
	const string indent =		" |  ";
	const string lastIndent =	" |_ ";
	
	int messageMaxWidth = getTermWidth();
	
	std::ostream * stream = &std::cerr;
	if (type == MESSAGE_DEBUG)
		stream = &std::cout;
	
	string typeStr;
	switch (type)
	{
	case MESSAGE_DEBUG:
		typeStr = "DEBUG";
		break;
	case MESSAGE_WARNING:
		typeStr = "WARNING";
		break;
	case MESSAGE_ASSERTION_FAILED:
		typeStr = "ASSERTION FAILED";
		break;
	case MESSAGE_FATAL_ERROR:
		typeStr = "FATAL ERROR";
		break;
	default:
		typeStr = "UNKNOWN MESSAGE TYPE";
	}
	string msg = typeStr + " [" + source + "]: " + message;
	vector<string> lines;
	
	int start = 0;
	int end = (int)msg.size();
	
	while (start < end)
	{
		int lineEnd = start + messageMaxWidth - (start == 0 ? 0 : (int)indent.size());
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
		if (start != 0)
			line += (lineEnd == end ? lastIndent : indent);
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
