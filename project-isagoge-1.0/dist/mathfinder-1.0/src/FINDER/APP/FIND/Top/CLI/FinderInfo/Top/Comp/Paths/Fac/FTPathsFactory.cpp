/*
 * FinderTrainingPathsFactory.cpp
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#include <FTPathsFactory.h>

#include <FinderTrainingPaths.h>
#include <Utils.h>

#include <string>

/**
 * Based on the Finder name, creates an object containing the training
 * paths relevant to that Finder.
 */
FinderTrainingPaths* FinderTrainingPathsFactory::FinderTrainingPathsFactory
::createFinderTrainingPaths(std::string finderName) {

  const std::string finderTrainingRoot =
      Utils::checkTrailingSlash(FinderTrainingPaths::getTrainingRoot()) + finderName + "/";

  return new FinderTrainingPaths(
      finderTrainingRoot,
      finderTrainingRoot + "info",
      finderTrainingRoot + "detector",
      finderTrainingRoot + "segmentor",
      finderTrainingRoot + "featureExt",
      finderTrainingRoot + "featureExt/TrainingSamples");
}

