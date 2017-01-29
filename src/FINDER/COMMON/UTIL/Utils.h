 /*****************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		Basic_Utils.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Aug 6, 2013 3:44:32 PM 
 * Description: Basic utilities
 * 
 * This file is part of Project Isagoge.
 * 
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>. 
 ****************************************************************************/

#ifndef BASIC_UTILS_H_
#define BASIC_UTILS_H_

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

#include <baseapi.h>

#include <allheaders.h> // leptonica

/****************************************
 * Some very basic helper utilities    **
 ***************************************/
namespace Utils {
  // convert integer to string
  std::string intToString(int i);

  // convert double to string with given precision (doesn't specify by default)
  std::string doubleToString(const double& d, const int& precision=0);

  // convert string to all lowercase (makes copy and returns that,
  // doesn't muck with the original
  std::string toLower(std::string str);

  // Read in an image using Leptonica, end execution with error
  // message if pixread fails
  Pix* leptReadImg(std::string fn);

  // returns the number of digits in a given integer decimal number
  int digit_count(int decnum);

  // checks to see if the string has a trailing slash, if it does
  // then returns the string as it is, otherwise put a trailing
  // slash on it at the end
  std::string checkTrailingSlash(std::string str);

  // same as above but doesn't use stl
  // doesn't modify original string (dellocation of it
  // belongs to the caller)
  char* checkStrTrailingSlash(const char* const str);

  // splits a string into a list of strings separated by the
  // given delimiter (does spaces by default)
  std::vector<std::string> stringSplit(std::string str, char delimiter=' ');

  // returns true if the string is a positive number (can take decimal point)
  bool isStrNumeric(const std::string& str);

  // takes a string and converts it to a double
  double strToDouble(const std::string& str);

  // Removes any trailing new line if it exists
  std::string chop(std::string str);

  // Removes any space or new lines
  std::string removeEmpty(std::string str);

  // Common Tesseract confidence threshold
  float getCertaintyThresh();

  // sometimes the apostrophe is mistakenly written in unicode as the
  // prime glyph which ends up being represented by the following sequence
  // of signed characters: -30 -128 -103. Checks to see if that sequence
  // ends at the given position. By default this function assumes the
  // position is strlen(str)-2, which would represent a possessive noun.
  bool checkForPrimeGlyph(const char* str, int position=-1);

  // sometimes there are extra trailing new lines that need
  // to be removed. this does just that.
  char* removeExtraNLs(char* str);

  // makes sure there's a trailing newline on the given string
  // if not adds one. if any change is made to the string the
  // old one is deleted and NULLed and a new one allocated
  char* ensureTrailingNL(char* str);

  // custom string compare (the stl version was thought to have been
  // causing seg faults prior to discovering valgrind.... I find this
  // function to be more convenient than the strcmp anyway..)
  bool stringCompare(const char* str1, const char* str2);

  // returns new string with str2 appended to str1
  // doesn't modify the original strings
  char* strAppend(char* str1, char* str2);

  // returns pointer to new string with the character at the given (zero-based) index
  // removed. deletes the old string then makes sure it points to the new one
  char* strRemoveChar(char*& str, int index);

  // doesn't modify original string, but returns newly allocated copy
  char* strCopy(const char* const str);

  inline void destroyStr(char*& str) {
    if(str != NULL) {
      delete [] str;
    }
    str = NULL;
  }

  /****************************************
   * Linux system command utilities      **
   ***************************************/
  // execute a system command (if disp false then don't display to stdout)
  std::string exec(std::string cmd, bool disp=false);

  // execute system command and display output
  void exec_display(std::string cmd);

  // count number of files in given directory
  int fileCount(std::string dir);

  // get the list of files in given directory
  std::vector<std::string> getFileList(std::string dir);

  // get the full path to a directory
  std::string getFullDirPath(std::string dir);

  // return true if directory exists
  bool existsDirectory(const std::string& dirname);

  // return true if file exists
  bool existsFile(const std::string& filename);

  std::string getHomeDir();
  std::string getTrainingRoot();
  std::string getGroundtruthRoot();

  // pulls out the name from the full path which includes an extension
  // so for instance, passing in "/home/bob/imname.jpg" would output
  // "imname".
  std::string getNameFromPath(const std::string& path);

