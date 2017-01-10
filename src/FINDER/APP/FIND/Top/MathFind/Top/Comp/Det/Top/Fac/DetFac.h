/*
 * MathExpressionDetectorFactory.h
 *
 *  Created on: Dec 7, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONDETECTORFACTORY_H_
#define MATHEXPRESSIONDETECTORFACTORY_H_

#include <Detector.h>
#include <FinderInfo.h>

#include <vector>
#include <string>

class MathExpressionDetectorFactory {

 public:

  MathExpressionDetectorFactory();

  /**
   * Creates a math expression detector using the given args
   */
  MathExpressionDetector* createMathExpressionDetector(
      FinderInfo* finderInfo);

  std::vector<std::string> getSupportedDetectorNames();

 private:

  // Supported detector names
  std::string svmDetectorName;
  std::vector<std::string> supportedDetectorNames; // as a list
};


#endif /* MATHEXPRESSIONDETECTORFACTORY_H_ */
