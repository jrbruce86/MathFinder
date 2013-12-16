/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   ITrainer.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 23, 2013 12:27:13 PM
 * ------------------------------------------------------------------------
 * Description: The interface for all training techniques. Examples of training
 *              methods which can implement this interface include cross validation
 *              boot strapping with aggregation (bagging), jackknife, or boosting.
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
#ifndef ITRAINER_H
#define ITRAINER_H

#include <Sample.h>
#include <IBinaryClassifier.h>

// types of trainers
#include "CrossValidationSVM.h" // this one is just a place holder
                                // real training for SVM is done in
                                // the SVM implementations

template <typename TrainerType,
typename BinClassType>
class ITrainer {
  // some shorthand to make things less messy
  typedef IBinaryClassifier<BinClassType> IClassifier;

 public:
  ITrainer<TrainerType, BinClassType>() {
    classifier = new IClassifier();
  }

  ~ITrainer<TrainerType, BinClassType>() {
    if(classifier != NULL) {
      delete classifier;
      classifier = NULL;
    }
  }

  inline void initTraining(IClassifier* classifier_) {
    classifier = classifier_;
    trainer.initTraining(classifier);
  }

  IClassifier* train_(const vector<vector<BLSample*> >& samples) {
    classifier = trainer.train_(samples);
    // return the interface to the trained classifier
    return classifier;
  }

  inline void reset() {
    trainer.reset();
  }

  TrainerType trainer;
  IClassifier* classifier;
};

#endif
