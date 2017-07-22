#pragma once

#include <string>
using std::string;
using std::to_string;

#include <vector>
using std::vector;

#include <iostream>
using std::cout;
using std::endl;

// loads an entire file into the contents string, returns if it succeeded
bool loadFile(string filename, string& contents, bool debug);
