/*
 * FinderInfo.cpp
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#include <FinderInfo.h>

#include <MathExpressionFinder.h>
#include <MFinderProvider.h>
#include <FinderTrainingPaths.h>
#include <FTPathsFactory.h>
#include <BlobFeatExt.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>
#include <GeometryCat.h>
#include <RecCat.h>

#include <stddef.h>
#include <assert.h>
#include <string>
#include <vector>

FinderInfo::FinderInfo(
    std::string finderName,
    FinderTrainingPaths* finderTrainingPaths,
    std::vector<std::string> featureExtractorUniqueNames,
    std::string detectorName,
    std::string segmentorName,
    std::string groundtruthName,
    std::string groundtruthDirPath,
    std::string groundtruthFileName,
    std::string description,
    std::vector<std::string> groundtruthImagePaths)
: finderTrainingPaths(NULL), finderName(""), detectorName(""),
  segmentorName(""), groundtruthName(""), groundtruthDirPath(""),
  groundtruthFilePath(""), description("") {
  this->finderName = finderName;
  this->finderTrainingPaths = finderTrainingPaths;
  this->featureExtractorUniqueNames = featureExtractorUniqueNames;
  this->detectorName = detectorName;
  this->segmentorName = segmentorName;
  this->groundtruthName = groundtruthName;
  this->groundtruthDirPath = groundtruthDirPath;
  this->groundtruthFilePath = groundtruthFileName;
  this->description = description;
  this->groundtruthImagePaths = groundtruthImagePaths;
}

/**
 * The finder name holds a user-defined name for distinguishing
 * a feature/detector/segmentor combo from others. For instance
 * one user-defined combo may include an SVM detector 8 features,
 * and a heuristic segmentation algorithm. Another may be the same
 * SVM detector but this time using a subset of those features and
 * a trained classifier-based approach to segmentation. A simple name
 * chosen by the user is probably the least confusing way of
 * distinguishing such combinations from one another.
 */
std::string FinderInfo::getFinderName() {
  return finderName;
}

/**
 * The paths to training directories relevant to this Finder
 */
FinderTrainingPaths* FinderInfo::getFinderTrainingPaths() {
  return finderTrainingPaths;
}

std::vector<std::string> FinderInfo::getFeatureExtractorUniqueNames() {
  return featureExtractorUniqueNames;
}

std::string FinderInfo::getDetectorName() {
  return detectorName;
}

std::string FinderInfo::getSegmentorName() {
  return segmentorName;
}

/**
 * Name of the dataset used to train this Finder.
 */
std::string FinderInfo::getGroundtruthName() {
  return groundtruthName;
}

/**
 * Path to the directory containing the groundtruth that was used to train the
 * detector and/or segmentor of this Finder if applicable. The groundtruth
 * consists of the images and their associated box file containing the bounding
 * boxes of the math expressions.
 */
std::string FinderInfo::getGroundtruthDirPath() {
  return groundtruthDirPath;
}

/**
 * The name of the ".dat" file containing the bounding boxes of the
 * groundtruth math expression regions in each image contained in the
 * dataset used for training.
 */
std::string FinderInfo::getGroundtruthFilePath() {
  return groundtruthFilePath;
}

/**
 * Optional description for providing further details about the Finder
 */
std::string FinderInfo::getDescription() {
  return description;
}

std::vector<std::string> FinderInfo::getGroundtruthImagePaths() {
  return groundtruthImagePaths;
}

