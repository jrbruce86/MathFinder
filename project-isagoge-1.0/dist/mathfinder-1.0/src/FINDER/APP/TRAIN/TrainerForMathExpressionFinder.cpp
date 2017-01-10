/*
 * TrainerForMathExpressionFinder.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <TrainerForMathExpressionFinder.h>

#include <DatasetMenu.h>
#include <GTParser.h>
#include <Sample.h>
#include <TrainingSampleExtractor.h>
#include <FinderInfo.h>
#include <FeatExt.h>
#include <Detector.h>
#include <Seg.h>

#include <stddef.h>
#include <assert.h>
#include <iostream>
#include <vector>

#define JUST_GET_SAMPLES

TrainerForMathExpressionFinder::TrainerForMathExpressionFinder(
    FinderInfo* finderInfo,
    MathExpressionFeatureExtractor* mathExpressionFeatureExtractor,
    MathExpressionDetector* mathExpressionDetector,
    MathExpressionSegmentor* mathExpressionSegmentor)
: samples(std::vector<std::vector<BLSample*> >()){
  this->finderInfo = finderInfo;
  this->featureExtractor = mathExpressionFeatureExtractor;
  this->detector = mathExpressionDetector;
  this->segmentor = mathExpressionSegmentor;
}

void TrainerForMathExpressionFinder::runTraining() {

  // train the detector
  trainDetector();

  // train the segmentor
  trainSegmentor();
}

void TrainerForMathExpressionFinder::trainDetector() {

  // if the predictor already exists, prompt to make sure user wants to retrain, if not return otherwise continue.
  bool doDetectorTraining = true;
  if(Utils::existsFile(detector->getDetectorPath())) {
    std::cout << "A trained detector at " << detector->getDetectorPath()
            << " already exists for the Finder that was selected for training. "
            << "Would you like to retrain that detector now? If you answer no, the "
            << "detector training will not be carried out, and the old detector will "
            << "be kept untouched. ";
    doDetectorTraining = Utils::promptYesNo();
  }
  if(!doDetectorTraining) {
    std::cout << "The detector was already trained and is not being re-trained.\n";
    return;
  }

  // Make sure the groundtruth directory is ok
  if(!DatasetSelectionMenu::groundtruthDirPathIsGood(finderInfo->getGroundtruthDirPath())) {
    std::cout << "ERROR: The groundtruth directory at " << finderInfo->getGroundtruthDirPath()
            << " appears to be corrupted. Cannot move forward with training without a valid groundtruth path.\n";
    std::cout << "Run the application with the -menu option to see more options.\n";
    assert(false);
  }

  // first get all of the binary labeled samples using the chosen feature extractors
  samples = getSamples();
  std::cout << "finished calling getSamples() method\n";
#ifdef JUST_GET_SAMPLES
  std::cout << "Finished getting samples. To continue with training, comment out "
      << "JUST_GET_SAMPLES.\n";
  exit(EXIT_SUCCESS);
#endif
  detector->doTraining(samples);
  std::cout << "Finished training the detector.\n";
}

std::vector<std::vector<BLSample*> > TrainerForMathExpressionFinder::getSamples() {
  if(samples.empty()) {
    samples = TrainingSampleExtractor(finderInfo, featureExtractor).getSamples();
  }
  return samples;
}

void TrainerForMathExpressionFinder::trainSegmentor() {
  //TODO fill in
}

// TODO make sure I'm deleting everything I need to here
TrainerForMathExpressionFinder::~TrainerForMathExpressionFinder() {
  delete finderInfo;
  delete featureExtractor;
  delete detector;
  delete segmentor;
  TrainingSampleExtractor::destroySamples(samples);
}



