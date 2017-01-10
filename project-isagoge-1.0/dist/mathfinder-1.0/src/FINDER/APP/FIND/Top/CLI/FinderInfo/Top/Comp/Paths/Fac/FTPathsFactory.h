/*
 * FinderTrainingPathsFactory.h
 *
 *  Created on: Dec 10, 2016
 *      Author: jake
 */

#ifndef FINDERTRAININGPATHSFACTORY_H_
#define FINDERTRAININGPATHSFACTORY_H_

#include <FinderTrainingPaths.h>

#include <string>

class FinderTrainingPathsFactory {

 public:

  /**
   * Based on the Finder name, creates an object containing the training
   * paths relevant to that Finder.
   */
  FinderTrainingPaths* createFinderTrainingPaths(std::string finderName);
};


#endif /* FINDERTRAININGPATHSFACTORY_H_ */
