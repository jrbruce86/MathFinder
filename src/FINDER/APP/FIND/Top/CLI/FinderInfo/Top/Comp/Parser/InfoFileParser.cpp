/*
 * TrainingInfoFileParser.cpp
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#include <InfoFileParser.h>

#include <FinderTrainingPaths.h>
#include <FTPathsFactory.h>
#include <Utils.h>

#include <iostream>
#include <string>
#include <assert.h>

TrainingInfoFileParser::TrainingInfoFileParser()
: finderNameKey("Finder name"),
  descriptionKey("Description"),
  detectorNameKey("Detector"),
  segmentorNameKey("Segmentor"),
  groundtruthNameKey("GroundtruthName"),
  groundtruthDirPathKey("GroundtruthDirPath"),
  groundtruthFilePathKey("GroundtruthFilePath"),
  featureExtractorsKey("Feature extractors"),
  groundtruthImagePathsKey("GroundtruthImagePaths") {}

bool TrainingInfoFileParser::writeInfoToFile(FinderInfo* const finderInfo) {

  // Create the directories indicated by file info
  //(except the groundtruth one, which should already be created)
  FinderTrainingPaths* const finderTrainingPaths = finderInfo->getFinderTrainingPaths();
  Utils::exec_display("mkdir -p " + finderTrainingPaths->getFeatureExtDirPath());
  Utils::exec_display("mkdir -p " + finderTrainingPaths->getDetectorDirPath());
  Utils::exec_display("mkdir -p " + finderTrainingPaths->getSegmentorDirPath());


  if(!Utils::existsDirectory(finderTrainingPaths->getTrainingDirPath())) {
    Utils::exec_display("mkdir -p " + finderTrainingPaths->getTrainingDirPath());
  }
  if(!Utils::existsDirectory(finderInfo->getGroundtruthDirPath())) {
    Utils::exec_display("mkdir -p " + finderInfo->getGroundtruthDirPath());
  }

  // Write the info to the file
  std::ofstream outStream;
  outStream.open(finderTrainingPaths->getInfoFilePath().c_str(),
      std::ofstream::out);
  if(!outStream.is_open()) {
    std::cout << "ERROR: Could not open file at " << finderTrainingPaths->getInfoFilePath() << std::endl;
    return false;
  }
  outStream << finderNameKey << ": " << finderInfo->getFinderName() << std::endl;
  outStream << descriptionKey << ": " << finderInfo->getDescription() << std::endl;
  outStream << detectorNameKey << ": " << finderInfo->getDetectorName() << std::endl;
  outStream << segmentorNameKey << ": " << finderInfo->getSegmentorName() << std::endl;
  outStream << groundtruthNameKey << ": " << finderInfo->getGroundtruthName() << std::endl;
  outStream << groundtruthDirPathKey << ": " << finderInfo->getGroundtruthDirPath() << std::endl;
  outStream << groundtruthFilePathKey << ": " << finderInfo->getGroundtruthFilePath() << std::endl;
  outStream << groundtruthImagePathsKey << ": ";
  for(int i = 0; i < finderInfo->getGroundtruthImagePaths().size(); ++i) {
    outStream << finderInfo->getGroundtruthImagePaths()[i] << " ";
  }
  outStream << std::endl;
  outStream << featureExtractorsKey << ": ";
  for(int i = 0; i < finderInfo->getFeatureExtractorUniqueNames().size(); ++i) {
    outStream << finderInfo->getFeatureExtractorUniqueNames()[i] << " ";
  }
  outStream << std::endl;
  outStream.close();
  return true;
}

/**
 * Parses the training info file and returns the resulting
 * Finder information. Result memory owned by caller.
 *
 * Prints error and/or asserts false if the file could not be found
 * or is corrupted.
 */
FinderInfo* TrainingInfoFileParser::readInfoFromFile(std::string finderName) {

  FinderTrainingPaths* const finderTrainingPaths =
      FinderTrainingPathsFactory().createFinderTrainingPaths(finderName);

  // Verify that the training paths exist
  assert(finderTrainingPaths->allRequiredExist());

  std::string infoFilePath = finderTrainingPaths->getInfoFilePath();
  FinderInfoBuilder builder;

  // Read in and parse the file at the above path
  std::ifstream infoFile;
  infoFile.open(infoFilePath.c_str());
  if(!infoFile.is_open()) {
    std::cout << "ERROR: Unable to open file at " << infoFilePath << std::endl;
    assert(false);
  }
  std::string line;
  int lineNumber = 0;
  while(getline(infoFile, line)) {
    std::string key = line.substr(0, line.find(":"));
    std::string value = line.substr(line.find(":") + 1);

    if(key == finderNameKey) {
      assertLineNumber(lineNumber++, 0, infoFilePath);
      assert(finderName == value);
      builder.setFinderName(finderName);
    } else if(key == descriptionKey) {
      assertLineNumber(lineNumber++, 1, infoFilePath);
      builder.setDescription(value);
    } else if(key == detectorNameKey) {
      assertLineNumber(lineNumber++, 2, infoFilePath);
      builder.setDetectorName(value);
    } else if(key == segmentorNameKey) {
      assertLineNumber(lineNumber++, 3, infoFilePath);
      builder.setSegmentorName(value);
    } else if(key == groundtruthNameKey) {
      assertLineNumber(lineNumber++, 4, infoFilePath);
      builder.setGroundtruthName(value);
    } else if(key == groundtruthDirPathKey) {
      assertLineNumber(lineNumber++, 5, infoFilePath);
      builder.setGroundtruthDirPath(value);
    } else if(key == groundtruthFilePathKey) {
      assertLineNumber(lineNumber++, 6, infoFilePath);
      builder.setGroundtruthFilePath(value);
    } else if(key == groundtruthImagePathsKey) {
      assertLineNumber(lineNumber++, 7, infoFilePath);
      builder.setGroundtruthImagePaths(Utils::stringSplit(value, ' '));
    } else if(key == featureExtractorsKey) {
      assertLineNumber(lineNumber++, 8, infoFilePath);
      builder.setFeatureExtractorUniqueNames(Utils::stringSplit(value, ' '));
    } else {
      std::cout << "ERROR: Unexpected input from file at " << infoFilePath << std::endl;
      assert(false);
    }

  }

  return builder.build();
}

std::string TrainingInfoFileParser::FlagDelimiter() {
  return "_flag_";
}

void TrainingInfoFileParser::assertLineNumber(int actual, int expected, const std::string& infoFilePath) {
  if(actual != expected) {
    std::cout << "ERROR: the training info file at " << infoFilePath << " appears to be corrupted.\n";
    assert(false);
  }
}
