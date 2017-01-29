/*
 * MathExpressionFinderResults.h
 *
 *  Created on: Nov 3, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONFINDERRESULTS_H_
#define MATHEXPRESSIONFINDERRESULTS_H_

#include <allheaders.h>

#include <baseapi.h>

#include <BlobMergeData.h>

#include <string>
#include <vector>

/**
 * Specifies how the application is run. If in "DETECT" mode
 * then just the detection algorithm is run. In "FIND" mode,
 * both the detection and segmentation algorithms are run.
 */
enum RunMode { DETECT, FIND };

/**
 * Contains the resulting labeled rectangles from running a math
 * finder on an image. Also contains a visual display of the results
 * by overlaying the bounding boxes over the image with different colors
 * for different result types.
 *
 * Uses the builder pattern
 */
class MathExpressionFinderResults {
 public:

  MathExpressionFinderResults(
      Pix* const visualResultsDisplay,
      GenericVector<Segmentation*> results,
      std::string resultsName,
      std::string resultsDirName,
      RunMode runMode);

  ~MathExpressionFinderResults();

  /**
   * The getters
   */
  Pix* getVisualResultsDisplay();
  GenericVector<Segmentation*> getSegmentationResults();
  std::string getResultsName();
  std::string getResultsDirName();
  RunMode getRunMode();

  /**
   * Other public methods
   */
  // displays the segmentations
  void displaySegmentationResults();

  // prints the given result objects (each corresponding with an image,
  // not a segmentations (each image can have 0 or more segmentations)
  static void printResultsToFiles(
      const std::vector<MathExpressionFinderResults*>& results,
      const std::string& resultsDirName);

 private:
  void ensureNoDuplicates();

  Pix* visualResultsDisplay;
  GenericVector<Segmentation*> segmentationResults;
  std::string resultsName;
  std::string resultsDirName;
  RunMode runMode;
};

/**
 * The builder
 */
class MathExpressionFinderResultsBuilder {
 public:
  MathExpressionFinderResultsBuilder();
  MathExpressionFinderResultsBuilder* setVisualResultsDisplay(Pix* const visualResultsDispaly);
  MathExpressionFinderResultsBuilder* setResults(GenericVector<Segmentation*> results);
  MathExpressionFinderResultsBuilder* setResultsName(std::string resultsName);
  MathExpressionFinderResultsBuilder* setResultsDirName(std::string resultsDirName);
  MathExpressionFinderResultsBuilder* setRunMode(RunMode runMode);
  MathExpressionFinderResults* build();
 private:
  Pix* visualResultsDisplay;
  GenericVector<Segmentation*> results;
  std::string resultsName;
  std::string resultsDirName;
  RunMode runMode;
};

#endif /* MATHEXPRESSIONFINDERRESULTS_H_ */
