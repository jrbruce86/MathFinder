/*
 * FinderTrainingPaths.cpp
 *
 *  Created on: Dec 9, 2016
 *      Author: jake
 */
#include <FinderTrainingPaths.h>

#include <Utils.h>

#include <string>
#include <iostream>

FinderTrainingPaths::FinderTrainingPaths(
    const std::string trainingDirPath,
    const std::string infoFilePath,
    const std::string detectorDirPath,
    const std::string segmentorDirPath,
    const std::string featureExtDirPath,
    const std::string sampleFilePath) {
  this->trainingDirPath = trainingDirPath;
  this->infoFilePath = infoFilePath;
  this->detectorDirPath = detectorDirPath;
  this->segmentorDirPath = segmentorDirPath;
  this->featureExtDirPath = featureExtDirPath;
  this->sampleFilePath = sampleFilePath;
}

bool FinderTrainingPaths::allRequiredExist() {
  if(!Utils::existsFile(infoFilePath)) {
    return printPathMissingError(infoFilePath);
  }
  if(!Utils::existsDirectory(trainingDirPath)) {
    return printPathMissingError(trainingDirPath);
  }
  if(!Utils::existsDirectory(detectorDirPath)) {
    return printPathMissingError(detectorDirPath);
  }
  if(!Utils::existsDirectory(segmentorDirPath)) {
    return printPathMissingError(segmentorDirPath);
  }
  if(!Utils::existsDirectory(featureExtDirPath)) {
    return printPathMissingError(featureExtDirPath);
  }
  return true;
}

bool FinderTrainingPaths::printPathMissingError(const std::string& path) {
  std::cout << "ERROR: The required path at " << path << " does not exist.\n";
  return false;
}

std::string FinderTrainingPaths::getGroundtruthRoot() {
  return Utils::checkTrailingSlash(Utils::getHomeDir()) + ".mathfinder/groundtruth/";
}
std::string FinderTrainingPaths::getTrainingRoot() {
  return Utils::checkTrailingSlash(Utils::getHomeDir()) + ".mathfinder/training/";
}

std::string FinderTrainingPaths::getTrainedFinderRoot() {
  return Utils::checkTrailingSlash(getTrainingRoot()) + "Finders/";
}

/**
 * Path to the root directory of this Finder's training data
 */
std::string FinderTrainingPaths::getTrainingDirPath() {
  return trainingDirPath;
}

/**
 * Path to the file containing generic info about the specific math finder.
 */
std::string FinderTrainingPaths::getInfoFilePath() {
  return infoFilePath;
}

/**
 * Path to directory containing any training resources needed for detection
 * if applicable.
 */
std::string FinderTrainingPaths::getDetectorDirPath() {
  return detectorDirPath;
}

/**
 * Path to directory containing any training resources needed for segmentation
 * if applicable.
 */
std::string FinderTrainingPaths::getSegmentorDirPath() {
  return segmentorDirPath;
}

/**
 * Path to directory containing any resources needed for feature extraction
 * if applicable.
 */
std::string FinderTrainingPaths::getFeatureExtDirPath() {
  return featureExtDirPath;
}

/**
 * Path to a file generated during training to store the samples so they could
 * be read in on subsequent trainings to save time.
 */
std::string FinderTrainingPaths::getSampleFilePath() {
  return sampleFilePath;
}
