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

template<typename T>
struct V2
{
	T x, y;
	
	V2(T xIn, T yIn) {x = xIn; y = yIn;}
};

typedef V2<int> V2i;
typedef V2<double> V2d;

// loads an entire file into the contents string, returns if it succeeded
bool loadFile(string filename, string& contents, bool debug);
