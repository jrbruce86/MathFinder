/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   IBinaryClassifier.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 14, 2013 7:15:28 PM
 * ------------------------------------------------------------------------
 * Description: Interface for the binary classification component of the
 *              MEDS module. Examples of binary classifiers which can
 *              implement this interface include SVMs, Neural Networks,
 *              Naive Bayes, Decision trees, etc. Compile-time polymorphism
 *              allows for various classification techniques to be conveniently
 *              compared with one another, using varying training methods.
 *              The ITrainer interface takes in the IBinaryClassifier
 *              and trains whatever classifier is implemented by it.
 * ------------------------------------------------------------------------
 * This file is part of Project Isagoge.
 *
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#ifndef IBINARYCLASSIFIER_H
#define IBINARYCLASSIFIER_H

#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>

// types of binary classifiers
#include "libSVM.h" // this implements Chu's soft margin classifier

template <typename BinClassType>
class IBinaryClassifier {
 public:
  IBinaryClassifier() {}

  inline void initClassifier(const string& predictor_path,
      const string& sample_path, const string& featext_name) {
    // allow for any initialization which may be required
    // for the classifier here....
    classifier.initClassifier(predictor_path, sample_path, featext_name);
  }

  inline void initPredictor() {
    classifier.initPredictor();
  }

  inline void doTraining(const vector<vector<BLSample*> >& samples) {
    classifier.doTraining(samples);
  }

  // Prediction is binary, either true or false.
  // True represents 1, false represents 0.
  inline bool predict(const std::vector<double>& sample) {
    if(classifier.isTrained()) {
      return classifier.predict(sample);
    }
    else {
      std::cout << "ERROR: MEDS detection module attempted prediction "
           << "with untrained classifier.\n";
      exit(EXIT_FAILURE);
    }
    return false;
  }

  inline bool isTrained() {
    return classifier.isTrained();
  }

  inline void reset() {
    classifier.reset();
  }

  inline string getFullPredictorPath() {
    return classifier.getFullPredictorPath();
  }

  inline string getFullSamplePath() {
    return classifier.getFullSamplePath();
  }

  BinClassType classifier;
};

#endif
