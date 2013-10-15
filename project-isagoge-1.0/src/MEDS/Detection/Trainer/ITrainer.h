/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   ITrainer.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 14, 2013 6:53:21 PM
 * ------------------------------------------------------------------------
 * Description: Interface for the binary classifier training component of
 *              the MEDS module. Examples of training methods that could
 *              implement this class include cross validation, boosting,
 *              boot strap with aggregation (BAG), etc.
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

#include <IBinaryClassifier.h>
#include <IFeatureExtractor.h>

template <typename Trainer_Type>
class ITrainer {
 public:
  ITrainer(Trainer_Type& t) : trainer(t) {}
  void setClassifierExtractor(IBinaryClassifier& bc, IFeatureExtractor& fe) {
    bin_class = bc;
    fe_extract = fe;
  }
  inline IBinaryClassifier train() {
    return trainer.train(bin_class, fe_extract);
  }
  IBinaryClassifier bin_class;
  IFeatureExtractor fe_extract;
  Trainer_Type trainer;
};

#endif
