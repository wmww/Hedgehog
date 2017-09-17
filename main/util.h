#pragma once

#include <string>
using std::string;
using std::to_string;

#include <vector>
using std::vector;

#include <iostream>
//using std::cout;
//using std::endl;

#include <memory>
using std::shared_ptr;
using std::enable_shared_from_this;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;
using std::weak_ptr;

#include <functional>
using std::function;

#include <GL/glew.h>
#include <GL/gl.h>

// a simple 2D vector class (vector in the x, y sense, not the array sense)
template<typename T>
struct V2
{
	T x, y;
	
	V2() { x = y = T(); }
	V2(T xIn, T yIn) { x = xIn; y = yIn; }
};

template<typename T>
string to_string(V2<T> in) { return "(" + to_string(in.x) + ", " + to_string(in.y) + ")"; }

typedef V2<int> V2i;
typedef V2<double> V2d;

#define FUNC string(__FUNCTION__)

// instead of using logMessage directly, it is generally a good idea to use the debug, warning, fatal and assert macros
enum MessageType { MESSAGE_DEBUG, MESSAGE_WARNING, MESSAGE_ASSERTION_FAILED, MESSAGE_FATAL_ERROR };
void logMessage(string source, MessageType type, string messaage); // this function does NOT support unicode

// the function-like macros debug, warning and fatal each take a string and print it along with the file and line number
// this is made possible by the __FILE__ and __LINE__ macros
// all debugs in a file can be disabled at 0 run time cost by putting #define NO_DEBUG at the top of the file (before includes)
// fatal automatically kills the program as soon as its done logging
// assert is used to easily check a boolean expression, and fatally error if its false
#define FILE_INFO string(__FILE__) + ":" + (__LINE__ < 100 ? (__LINE__ < 10 ? "   " : "  ") : (__LINE__ < 1000 ? " " : "")) + std::to_string(__LINE__)
#define debug_off(message)
#define debug_on(message) logMessage(FILE_INFO, MESSAGE_DEBUG, message)
#define warning(message) logMessage(FILE_INFO, MESSAGE_WARNING, message)
#define fatal(message) logMessage(FILE_INFO, MESSAGE_FATAL_ERROR, message)
#define assert(condition) if (!(condition)) { logMessage(FILE_INFO, MESSAGE_ASSERTION_FAILED, "assertion '" #condition "' failed"); }

//#define ASSERT_OR_BUST(condition) if (!(condition)) { logMessage(FILE_INFO, MESSAGE_ASSERTION_FAILED, "assertion '" #condition "' failed"); }
//#define ASSERT_ELSE_RETURN(condition) if (!(condition)) { logMessage(FILE_INFO, MESSAGE_WARNING, "assertion '" #condition "' failed; returning early from " + FUNC); return; }
#define ASSERT_ELSE(condition, action) if (!(condition)) { logMessage(FILE_INFO, MESSAGE_WARNING, "assertion '" #condition "' failed in " + FUNC); action; }
#define ASSERT_THEN(condition) ASSERT_ELSE(condition, ) else
#define ASSERT(condition) ASSERT_ELSE(condition, )
//#define ASSERT_ELSE_IGNORE(condition) ASSERT_ELSE(condition, )

// loads an entire file into the contents string, returns if it succeeded
bool loadFile(string filename, string& contents);

// sleep for the given number of seconds (millisecond precision)
void sleepForSeconds(double seconds);

// get the time (in seconds) since the program started
double timeSinceStart();
int timeSinceStartMili();

// returns the path to the folder containing all shaders (ends in slash)
// this is abstracted into a function because in the future it may need to be determined dynamically
inline string getShaderPath() {return "shaders/";}

// for lazy evaluation of static class members
// to use, put a static function in your class called firstInstanceSetup, and call setupIfFirstInstance with this in the constructor
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
