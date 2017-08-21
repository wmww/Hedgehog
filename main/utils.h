#pragma once

#include <string>
using std::string;
using std::to_string;

#include <vector>
using std::vector;

#include <iostream>
using std::cout;
using std::endl;

#include <memory>
using std::shared_ptr;
using std::unique_ptr;
using std::enable_shared_from_this;

#include <GL/glew.h>
#include <GL/gl.h>

template<typename T>
struct V2
{
	T x, y;
	
	V2(T xIn, T yIn) { x = xIn; y = yIn; }
};

template<typename T>
string to_string(V2<T> in) { return "(" + to_string(in.x) + ", " + to_string(in.y) + ")"; }

typedef V2<int> V2i;
typedef V2<double> V2d;

enum VerboseToggle
{
	VERBOSE_ON = 1,
	VERBOSE_OFF = 0,
};

class MessageLogger
{
public:
	void status(string msg);
	void important(string msg);
	void fatal(string msg);
	
	static void show(string msg);
	
	VerboseToggle verbose = VERBOSE_ON;
	string tag = "[unknown]";
};

#define assert(condition) if (!(condition)) { MessageLogger::show(__FILE__ ":" + to_string(__LINE__) + ": assertion failed: " #condition); exit(1); }

void logError(string msg);

// loads an entire file into the contents string, returns if it succeeded
bool loadFile(string filename, string& contents, bool debug);

// sleep for the given number of seconds (millisecond precision)
void sleepForSeconds(double seconds);

// returns the path to the folder containing all shaders (ends in slash)
// this is abstracted into a function because in the future it may need to be determined dynamically
inline string getShaderPath() {return "shaders/";}

template<typename T>
void setupIfFirstInstance(T * type)
{
	static bool hasInitialized = false;
	
	if (!hasInitialized)
	{
		T::firstInstanceSetup();
		hasInitialized = true;
	}
}
