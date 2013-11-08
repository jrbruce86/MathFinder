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
#include <string.h>
#include <assert.h>


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

char* checkStrTrailingSlash(const char* const str) {
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

/*
vector<char*> lineSplit(const char* txt) {
  int txtlen = (int)strlen(txt);
  // pass 1: find split points
  vector<int> splitpoints;
  for(int i = 0; i < txtlen; i++) {
    if(txt[i] == '\n' && (i < (txtlen-1)))
      splitpoints.push_back(i);
  }
  // pass 2: iterate split points to do all the splitting
  int prevsplit = 0;
  vector<char*> res;
  if(splitpoints.empty()) {
    res.push_back((char*)txt);
    return res;
  }
  for(vector<int>::iterator it = splitpoints.begin();
      it != splitpoints.end(); it++) {
    int split = (int)(*it);
    int newstrsize = split-prevsplit;
    char* ln = new char[newstrsize+1]; // +1 for null terminator
    for(int i = 0; i < newstrsize+1; i++)
      ln[i] = txt[prevsplit+i];
    ln[newstrsize] = '\0'; // null terminator
    res.push_back(ln);
    splitpoints.clear();
    prevsplit = split;
  }
  // now just need to add the last line
  int lastsplit = prevsplit;
  int newstrsize = txtlen - prevsplit;
  char* ln = new char[newstrsize+1];
  for(int i = 0; i < newstrsize; i++)
    ln[i] = txt[prevsplit+i];
  ln[newstrsize] = '\0';
  res.push_back(ln);
  return res;
}*/

char* removeExtraNLs(char* str) {
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

char* ensureTrailingNL(char* str) {
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

bool stringCompare(const char* str1, const char* str2) {
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

char* strAppend(char* str1, char* str2) {
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

string exec(string cmd, bool disp) {
  if (!disp)
    cmd = cmd + " 2>/dev/null";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    cout << "ERROR: Unable to run linux command!\n";
    exit(EXIT_FAILURE);
  }
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


void waitForInput() {
  char c = 'a';
  char pc = 'b';
  while(c != '\n') {
    if(c == 'q')
      exit(EXIT_SUCCESS);
    if(pc != c && c != '\n')
      printf("Press enter to continue! or 'q' to quit\n");
    pc = c;
    scanf("%c", &c);
  }
}




}
