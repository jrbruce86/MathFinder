 /*****************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		Basic_Utils.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Aug 6, 2013 3:44:32 PM 
 * Description: TODO
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

using namespace std;

#include <allheaders.h> // leptonica

/****************************************
 * Some very basic helper utilities    **
 ***************************************/
namespace Basic_Utils {
  // convert integer to string
  string intToString(int i);

  // Read in an image using Leptonica, end execution with error
  // message if pixread fails
  Pix* leptReadImg(string fn);

  // returns the number of digits in a given integer decimal number
  int digit_count(int decnum);

  // checks to see if the string has a trailing slash, if it does
  // then returns the string as it is, otherwise put a trailing
  // slash on it at the end
  string checkTrailingSlash(string str);

  // same as above but doesn't use stl
  // doesn't modify original string (dellocation of it
  // belongs to the caller)
  char* checkStrTrailingSlash(const char* const str);

  // splits a string into a list of strings separated by the
  // given delimiter (does spaces by default)
  vector<string> stringSplit(string str, char delimiter=' ');

  // returns true if the string is a positive number (can take decimal point)
  bool isStrNumeric(const string& str);

  // takes a string and converts it to a double
  double strToDouble(const string& str);

  // splits txt into separate lines
  // (TAKEN OUT BECAUSE IT'S BUGGY... SEE M_UTILS.H FOR REPLACEMENT!!!)
  //vector<char*> lineSplit(const char* txt);

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
  string exec(string cmd, bool disp=false);

  // execute system command and display output
  void exec_display(string cmd);

  // count number of files in given directory
  int fileCount(string dir);

  // return true if directory exists
  bool existsDirectory(const string& dirname);

  // return true if file exists
  bool existsFile(const string& filename);




  // Debugging:::::
  void waitForInput();
}

#endif /* BASIC_UTILS_H_ */
