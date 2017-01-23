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

#include <Utils.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <baseapi.h>
#include <locale.h>

// convert integer to string
std::string Utils::intToString(int i) {
  char buf[digit_count(i)];
  sprintf(buf, "%d", i);
  return (std::string) buf;
}

static std::locale l;
// convert string to all lowercase (makes copy and returns that,
// doesn't muck with the original
std::string Utils::toLower(std::string str) {
  for(int i = 0; i < str.size(); ++i) {
    str[i] = std::tolower(str[i], l);
  }
  return str;
}

// Read in an image using Leptonica, end execution with error
// message if pixread fails
Pix* Utils::leptReadImg(std::string fn) {
  Pix* img = pixRead(fn.c_str());
  if (img == NULL) {
    std::cout << "ERROR: Could not open " << fn << std::endl;
    assert(false);
  }
  return img;
}

// returns the number of digits in a given integer decimal number
int Utils::digit_count(int decnum) {
  int numdigits = 0;
  double ddecnum = (double) decnum;
  while (floor(ddecnum) != 0) {
    ddecnum /= (double) 10;
    numdigits++;
  }
  return numdigits;
}

std::string Utils::checkTrailingSlash(std::string str) {
  if (str.at(str.length() - 1) != '/')
    str += "/";
  return str;
}

char* Utils::checkStrTrailingSlash(const char* const str) {
  if(str == NULL)
    return NULL;
  int len = strlen(str);
  if(str[len-1] != '\n') {
    int newlen = len+1;
    char* newstr = new char[newlen+1];
    for(int i = 0; i < len; i++)
      newstr[i] = str[i];
    assert(len == (newlen-1));
    newstr[len] = '/';
    newstr[newlen] = '\0';
    return newstr;
  }
  else
    return (char*)str;
}

std::vector<std::string> Utils::stringSplit(std::string str, char delimiter) {
  std::vector<std::string> stringlist;
  std::vector<int> splitlocations;
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
    stringlist.push_back((std::string) "");
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
        // on the last split get last string up till the end
        curindex++;
        cursplit = str.length();
      }
    }
  }
  // Don't really care about performance and am strapped for time.. so yeah..
  for(int i = stringlist.size() - 1; i > -1; --i) {
    if(stringlist[i].size() == 0) {
      stringlist.erase(stringlist.begin() + i); // delete empties
    }
  }
  return stringlist;
}

bool Utils::isStrNumeric(const std::string& str) {
  bool isnumber = true;
  if(str.empty())
    return false;
  int decimalpoint = 0;
  for(int i = 0; i < str.length(); i++) {
    if(!isdigit(str.at(i))) {
      if(str.at(i) == '.') {
        if(++decimalpoint > 1) {
          isnumber = false;
          break;
        }
      }
      else {
        isnumber = false;
        break;
      }
    }
  }
  return isnumber;
}

double Utils::strToDouble(const std::string& str) {
  std::istringstream i(str);
  double x;
  if (!(i >> x))
    return 0;
  return x;
}


bool Utils::checkForPrimeGlyph(const char* str, int position) {
  if(position < 0)
    position = strlen(str) - 2;
  assert(position > -1 && position < strlen(str));
  const int seq_size = 3;
  const char glyph_seq[] = {-30, -128, -103};
  for(int i = 0; i < seq_size; ++i) {
    const int curpos = position - i;
    if(str[curpos] != glyph_seq[seq_size - (i+1)])
      return false;
  }
  return true;
}

char* Utils::removeExtraNLs(char* str) {
  if(!str)
    return NULL;
  int end = (int)strlen(str);
  if(str[end-1] != '\n')
    return str;
  int toremove = 0;
  for(int i = end-2; i > 0; i--) {
    if(str[i] == '\n')
      toremove++;
    else
      break;
  }
  if(toremove == 0)
    return str;
  char* newstr = new char[end-toremove+1];
  for(int i = 0; i < (end-toremove); i++)
    newstr[i] = str[i];
  newstr[end-toremove] = '\0';
  delete [] str;
  str = NULL;
  return newstr;
}