  /*****************************************
   * Console interface utilities           *
   ****************************************/
  // return true if user answers y/Y, false if n/N
  bool promptYesNo();

  // prompts for a string value until one that doesn't match
  // the contents of the provided list is given as input
  std::string promptForValueNotOnList(
      const std::string& promptText,
      const std::vector<std::string>& list);

  // displays the vector as a matrix with labeled entries
  // for instance with row size of three:
  // [0] item1   [1] item2   [2] item3
  // [3] item4   [4] item5   [5] item6
  template <typename T> void displayVectorAsLabeledMatrix(
      std::vector<T*> vec,
      const int& rowSize) {
    std::cout << std::endl;
    // first determine the maximum string length for each column
    // and also store all the string lengths for each item in a vector
    std::vector<int> maxColLengths;
    for(int i = 0; i < rowSize; ++i)
      maxColLengths.push_back(0);
    std::vector<int> strLengths;
    {
      int i = 0;
      while(i < vec.size()) {
        for(int j = 0; j < rowSize; ++j) {
          if((i + j) < vec.size()) {
            std::stringstream buf;
            buf << *(vec[i + j]);
            int len = buf.str().size();
            if(len > maxColLengths[j]) {
              maxColLengths[j] = len;
            }
            strLengths.push_back(len);
          }
        }
        i+= rowSize;
      }
    }

    // now display the rows with the proper spacing
    int i = 0;
    while(i < vec.size()) {
      int numToShow = rowSize;
      const int numRemaining = vec.size() - i;
      if(numRemaining < rowSize) {
        numToShow = numRemaining;
      }
      for(int j = 0; j < numToShow; ++j) {
        const int cur = i + j;
        const int spacing = maxColLengths[j] - strLengths[cur];
        std::cout << "[" << Utils::intToString(cur) << "] "
            << *(vec[cur]);
        for(int k = 0; k < spacing; ++k) {
          std::cout << " ";
        }
        std::cout << " ";
      }
      std::cout << std::endl;
      i += rowSize;
    }
    std::cout << "\n";
  }

  void displayStrVectorAsLabeledMatrix(
      std::vector<std::string> vec,
      const int& rowSize);

  // prompts user to select an index from the labeled matrix
  // the selected index will be the vector index of the selected
  // item
  template <typename T> int promptSelectFromLabeledMatrix(
      const std::vector<T*>& vec,
      const int& rowSize) {
    displayVectorAsLabeledMatrix(vec, rowSize);
    int i = -1;
    while(i < 0 || i >= vec.size()) {
      std::cout << "------------------------------\nChoose an above numeric entry ["
          << intToString(0) << ":" << intToString(vec.size() - 1)
          << "] and press enter.\n------------------------------\n:-> ";
      std::cin >> i;
    }
    std::cout << std::endl;
    return i;
  }

  int promptSelectStrFromLabeledMatrix(
      const std::vector<std::string> vec,
      const int& rowSize);

  static bool intVecContains(const int& x, GenericVector<int> vec) {
    for(int i = 0; i < vec.size(); ++i) {
      if(vec[i] == x) {
        return true;
      }
    }
    return false;
  }

  template <typename T> GenericVector<int> promptMultiSelectFromLabeledMatrix(
      const std::vector<T*>& vec,
      const int& rowSize) {
    displayVectorAsLabeledMatrix(vec, rowSize);
    GenericVector<int> selections;
    while(selections.size() < 1 || selections[0] < 0
        || selections.back() >= vec.size()) {
      std::cout << "------------------------------\nChoose an above set of numeric entries delimited by spaces each in the range ["
          << intToString(0) << ":" << intToString(vec.size() - 1)
          << "] and press enter.\n------------------------------\n:-> ";
      std::string input;
      std::cin.ignore();
      std::getline(std::cin, input);
      std::vector<std::string> splitInput = Utils::stringSplit(input);
      for(int i = 0; i < splitInput.size(); ++i) {
        int inputVal = atoi(splitInput[i].c_str());
        if(!intVecContains(inputVal, selections)) {
          selections.push_back(inputVal);
          selections.sort();
        }
      }
    }
    std::cout << std::endl;
    return selections;
  }

  // wrapper around cin getline to avoid annoying /n issue.
  void getline(std::string& str);

  // wait for user input
  void waitForInput();
}

#endif /* BASIC_UTILS_H_ */
