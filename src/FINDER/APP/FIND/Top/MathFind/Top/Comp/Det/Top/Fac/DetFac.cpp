/*
 * MathExpressionDetectorFactory.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: jake
 */
#include <DetFac.h>

#include <Detector.h>
#include <FinderInfo.h>
#include <SvmDetector.h>

#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <stddef.h>

MathExpressionDetectorFactory::MathExpressionDetectorFactory()
: svmDetectorName("SVM") {
  supportedDetectorNames.push_back(svmDetectorName);
}

MathExpressionDetector* MathExpressionDetectorFactory
::createMathExpressionDetector(
    FinderInfo* const finderInfo) {

  // Fix the detector data path
  const std::string detectorDataPath =
      finderInfo->getFinderTrainingPaths()->getDetectorDirPath();

  if(finderInfo->getDetectorName() == svmDetectorName) {
    return new TrainedSvmDetector(detectorDataPath);
  }

  std::cout << "Error: Could not find the detector named " << finderInfo->getDetectorName() << "\n";
  assert(false); // Hopefully won't get here....
  return NULL;
}

std::vector<std::string> MathExpressionDetectorFactory::getSupportedDetectorNames() {
  return supportedDetectorNames;
}

