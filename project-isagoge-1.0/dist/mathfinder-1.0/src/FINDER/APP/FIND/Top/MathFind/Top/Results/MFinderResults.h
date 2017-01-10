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
      std::string resultsDirName);

  ~MathExpressionFinderResults();

  /**
   * The getters
   */
  Pix* getVisualResultsDisplay();
  GenericVector<Segmentation*> getResults();
  std::string getResultsName();
  std::string getResultsDirName();

  void displayResults();

  void printResultsToFiles();

 private:
  void ensureNoDuplicates();

  Pix* visualResultsDisplay;
  GenericVector<Segmentation*> results;
  std::string resultsName;
  std::string resultsDirName;
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
  MathExpressionFinderResults* build();
 private:
  Pix* visualResultsDisplay;
  GenericVector<Segmentation*> results;
  std::string resultsName;
  std::string resultsDirName;
};

#endif /* MATHEXPRESSIONFINDERRESULTS_H_ */
