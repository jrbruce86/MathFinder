/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   CrossValidation.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 22, 2013 4:45:42 PM
 * ------------------------------------------------------------------------
 * Description: Uses DLib's cross validation implementation in order to train
 *              a binary classifier. This is really unnecessary in this case
 *              but is kept for consistency with other classifiers that may
 *              allow for more separation between the classification and
 *              training tasks. In DLib they are more paired together so it
 *              is easier to just implement all the training within the
 *              classifier itself. This is essentially just a placeholder...
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
#ifndef CROSSVALIDATIONSVM_H
#define CROSSVALIDATIONSVM_H

#include <Sample.h>
#include <ITrainer.h>
#include <dlib/svm.h>

template <typename BinClassType>
class CrossValidatorSVM {
  // some shorthand to make things less messy
  typedef IBinaryClassifier<BinClassType> IClassifier;

 public:
  CrossValidatorSVM<BinClassType>() {}

  inline void initTraining(IClassifier& classifier_) {
    classifier = classifier_;
  }

  inline IClassifier train_(const vector<vector<BLSample*> >& samples) {
    classifier.doTraining(samples);
    return classifier;
  }

  inline void reset() {
  }

  IClassifier classifier;
};

#endif
