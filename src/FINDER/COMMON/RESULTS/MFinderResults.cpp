/*
 * MathExpressionFinderResults.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: jake
 */

#include <MFinderResults.h>

#include <allheaders.h>

#include <baseapi.h>

#include <BlobMergeData.h>
#include <M_Utils.h>
#include <Utils.h>

#include <string>
#include <iostream>
#include <vector>
#include <stddef.h>

/**
 * Constructor (only invoked by builder)
 */
MathExpressionFinderResults::MathExpressionFinderResults(
    Pix* const visualResultsDisplay,
    GenericVector<Segmentation*> results,
    std::string resultsName,
    std::string resultsDirName,
    RunMode runMode) {
  this->visualResultsDisplay = visualResultsDisplay;
  this->segmentationResults = results;
  this->resultsName = resultsName;
  this->resultsDirName = resultsDirName;
  this->runMode = runMode;
}

/**
 * Destructor
 */
MathExpressionFinderResults::~MathExpressionFinderResults() {
  pixDestroy(&visualResultsDisplay);
  for(int i = 0; i < segmentationResults.length(); ++i) {
    delete segmentationResults[i];
  }
  segmentationResults.clear();
}

/**
 * Getters
 */
Pix* MathExpressionFinderResults::getVisualResultsDisplay() {
  return visualResultsDisplay;
}

GenericVector<Segmentation*> MathExpressionFinderResults::getSegmentationResults() {
  return segmentationResults;
}

std::string MathExpressionFinderResults::getResultsName() {
  return resultsName;
}

std::string MathExpressionFinderResults::getResultsDirName() {
  return getResultsDirName();
}

RunMode MathExpressionFinderResults::getRunMode() {
  return getRunMode();
}

/**
 * Other public methods
 */
void MathExpressionFinderResults::displaySegmentationResults() {
  pixDisplay(visualResultsDisplay, 100, 100);
  const std::string runModeStr = (runMode == FIND) ?
      std::string("segmentation") : std::string("detection");
  std::cout << "Displaying " << runModeStr << " results for "
      << resultsName << std::endl;
  M_Utils::waitForInput();
}

void MathExpressionFinderResults::printResultsToFiles(
    const std::vector<MathExpressionFinderResults*>& results,
    const std::string& resultsDirPath_) {

  // Clear existing results directory and make new one to put results in
  if(Utils::existsDirectory(resultsDirPath_)) {
    Utils::exec("rm -rf " + resultsDirPath_, true);
  }
  Utils::exec("mkdir " + resultsDirPath_, true);
  std::cout << "Creating results directory at " << resultsDirPath_ << std::endl;

  const std::string resultsDirPath = Utils::checkTrailingSlash(resultsDirPath_);
  const std::string rectfile = resultsDirPath + std::string("results.rect");
  // save the results for all images into a file in the following format:
   // #.ext type left top right bottom
   std::ofstream rectstream(rectfile.c_str());

  for(int i = 0; i < results.size(); ++i) {

    MathExpressionFinderResults* const imageResults = results[i];

    const std::string imgname = resultsDirPath + imageResults->getResultsName();

    // make sure no duplicate regions in segmentation results (sanity check)
    imageResults->ensureNoDuplicates();

    // print the segmentation results to the rect file
    GenericVector<Segmentation*> segmentationResults = imageResults->getSegmentationResults();
    for(int j = 0; j < segmentationResults.length(); ++j) {
      const Segmentation* seg = segmentationResults[j];
      BOX* bbox = M_Utils::tessTBoxToImBox(seg->box, imageResults->getVisualResultsDisplay());
      const RESULT_TYPE restype = seg->res;
      rectstream << imageResults->getResultsName() << " " <<
          ((restype == DISPLAYED) ? "displayed" : (restype == EMBEDDED)
              ? "embedded" : "label") << " " << bbox->x << " " << bbox->y
              << " " << bbox->x + bbox->w << " " << bbox->y + bbox->h << std::endl;
      boxDestroy(&bbox);
    }

    // save the image
    pixWrite((imgname + (std::string)".png").c_str(),
        imageResults->getVisualResultsDisplay(),
        IFF_PNG);
    rectstream.flush();
  }

  rectstream.close();
}


void MathExpressionFinderResults::ensureNoDuplicates() {
  GenericVector<int> toRemove;
  for(int i = 0; i < segmentationResults.length(); ++i) {
    for(int j = i + 1; j < segmentationResults.length(); ++j) {
      if(*(segmentationResults[i]->box) == *(segmentationResults[j]->box)) {
        std::cout << "Warning: duplicate results found at " << segmentationResults[i]->box << ", removing duplicate..." << std::endl;
        toRemove.push_back(j);
      }
    }
  }
  toRemove.sort();
  for(int i = toRemove.size() - 1; i > -1; --i) {
    std::cout << "Removing duplicate at " << segmentationResults[toRemove[i]]->box << std::endl;
    segmentationResults.remove(toRemove[i]);
  }
}


/***************************************************************
 * Builder stuff below here
 **************************************************************/

MathExpressionFinderResultsBuilder::MathExpressionFinderResultsBuilder()
: visualResultsDisplay(NULL), resultsName("") {}

/**
 * Setters
 */
MathExpressionFinderResultsBuilder* MathExpressionFinderResultsBuilder::setVisualResultsDisplay(
    Pix* const visualResultsDisplay) {
  this->visualResultsDisplay = visualResultsDisplay;
  return this;
}

MathExpressionFinderResultsBuilder* MathExpressionFinderResultsBuilder::setResults(
    GenericVector<Segmentation*> results) {
  this->results = results;
  return this;
}

MathExpressionFinderResultsBuilder* MathExpressionFinderResultsBuilder::setResultsName(
    std::string resultsName) {
  this->resultsName = resultsName;
  return this;
}

MathExpressionFinderResultsBuilder* MathExpressionFinderResultsBuilder::setResultsDirName(
    std::string resultsDirName) {
  this->resultsDirName = resultsDirName;
  return this;
}

MathExpressionFinderResultsBuilder* MathExpressionFinderResultsBuilder::setRunMode(
    RunMode runMode) {
  this->runMode = runMode;
  return this;
}

/**
 * Build the object
 */
MathExpressionFinderResults* MathExpressionFinderResultsBuilder::build() {
  return new MathExpressionFinderResults(visualResultsDisplay,
      results,
      resultsName,
      resultsDirName,
      runMode);
}