void FinderInfo::displayInformation() {
  // Temporarily create the math finder so I can get the info from it
  GeometryBasedExtractorCategory spatialCategory;
  RecognitionBasedExtractorCategory recognitionCategory;
  MathExpressionFinder* mathFinder = MathExpressionFinderProvider().createMathExpressionFinder(
      &spatialCategory,
      &recognitionCategory,
      this);
  std::cout << "The math finder, " << finderName << " uses the following "
      << "feature extractors:\n";
  std::vector<BlobFeatureExtractor*> extractors = mathFinder->getFeatureExtractor()->getBlobFeatureExtractors();
  for(int i = 0; i < extractors.size(); ++i) {
    BlobFeatureExtractorDescription* const extractorDesc = extractors[i]->getFeatureExtractorDescription();
    std::cout << "\t" << extractorDesc->getName() << ": " << extractorDesc->getDescriptionText() << std::endl;
    std::vector<FeatureExtractorFlagDescription*> enabledFlags =
        extractors[i]->getEnabledFlagDescriptions();
    if(enabledFlags.size() > 0) {
      std::cout << "\tEnabled Flags:\n";
    }
    for(int j = 0; j < enabledFlags.size(); ++j) {
      std::cout << "\t\t" << enabledFlags[j]->getName() << ": " << enabledFlags[j]->getDescriptionText() << std::endl;
    }
  }
  std::cout << "Detector name: " << getDetectorName() << std::endl;
  std::cout << "Segmentor name: " << getSegmentorName() << std::endl;
  std::cout << "Groundtruth path: " << getGroundtruthDirPath() << std::endl;
  std::cout << "Description: " << getDescription() << std::endl;

  delete mathFinder; // done so deallocate
}

// Builder Stuff

FinderInfoBuilder* FinderInfoBuilder
::setFinderName(std::string finderName) {
  this->finderName = finderName;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setFinderTrainingPaths(
    FinderTrainingPaths* const finderTrainingPaths) {
  this->finderTrainingPaths = finderTrainingPaths;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setFeatureExtractorUniqueNames(std::vector<std::string> featureExtractorUniqueNames) {
  this->featureExtractorUniqueNames = featureExtractorUniqueNames;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setDetectorName(std::string detectorName) {
  this->detectorName = detectorName;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setSegmentorName(std::string segmentorName) {
  this->segmentorName = segmentorName;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setGroundtruthName(
    std::string groundtruthName) {
  this->groundtruthName = groundtruthName;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setGroundtruthDirPath(
    std::string groundtruthDirPath) {
  this->groundtruthDirPath = groundtruthDirPath;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setGroundtruthFilePath(std::string groundtruthFilePath) {
  this->groundtruthFilePath = groundtruthFilePath;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setDescription(std::string description) {
  this->description = description;
  return this;
}

FinderInfoBuilder* FinderInfoBuilder
::setGroundtruthImagePaths(std::vector<std::string> groundtruthImagePaths) {
  this->groundtruthImagePaths = groundtruthImagePaths;
  return this;
}

std::string FinderInfoBuilder::getFinderName() {
  return finderName;
}
FinderTrainingPaths* FinderInfoBuilder::getFinderTrainingPaths() {
  return finderTrainingPaths;
}
std::vector<std::string> FinderInfoBuilder::getFeatureExtractorUniqueNames() {
  return featureExtractorUniqueNames;
}
std::string FinderInfoBuilder::getDetectorName() {
  return detectorName;
}
std::string FinderInfoBuilder::getSegmentorName() {
  return segmentorName;
}
std::string FinderInfoBuilder::getGroundtruthName() {
  return groundtruthName;
}
std::string FinderInfoBuilder::getGroundtruthDirPath() {
  return groundtruthDirPath;
}
std::string FinderInfoBuilder::getGroundtruthFilePath() {
  return groundtruthFilePath;
}
std::string FinderInfoBuilder::getDescription() {
  return description;
}
std::vector<std::string> FinderInfoBuilder::getGroundtruthImagePaths() {
  return groundtruthImagePaths;
}

FinderInfo::~FinderInfo() {
  delete finderTrainingPaths;
}

/**
 * From the given data, provides info pertaining to a Finder
 * including all of the relevant training paths. Output memory
 * is owned by the caller.
 */
FinderInfo* FinderInfoBuilder::build() {
  if(finderTrainingPaths == NULL) {
    if(finderName == "") {
      std::cout << "Error: Attempt to create FinderInfo without name.\n";
      assert(false);
    }
    finderTrainingPaths =
        FinderTrainingPathsFactory().createFinderTrainingPaths(finderName);
  }
  return new FinderInfo(finderName,
      finderTrainingPaths,
      featureExtractorUniqueNames,
      detectorName,
      segmentorName,
      groundtruthName,
      groundtruthDirPath,
      groundtruthFilePath,
      description,
      groundtruthImagePaths);
}

