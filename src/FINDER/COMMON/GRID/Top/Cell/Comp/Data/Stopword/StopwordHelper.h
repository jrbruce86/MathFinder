/*
 * StopwordFileReader.h
 *
 *  Created on: Dec 23, 2016
 *      Author: jake
 */

#ifndef STOPWORDFILEREADER_H_
#define STOPWORDFILEREADER_H_

#include <string>
#include <baseapi.h>
#include <locale.h>

class StopwordFileReader {

 public:

  StopwordFileReader();

  GenericVector<std::string> getStopwords();

  bool isStopWord(std::string word);

 private:

  GenericVector<std::string> stopwords;
};


#endif /* STOPWORDFILEREADER_H_ */
