/*
 * Evaluator.h
 *
 *  Created on: Dec 31, 2016
 *      Author: jake
 */

#ifndef EVALUATOR_H_
#define EVALUATOR_H_

#include <DatasetMetrics.h>
#include <baseapi.h>

/********************************************************************************
* Evaluate layout accuracy.                                                     *
*                                                                               *
* The evaluation                                                                *
* technique is primarily based off of the evaluation used in:                   *
*                                                                               *
*  - F. Shafait, D. Keysers, and T. M. Breuel. "Pixel-Accurate Representation   *
*  - and Evaluation of Page Segmentation in Document Images," 18th Int. Conf.   *
*  - on Pattern Recognition, (Hong Kong, China), pp. 872–875, Aug. 2006.        *
*                                                                               *
* The output data structures representing the default Tesseract results and the *
* system created in this word will be compared in a separate method.            *
*                                                                               *
* Takes the groundtruth data                                                    *
* and uses it to color the blobs of interest appropriately based on their type. *
*                                                                               *
********************************************************************************/
class Evaluator {
 public:

  Evaluator(
      std::string resultsPath,
      std::string groundtruthPath,
      bool typeSpecificMode);

  void evaluateSingleRun();

  void evaluateMultipleRuns();

 private:

  bool verifyResultsAndGroundtruthPaths();
  bool verifyOneRectFileAt(const std::string& dirPath);

  bool colorGroundtruthBlobs();
  bool coloredImageCopiesExist(const std::vector<std::string>& inputImagePaths);

  bool checkImNameMatchesIndex(const std::string& name, const int index);

  Pix* readInColorImage(const std::string& imagePath);

  HypothesisMetrics getEvaluationMetrics(const std::string typenamespec,
      const int imageIndex);
  std::vector<DatasetMetrics> getDatasetAverages(
      const std::vector<std::vector<HypothesisMetrics> >& all_metrics);

  std::string resultsDirPath;
  std::string groundtruthDirPath;
  bool typeSpecificMode;

  const std::string coloredImageSubDirName;
  std::string coloredGroundtruthImageDirPath;
  std::string groundtruthRectFilePath;
  std::string resultsRectFilePath;

  std::vector<std::string> inputImagePaths;
  std::vector<std::string> coloredGroundtruthImagePaths;
  std::vector<std::string> coloredResultsImagePaths;

  tesseract::ImageThresholder imageThresholder;
};


#endif /* EVALUATOR_H_ */
