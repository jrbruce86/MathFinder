/*
 * TrainingInfoFileParser.h
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#ifndef TRAININGINFOFILEPARSER_H_
#define TRAININGINFOFILEPARSER_H_

#include <FinderInfo.h>

#include <string>

class TrainingInfoFileParser {

 public:

  TrainingInfoFileParser();

  /**
   * Parses the training info file and returns the resulting
   * Finder information. Result memory owned by caller.
   *
   * Prints error and returns NULL if the file could not be found
   * or is corrupted.
   */
  FinderInfo* readInfoFromFile(std::string finderName);

  void writeInfoToFile(FinderInfo* const finderInfo);

  static std::string FlagDelimiter();

 private:

  void assertLineNumber(int actual, int expected, const std::string& infoFilePath);

  const std::string finderNameKey;
  const std::string descriptionKey;
  const std::string detectorNameKey;
  const std::string segmentorNameKey;
  const std::string groundtruthNameKey;
  const std::string groundtruthDirPathKey;
  const std::string groundtruthFilePathKey;
  const std::string featureExtractorsKey;
  const std::string groundtruthImagePathsKey;
};


#endif /* TRAININGINFOFILEPARSER_H_ */
