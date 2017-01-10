/*
 * main.cpp
 *
 *  Created on: Mar 16, 2013
 *      Author: Jake Bruce
 *  Description: Utility to randomly generate training data from a set of images
 *               Assumes images are all in a unix file strucutre in a single
 *               directory and named in numerical order in the format "%03d"
 *               where d is the number from 0 to total files. All are .png.
 *               Takes random samples of a chosen number of images which will
 *               be used for training data
 */

#include <string>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <set>
#include <time.h>
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
  cout << "usage: select_training <outputdir> <size>\n";
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    usage();
    exit(EXIT_FAILURE);
  }
  top_dir = chop(exec("pwd")) + "/";
  string out_dir = argv[1];
  int num = string_to_num<int>((string)argv[2]);
  int totalimages = string_to_num<int>(exec("ls *.png | wc -l"));
  cout << totalimages << endl;
  int randarr[totalimages];
  int chosentraining[num];
  for(int i = 0; i < totalimages; i++) randarr[i] = i;
  set<int> randset(randarr, randarr + totalimages);
  for(int i = 0; i < num; i++) {
    chosentraining[i] = set_pull_random(randset, 0);
  }
  char index_[10];
  exec("rm " + top_dir + "/" + out_dir + "/*.png");
  exec("mkdir /" + top_dir + "/" + out_dir);
  for(int i = 0; i < num; i++) {
    sprintf(index_, "%03d.png", chosentraining[i]);
    string nm = index_;
    exec("cp " + nm + " " + top_dir + "/" + out_dir);
  }
  return 0;
}