char* Utils::ensureTrailingNL(char* str) {
  if(!str)
    return NULL;
  const int end = (int)strlen(str);
  if(str[end-1] == '\n')
    return str;
  // theres no new line, need to make new string
  const int newend = end + 1; // (+1 for newline)
  char* newstr = new char[newend+1]; // allocate (+1 for null terminator)
  for(int i = 0; i < end; i++) // copy everything over
    newstr[i] = str[i];
  assert(end == (newend-1));
  newstr[newend-1] = '\n'; // add the new line
  newstr[newend] = '\0'; // null terminator, done
  delete [] str; // get rid of old one
  str = NULL;
  return newstr;
}

bool Utils::stringCompare(const char* str1, const char* str2) {
  if(str1 == NULL || str2 == NULL)
    return false;
  if(strlen(str1) != strlen(str2))
    return false;
  for(int i = 0; i < strlen(str1); ++i) {
    if(str1[i] != str2[i])
      return false;
  }
  return true;
}

char* Utils::strAppend(char* str1, char* str2) {
  int str1_len = strlen(str1);
  int str2_len = strlen(str2);
  int total_len = str1_len + str2_len;
  char* newstr = new char[total_len+1]; // +1 for null terminator
  for(int i = 0; i < str1_len; i++)
    newstr[i] = str1[i];
  for(int i = 0; i < str2_len; i++)
    newstr[str1_len+i] = str2[i];
  newstr[total_len] = '\0';
  return newstr;
}

std::string Utils::chop(std::string str) {
  const int last = str.size() - 1;
  if(str.at(last) == '\n') {
    str.erase(last);
  }
  return str;
}

float Utils::getCertaintyThresh() {
  return -5;
}

char* Utils::strRemoveChar(char*& str, int index) {
  if(str == NULL)
    return NULL;
  int strlen_ = strlen(str);
  if(index > strlen_ || index < 0) {
    std::cout << "ERROR: Attempted removal of character outside string bounds\n";
    exit(EXIT_FAILURE);
  }
  int newlen_ = strlen_ - 1;
  char* newstr = NULL;
  if(newlen_ > 0) {
    newstr = new char[newlen_ + 1]; // + 1 for NULL terminator
    int j = 0;
    for(int i = 0; i < strlen_; i++) {
      if(i != index) {
        newstr[j] = str[i];
        j++;
      }
    }
    newstr[newlen_] = '\0';
  }
  delete [] str;
  str = newstr;
  return newstr;
}

char* Utils::strCopy(const char* const str) {
  int len_ = strlen(str);
  char* cpy = new char[len_ + 1]; // + 1 for NULL terminator
  for(int i = 0; i < len_; i++)
    cpy[i] = str[i];
  cpy[len_] = '\0';
  return cpy;
}

std::string Utils::exec(std::string cmd, bool disp) {
  if (!disp)
    cmd = cmd + " 2>/dev/null";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    std::cout << "ERROR: Unable to run linux command!\n";
    std::cout << "Command that failed is: " << cmd << std::endl;
    exit(EXIT_FAILURE);
  }
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

// execute system command and display output
void Utils::exec_display(std::string cmd) {
  std::cout << cmd << std::endl;
  std::string res = exec(cmd);
  if (!res.empty())
    std::cout << "% " << res << std::endl;
}

// count number of files in given directory
int Utils::fileCount(std::string dir) {
  std::string res = exec("ls " + dir);
  int count = 0;
  if (!(res.empty() || res.size() < 3)) {
    if (res.substr(3) != "ls:") {
      for (unsigned int i = 0; i < res.size(); i++) {
        if (10 == res[i])
          count++;
      }
    }
  }
  return count;
}

std::vector<std::string> Utils::getFileList(std::string dir) {
  const std::string res = exec("ls " + dir);
  if(!(res.empty() || res.size() < 3)) {
    if(res.substr(3) != "ls:") {
      return stringSplit(res, 10);
    }
  }
  return std::vector<std::string>();
}

