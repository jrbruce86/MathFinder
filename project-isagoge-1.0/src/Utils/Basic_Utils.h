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

  // splits a string into a list of strings separated by the
  // given delimiter (does spaces by default)
  vector<string> stringSplit(string str, char delimiter=' ');


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
}

#endif /* BASIC_UTILS_H_ */
