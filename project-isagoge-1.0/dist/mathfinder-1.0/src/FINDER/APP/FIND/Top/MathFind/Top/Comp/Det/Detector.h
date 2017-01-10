/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   MathExpressionDetector.h
 * Written by:  Jake Bruce
 * History:     Created Oct 14, 2013 6:53:21 PM
 *              Refactored Oct 30, 2016
 ***************************************************************************/
#ifndef MATH_EXPRESSION_DETECTOR_H
#define MATH_EXPRESSION_DETECTOR_H

#include <FeatExt.h>

#include <Sample.h>

#include <vector>
#include <string>

/**
 * Interface definition for a math detector, a component of the
 * math expression finder which takes a connected component from
 * an image and predicts whether or not this component is part of a math
 * expression.
 */
class MathExpressionDetector {
 public:

  /**
   * Takes the output of feature extraction as input. This is a grid containing
   * the connected pixels (blobs) and their features.
   *
   * Returns a grid containing the same input connected components which includes
   * their math expression detection status.
   */
  virtual void detectMathExpressions(BlobDataGrid* const featureExtractionOutput)=0;

  /**
   * If applicable, gets the harddrive path to the detector's binary file,
   * otherwise return empty string.
   */
  virtual std::string getDetectorPath()=0;

  /**
   * Trains the detector
   */
  virtual void doTraining(const std::vector<std::vector<BLSample*> >& samples)=0;

  virtual ~MathExpressionDetector(){};

 };

#endif
