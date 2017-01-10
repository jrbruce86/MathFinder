/*
 * FinderInfo.h
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#ifndef FINDERINFO_H_
#define FINDERINFO_H_

#include <FinderTrainingPaths.h>

#include <vector>
#include <string>

class FinderInfo {

 public:

  FinderInfo(
      std::string finderName,
      FinderTrainingPaths* finderTrainingPaths,
      std::vector<std::string> featureExtractorNames,
      std::string detectorName,
      std::string segmentorName,
      std::string groundtruthName,
      std::string groundtruthDirPath,
      std::string groundtruthFileName,
      std::string description,
      std::vector<std::string> groundtruthImagePaths);

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
  std::string getFinderName();

  /**
   * The paths to training directories relevant to this Finder
   */
  FinderTrainingPaths* getFinderTrainingPaths();


  std::vector<std::string> getFeatureExtractorUniqueNames();

  std::string getDetectorName();

  std::string getSegmentorName();

  /**
   * Name of the dataset used to train this Finder.
   */
  std::string getGroundtruthName();

  /**
   * Path to the directory containing the groundtruth that was used to train the
   * detector and/or segmentor of this Finder if applicable. The groundtruth
   * consists of the images and their associated box files containing the bounding
   * boxes of the math expressions.
   */
  std::string getGroundtruthDirPath();

  /**
   * The name of the ".dat" file containing the bounding boxes of the
   * groundtruth math expression regions in each image contained in the
   * dataset used for training.
   */
  std::string getGroundtruthFilePath();

  /**
   * Optional description for providing further details about the Finder
   */
  std::string getDescription();

  /**
   * Gets the paths to the training images. These should be named as
   * "0", "1", "2", etc. with the extension at the end.
   */
  std::vector<std::string> getGroundtruthImagePaths();

  void displayInformation();

 private:
  std::string finderName;
  FinderTrainingPaths* finderTrainingPaths;
  std::vector<std::string> featureExtractorUniqueNames;
  std::string detectorName;
  std::string segmentorName;
  std::string groundtruthName;
  std::string groundtruthDirPath;
  std::string groundtruthFilePath;
  std::string description;
  std::vector<std::string> groundtruthImagePaths;
};

class FinderInfoBuilder {

 public:

  FinderInfoBuilder* setFinderName(std::string finderName);

  FinderInfoBuilder* setFinderTrainingPaths(FinderTrainingPaths* const finderTraininPaths);

  FinderInfoBuilder* setFeatureExtractorUniqueNames(std::vector<std::string> featureExtractorNames);

  FinderInfoBuilder* setDetectorName(std::string detectorName);

  FinderInfoBuilder* setSegmentorName(std::string segmentorName);

  FinderInfoBuilder* setGroundtruthName(std::string datasetName);

  FinderInfoBuilder* setGroundtruthDirPath(std::string datasetDirPath);

  FinderInfoBuilder* setGroundtruthFilePath(std::string groundtruthFileName);

  FinderInfoBuilder* setDescription(std::string description);

  FinderInfoBuilder* setGroundtruthImagePaths(std::vector<std::string> trainingImagePaths);

  std::string getFinderName();
  FinderTrainingPaths* getFinderTrainingPaths();
  std::vector<std::string> getFeatureExtractorUniqueNames();
  std::string getDetectorName();
  std::string getSegmentorName();
  std::string getGroundtruthName();
  std::string getGroundtruthDirPath();
  std::string getGroundtruthFilePath();
  std::string getDescription();
  std::vector<std::string> getGroundtruthImagePaths();


  /**
   * Builds an immutable data structure out of the parsed data.
   * The resulting memory is owned by the caller.
   */
  FinderInfo* build();

 private:
  std::string finderName;
  FinderTrainingPaths* finderTrainingPaths;
  std::vector<std::string> featureExtractorUniqueNames;
  std::string detectorName;
  std::string segmentorName;
  std::string groundtruthName;
  std::string groundtruthDirPath;
  std::string groundtruthFilePath;
  std::string description;
  std::vector<std::string> groundtruthImagePaths;
};

#endif /* FINDERINFO_H_ */
