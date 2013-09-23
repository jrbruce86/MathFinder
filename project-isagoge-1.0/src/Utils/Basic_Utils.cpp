/*****************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		Basic_Utils.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Aug 6, 2013 3:45:41 PM 
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

#include "Basic_Utils.h"

namespace Basic_Utils {
// convert integer to string
string intToString(int i) {
  char buf[digit_count(i)];
  sprintf(buf, "%d", i);
  return (string) buf;
}

// Read in an image using Leptonica, end execution with error
// message if pixread fails
Pix* leptReadImg(string fn) {
  Pix* img = pixRead(fn.c_str());
  if (img == NULL) {
    cout << "ERROR: Could not open " << fn << endl;
    exit (EXIT_FAILURE);
  }
  return img;
}

// returns the number of digits in a given integer decimal number
int digit_count(int decnum) {
  int numdigits = 0;
  double ddecnum = (double) decnum;
  while (floor(ddecnum) != 0) {
    ddecnum /= (double) 10;
    numdigits++;
  }
  return numdigits;
}

string checkTrailingSlash(string str) {
  if (str.at(str.length() - 1) != '/')
    str += "/";
  return str;
}

vector<string> stringSplit(string str, char delimiter) {
  vector<string> stringlist;
  vector<int> splitlocations;
  for (int i = 0; i < str.length(); i++) {
    if (str.at(i) == delimiter)
      splitlocations.push_back(i);
  }
  if (splitlocations.empty()) {
    stringlist.push_back(str);
    return stringlist;
  }
  int curindex = 0;
  int cursplit = splitlocations[curindex];
  int numsplits = splitlocations.size();
  for (int i = 0; i < (numsplits + 1); i++) {
    stringlist.push_back((string) "");
  }
  for (int i = 0; i < str.length(); i++) {
    if (i < cursplit)
      stringlist[curindex].append(1, str.at(i));
    else {
      if ((curindex + 1) < numsplits) {
        // have more splitting to do
        curindex++;
        cursplit = splitlocations[curindex];
      } else {
        // on the last split get last string is up till the end
        curindex++;
        cursplit = str.length();
      }
    }
  }
  return stringlist;
}

string exec(string cmd, bool disp) {
  if (!disp)
    cmd = cmd + " 2>/dev/null";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "ERROR";
  char buffer[128];
  string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

// execute system command and display output
void exec_display(string cmd) {
  string res = exec(cmd);
  if (!res.empty())
    cout << "% " << res << endl;
}

// count number of files in given directory
int fileCount(string dir) {
  string res = exec("ls " + dir);
  int count = 0;
  if (!res.empty()) {
    if (res.substr(3) != "ls:") {
      for (unsigned int i = 0; i < res.size(); i++) {
        if (10 == res[i])
          count++;
      }
    }
  }
  return count;
}

bool existsDirectory(const string& dirname) {
  string exists = exec(
      (string) "if test -d " + dirname + (string) "; then echo 'exist'; fi");
  if (exists == (string) "exist\n")
    return true;
  return false;
}

bool existsFile(const string& filename) {
  string exists = exec(
      (string) "if test -e " + filename + (string) "; then echo 'exist'; fi");
  if (exists == (string) "exist\n")
    return true;
  return false;
}

}
