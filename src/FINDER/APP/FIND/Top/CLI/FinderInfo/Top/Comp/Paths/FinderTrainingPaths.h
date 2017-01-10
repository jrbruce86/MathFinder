/*
 * FinderTrainingPaths.h
 *
 *  Created on: Dec 9, 2016
 *      Author: jake
 */

#ifndef FINDERTRAININGPATHS_H_
#define FINDERTRAININGPATHS_H_

#include <string>

/**
 * Defines the paths relevant to FINDER training
 */
class FinderTrainingPaths {

 public:

  FinderTrainingPaths(
      const std::string trainingDirPath,
      const std::string infoFilePath,
      const std::string detectorDirPath,
      const std::string segmentorDirPath,
      const std::string featureExtDirPath,
      const std::string sampleFilePath);

  static std::string getTrainingRoot();
  static std::string getGroundtruthRoot();

  /**
   * Returns true if all of the required paths exist as expected,
   * prints an error and returns false otherwise. The sampe file path
   * is not required, so this returns true even if that does not exist
   */
  bool allRequiredExist();

  /**
   * Path to the root directory of this Finder's training data
   */
  std::string getTrainingDirPath();

  /**
   * Path to the file containing generic info about the specific math finder.
   */
  std::string getInfoFilePath();

  /**
   * Path to the directory containing any training resources needed for detection
   * if applicable for a specific Finder.
   */
  std::string getDetectorDirPath();

  /**
   * Path to the directory containing any training resources needed for segmentation
   * if applicable for a specific Finder.
   */
  std::string getSegmentorDirPath();

  /**
   * Path to the directory containing any resources needed for feature extraction
   * if applicable for a specific Finder.
   */
  std::string getFeatureExtDirPath();

  /**
   * Path to a file generated during training to store the samples so they could
   * be read in on subsequent trainings to save time.
   */
  std::string getSampleFilePath();

 private:

  // root dir for training of this Finder
  std::string trainingDirPath;

  // files and subdirs within the root dir
  std::string infoFilePath;
  std::string detectorDirPath;
  std::string segmentorDirPath;
  std::string featureExtDirPath;
  std::string sampleFilePath;

  bool printPathMissingError(const std::string& path);
};


#endif /* FINDERTRAININGPATHS_H_ */
