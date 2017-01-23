/*
 * StopwordFileReader.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: jake
 */

#include <StopwordHelper.h>
#include <Utils.h>

#include <iostream>
#include <assert.h>

StopwordFileReader::StopwordFileReader()
: stopwords(GenericVector<std::string>()) {
}

GenericVector<std::string> StopwordFileReader::getStopwords() {
  if(stopwords.size() > 0) {
    return stopwords;
  }
  // Otherwise read in the file, update internal value, and return result
  std::string stopwordFileName = Utils::getTrainingRoot() +
      (std::string)"stopwords";

  std::ifstream stpwrdfs;
  stpwrdfs.open(stopwordFileName.c_str());
  if(!stpwrdfs.is_open()) {
    std::cout << "ERROR: Could not open the stopword file at " << stopwordFileName
        << std::endl << "Check to make sure the stopword file is located at "
        << stopwordFileName << ". If not you may need to reinstall." << std::endl;
    assert(false);
  }
  std::string line;
  while(getline(stpwrdfs, line)) {
    // should only be one word per line
    for(int i = 0; i < line.size(); ++i) {
      if(line[i] == ' ') {
        std::cout << "ERROR: more than one stopword detected on a line in stopword file!\n";
        assert(false);
      }
    }
    stopwords.push_back(line);
  }
  return stopwords;
}

bool StopwordFileReader::isStopWord(std::string word) {
  stopwords = getStopwords();
  for(int i = 0; i < stopwords.length(); ++i) {
    if(Utils::toLower(word) == Utils::toLower(stopwords[i])) {
      return true;
    }
  }
  return false;
}


