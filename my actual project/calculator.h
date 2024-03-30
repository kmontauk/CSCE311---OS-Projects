// Copyright D. L. Devito 2024
#ifndef PROJ1_CALCULATOR_H_
#define PROJ1_CALCULATOR_H_
#include <string>
#include <vector>
using std::string, std::vector;

/**
 * Calculate() takes an integer representing the size of a vector, as well as
 * the corresponding string vector, and calculates the final result before
 * returning a double value, solving the infix equation and returning it to 
 * main.cc
**/
double calculate(int size, std::vector<string> string_vec);

/**
 * convert_string_to_double() takes a string vector and an integer representing
 * the size of that vector, and creates an equivalent double vector holding
 * values that can be used for calculations directly. This returns to
 * calculate()
*/
vector<double> convert_string_to_double(int size, vector<string> str_vec);

#endif  // PROJ1_CALCULATOR_H_

