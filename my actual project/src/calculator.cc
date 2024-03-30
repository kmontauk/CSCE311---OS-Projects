// Copyright D. 'L.' Devito 2024
#include <calculator.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using std::string, std::vector, std::stod, std::cout, std::endl;

namespace calculator {

/**
 * Calculate() takes an integer representing the size of a vector, as well as
 * the corresponding string vector, and calculates the final result before
 * returning a double value, solving the infix equation and returning it to 
 * main.cc
**/
double calculate(int size, vector<string> str_vec) {
    // Define the operand flags used in convert_string_to_double
    double add_flag = 85154372.17543542325;
    double suby_flag = 529816979.14614;
    double multi_flag = 340909.415216924;

    // Define the double vector used to calculate final answer.
    vector<double> double_vec;
    double_vec = convert_string_to_double(size, str_vec);

    // Iterate through array, parsing for Multi operands.
    bool multi = true;
    // Outer while verifies we finish multiplying before dividing.
    while (multi == true) {
        // Gotta check for multi_flags, if none found, break.
        auto check = std::find(double_vec.begin()
        , double_vec.end(), multi_flag);
        if (check == double_vec.end()) {
            multi = false;
            break;
        }
        for (int i = 1; i < size - 1; i+=2) {
            if (double_vec[i] == multi_flag) {
                double temp = double_vec[i - 1] * double_vec[i + 1];
                double_vec.erase(double_vec.begin() + i + 1);
                double_vec.erase(double_vec.begin() + i);
                double_vec.erase(double_vec.begin() + i - 1);
                double_vec.insert(double_vec.begin() + i - 1, temp);
                size -= 2;
            }
        }
    }

    // Now, finish up adding and subtracting.
    bool add_sub = true;
    while (add_sub == true) {
        auto check_add = std::find(double_vec.begin(),
        double_vec.end(), add_flag);
        auto check_sub = std::find(double_vec.begin(),
         double_vec.end(), suby_flag);
        if (check_add == double_vec.end() && check_sub == double_vec.end()) {
            add_sub = false;
            break;
        }
        for (int i = 1; i < size; i+=1) {
            if (double_vec[i] == add_flag) {
                double temp = double_vec[i - 1] + double_vec[i + 1];
                double_vec.erase(double_vec.begin() + i + 1);
                double_vec.erase(double_vec.begin() + i);
                double_vec.erase(double_vec.begin() + i - 1);
                double_vec.insert(double_vec.begin() + i - 1, temp);
                size -= 2;
                // i = 1;
            } if (double_vec[i] == suby_flag) {
                double temp = double_vec[i - 1] - double_vec[i + 1];
                double_vec.erase(double_vec.begin() + i + 1);
                double_vec.erase(double_vec.begin() + i);
                double_vec.erase(double_vec.begin() + i - 1);
                double_vec.insert(double_vec.begin() + i - 1, temp);
                size -= 2;
                i -= 2; // Decrease i by 2 to correctly handle subsequent iterations.
            }
        }
    }
    return double_vec[0];
}

/**
 * convert_string_to_double() takes a string vector and an integer representing
 * the size of that vector, and creates an equivalent double vector holding
 * values that can be used for calculations directly. This returns to
 * calculate()
*/
vector<double> convert_string_to_double(int size, vector<string> str_vec) {
    double add_flag = 85154372.17543542325;
    double suby_flag = 529816979.14614;
    double multi_flag = 340909.415216924;
    vector<double> converted_vector;

    for (int i = 0; i < size; i++) {
        if (str_vec[i] == "+") {
            converted_vector.push_back(add_flag);
        } else if (str_vec[i] == "-") {
            converted_vector.push_back(suby_flag);
        } else if (str_vec[i] == "x") {
            converted_vector.push_back(multi_flag);
        } else if (str_vec[i] == "/") {
            converted_vector.push_back(multi_flag);
            i++;
            // After using multi flag, ensure we will be multiplying by
            // 1 / (denominator) to actually perform division.
            converted_vector.push_back(1.0 / stod(str_vec[i]));
        } else {
            converted_vector.push_back(stod(str_vec[i]));
        }
    }
    return converted_vector;
}
    

};  // namespace calculator