std::string Utils::getFullDirPath(std::string dir) {
  return exec((std::string)"cd " + dir + (std::string)"; pwd");
}

bool Utils::existsDirectory(const std::string& dirname) {
  std::string exists = exec(
      (std::string) "if test -d " + dirname + (std::string) "; then echo 'exist'; fi");
  if (exists == (std::string) "exist\n")
    return true;
  return false;
}

bool Utils::promptYesNo() {
  std::string input = "";
  while(input != "y" && input != "Y"
      && input != "n" && input != "N") {
      std::cout << "(y/n)? ";
    std::cin >> input;
    std::cout << std::endl;
  }
  if(input == "y" || input == "Y") {
    return true;
  }
  return false;
}

std::string Utils::promptForValueNotOnList(
    const std::string& promptText,
    const std::vector<std::string>& list) {
  std::string enteredString = "";
  bool enteredStringExists = true;
  while(enteredStringExists) {
    std::cout << promptText << " ";
    std::cin >> enteredString;
    bool found = false;
    for(int i = 0; i < list.size(); ++i) {
      if(list[i] == enteredString) {
        found = true;
        enteredString = "";
        std::cout << "The value you entered is already in use. "
            << "Choose a different one.\n";
        break;
      }
    }
    enteredStringExists = found;
  }
  return enteredString;
}

void Utils::displayStrVectorAsLabeledMatrix(
    std::vector<std::string> vec,
    const int& rowSize) {
  std::vector<std::string*> strPointerVec;
  for(int i = 0; i < vec.size(); ++i) {
    strPointerVec.push_back(&vec[i]);
  }
  displayVectorAsLabeledMatrix(strPointerVec, rowSize);
}

int Utils::promptSelectStrFromLabeledMatrix(
    std::vector<std::string> vec,
    const int& rowSize) {
  std::vector<std::string*> strPointerVec;
  for(int i = 0; i < vec.size(); ++i) {
    strPointerVec.push_back(&vec[i]);
  }
  return promptSelectFromLabeledMatrix(strPointerVec, rowSize);
}

bool Utils::existsFile(const std::string& filename) {
  std::string exists = exec(
      (std::string) "if test -e " + filename + (std::string) "; then echo 'exist'; fi");
  if (exists == (std::string) "exist\n")
    return true;
  return false;
}

std::string Utils::getHomeDir() {
  return chop(exec("echo ~"));
}

std::string Utils::getTrainingRoot() {
  return checkTrailingSlash(getHomeDir()) + std::string(".mathfinder/training/");
}

std::string Utils::getGroundtruthRoot() {
  return checkTrailingSlash(getHomeDir()) + std::string(".mathfinder/groundtruth/");
}

std::string Utils::getNameFromPath(const std::string& path) {
  const std::string::size_type dotIndex = path.find_last_of(".");
  const std::string::size_type slashIndex = path.find_last_of("/");
  if(slashIndex != std::string::npos) {
    if(dotIndex > slashIndex && dotIndex != std::string::npos) {
      return path.substr(slashIndex + 1, dotIndex - slashIndex - 1);
    } else {
      return path.substr(slashIndex + 1);
    }
  }
  return path;
}

// wrapper around cin getline to avoid annoying \n issue.
void Utils::getline(std::string& str) {
  while((char)(std::cin.peek()) == '\n') {
    std::cin.ignore();
  }
  getline(std::cin, str);
}


void Utils::waitForInput() {
  std::cout << "Press enter to continue! or 'q' to quit\n";
  if((char)std::cin.peek() == '\n') {
    std::cin.ignore();
  }
  char c = 'a';
  char pc = 'b';
  do {
    if(c == 'q')
      exit(EXIT_SUCCESS);
    if(pc != c && c != '\n')
      std::cout << "Press enter to continue! or 'q' to quit\n";
    pc = c;
    c = std::getchar();
  } while(c != '\n');
}

