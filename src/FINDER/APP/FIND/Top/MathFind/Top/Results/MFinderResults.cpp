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
 * Getters
 */
Pix* MathExpressionFinderResults::getVisualResultsDisplay() {
  return visualResultsDisplay;
}

GenericVector<Segmentation*> MathExpressionFinderResults::getResults() {
  return results;
}

/**
 * Constructor (only invoked by builder)
 */
MathExpressionFinderResults::MathExpressionFinderResults(
    Pix* const visualResultsDisplay,
    GenericVector<Segmentation*> results,
    std::string resultsName,
    std::string resultsDirName) {
  this->visualResultsDisplay = visualResultsDisplay;
  this->results = results;
  this->resultsName = resultsName;
  this->resultsDirName = resultsDirName;
}

/**
 * Destructor
 */
MathExpressionFinderResults::~MathExpressionFinderResults() {
  pixDestroy(&visualResultsDisplay);
  for(int i = 0; i < results.length(); ++i) {
    delete results[i];
  }
  results.clear();
}

void MathExpressionFinderResults::displayResults() {
  pixDisplay(visualResultsDisplay, 100, 100);
  std::cout << "Displaying results for " << resultsName << std::endl;
  M_Utils::waitForInput();
}

void MathExpressionFinderResults::printResultsToFiles() {

  // Clear existing results directory and make new one to put results in
  if(Utils::existsDirectory(resultsDirName)) {
    Utils::exec("rm -rf " + resultsDirName);
  }
  Utils::exec("mkdir " + resultsDirName);

  resultsDirName = Utils::checkTrailingSlash(resultsDirName);
  std::string imgname = resultsDirName + resultsName;

  // save the results for all images into a file in the following format:
  // #.ext type left top right bottom
  std::string rectfile = resultsDirName + std::string("results.rect");
  std::ofstream rectstream(rectfile.c_str(), std::ios::app); // append to existing

  // make sure no duplicate regions in segmentation results (sanity check)
  ensureNoDuplicates();

  // print the segmentation results to the rect file
  for(int i = 0; i < results.length(); ++i) {
    const Segmentation* seg = results[i];
    BOX* bbox = M_Utils::tessTBoxToImBox(seg->box, visualResultsDisplay);
    const RESULT_TYPE& restype = seg->res;
    rectstream << resultsName <<
        ((restype == DISPLAYED) ? "displayed" : (restype == EMBEDDED)
            ? "embedded" : "label") << " " << bbox->x << " " << bbox->y
            << " " << bbox->x + bbox->w << " " << bbox->y + bbox->h << std::endl;
    boxDestroy(&bbox);
  }

   // save the images
   pixWrite((imgname + (std::string)".png").c_str(), visualResultsDisplay, IFF_PNG);
   rectstream.close();
}


void MathExpressionFinderResults::ensureNoDuplicates() {
  GenericVector<int> toRemove;
  for(int i = 0; i < results.length(); ++i) {
    for(int j = i + 1; j < results.length(); ++j) {
      if(*(results[i]->box) == *(results[j]->box)) {
        std::cout << "Warning: duplicate results found at " << results[i]->box << ", removing duplicate..." << std::endl;
        toRemove.push_back(j);
      }
    }
  }
  toRemove.sort();
  for(int i = toRemove.size() - 1; i > -1; --i) {
    std::cout << "Removing duplicate at " << results[toRemove[i]]->box << std::endl;
    results.remove(toRemove[i]);
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

/**
 * Build the object
 */
MathExpressionFinderResults* MathExpressionFinderResultsBuilder::build() {
  return new MathExpressionFinderResults(visualResultsDisplay,
      results,
      resultsName,
      resultsDirName);
}
