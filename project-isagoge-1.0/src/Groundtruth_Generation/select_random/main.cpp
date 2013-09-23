/*
 * main.cpp
 *
 *  Created on: Mar 16, 2013
 *      Author: Jake Bruce
 *  Purpose:
 *  	When supplied with a unix-based directory consisting of
 *  	one or more pdf documents and their corresponding .png images
 *  	(each .png image being numbered from 0 in the same order as
 *  	given in the pdf document and having the same name as the pdf
 *  	document in the format <pdfname>_%05d), randomly choose a specified
 *  	number of images (pages) and copy them into a specified directory.
 */

#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <set>
using namespace std;

string top_dir;

// cuts off everything past the last new line
string chop(string str) {
  string::iterator it;
  int index = str.length() - 1;
  for (it = str.end() ; it > str.begin(); it-- ,index--) {
    if(str.at(index) == '\n') {
      str.erase(index);
      break;
    }
  }
  return str;
}

template <typename T>
T string_to_num(string str) {
  stringstream s(str);
  T res;
  return s >> res ? res : 0;
}

void back_to_top() {
  if(chdir(top_dir.c_str()) == -1) {
    cout << "ERROR: Could not open directory, " << top_dir << endl;
    exit(EXIT_FAILURE);
  }
}

void cd(string dir) {
  back_to_top();
  if(chdir(dir.c_str()) == -1) {
    cout << "ERROR: Could not open directory, " << dir << endl;
    exit(EXIT_FAILURE);
  }
}


int random_num(int low, int high) {
  int range = high - low;
  srand(time(NULL));
  int rand_ = rand() % range;
  // add the offset back in so its btwn low and high
  return rand_ + low;
}

int set_find_closest(set<int> s, int val) {
  if(s.empty())
    return -1;
  if(s.count(val))
    return val;
  set<int>::iterator iterator;
  int prev = -1, cur = -1;
  for(iterator = s.begin(); iterator != s.end(); iterator++) {
    cur = *iterator;
    if((prev < val) && (cur > val) && (cur != prev) && (prev != -1)) {
      return ((cur - val) > (val - prev)) ? prev : cur;
    }
    prev = cur;
  }
  return -1;
}

int set_pull_random(set<int>& s, int offset) {
  int setsize = s.size();
  int rand = random_num(offset, setsize);
  rand += *s.begin(); // offset by lowest value in set
  int closest = set_find_closest(s, rand);
  if(closest == -1) {
    cout << "ERROR in set_find_closest given the value " << rand << endl;
    exit(EXIT_FAILURE);
  }
  s.erase(closest);
  return closest;
}


string exec(const string cmd) {
  FILE* stdout = popen(cmd.c_str(), "r");
  const int bytenum = 32; // bytes per chunk
  if(!stdout) {
    cout << "ERROR executing subprocess\n";
    exit(EXIT_FAILURE);
  }
  string output = "";
  char chunk[bytenum];
  while(!feof(stdout)) {
    if(fgets(chunk, bytenum, stdout))
      output += chunk;
    else if(!feof(stdout)) {
      cout << "ERROR reading from stdout\n";
      exit(EXIT_FAILURE);
    }
  }
  pclose(stdout);
  return output;
}

void exec_display(string cmd) {
  string output = exec(cmd);
  cout << output << endl;
  return;
}

void usage() {
  cout << "\nusage: select_random <src_dir> <pdf-name> <num> <out_dir> <offset> \n\n\
src_dir:  a directory consisting of one or more pdf documents, and \n\
          their corresponding images numbered from 0 in the same order \n\
          as given in the pdf document formatted as <pdfname>_%05d.\n\
          images must all be in png format!!No trailing slashes!!\n\
pdf-name: the name of the pdf document described in src_dir description\n\
          without the .pdf extension (DON'T include .pdf extension. just\n\
          need the name that comes before the extension\n\
num:      the number of pages to randomly select.\n\
out_dir:  the directory to output the randomly selected pages along with \n\
          their pdf. No trailing slashes!!!\n\
offset:   (optional) the number of first pages to disregard (title pages)\n\n";
}

int main(int argc, char* argv[]) {
  if(argc < 5 || argc > 6) {
    usage();
    exit(EXIT_FAILURE);
  }

  top_dir = chop(exec("pwd")) + "/";
  const string src_dir = argv[1];
  const string pdf_name = argv[2];
  const int num = string_to_num<int>((string)argv[3]);
  const string out_dir = argv[4];
  int offset = 0;
  if(argc == 6)
    offset = string_to_num<int>((string)argv[5]);
  // change to the src directory
  cd(src_dir);

  // count the total number of png's in the directory
  int tpagenum = string_to_num<int>(exec("ls " + pdf_name + "*.png | wc -l"));

  // if there are fewer pages available than the number requested to randomly
  // select then just need to copy all the files into the destination
  if(tpagenum < num) {
    cout << "Copying all " << tpagenum << " of the " << src_dir << " images into" \
        << out_dir << " along with the pdf and info file.\n";
    exec_display("cp " + pdf_name + "*" + " " + out_dir);
    exit(EXIT_SUCCESS);
  }

  // only want one copy of each random page
  // thus initialize efficient data structure
  // to keep track of the index of each copy
  // that hasn't been used yet
  int nums[tpagenum];
  for(int i = 0; i < tpagenum; i++) nums[i] = i;
  set<int> availablenums(nums, nums+tpagenum);
  int randompages[num];
  for(int i = 0; i < num; i++) {
    randompages[i] = set_pull_random(availablenums, offset);
  }

  // delete any pre-existing copies in the destination
  cd(out_dir);
  exec("rm " + pdf_name + "*");

  // copy all the randomly selected folders
  // to their destination
  back_to_top();
  char index_[10];
  for(int i = 0; i < num; i++) {
    sprintf(index_, "_%05d.png", randompages[i]);
    exec("cp " + top_dir + "/" + src_dir + "/" + pdf_name + index_ + " " \
        + top_dir + "/" + out_dir);
  }
  exec("cp " + pdf_name + ".pdf" + " " + pdf_name + "_info " \
      + out_dir);


  return 0;
}
