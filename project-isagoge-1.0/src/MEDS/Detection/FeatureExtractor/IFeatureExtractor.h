/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   IFeatureExtractor.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 14, 2013 7:05:34 PM
 * ------------------------------------------------------------------------
 * Description: Interface for the feature extraction component of the MEDS
 *              module. The goal of abstracting away the details of feature
 *              extraction are so that it can be made easier to train a
 *              binary classifier on different variations of features and
 *              then compare the results.
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
#ifndef IFEATUREEXTRACTOR_H
#define IFEATUREEXTRACTOR_H

#include <BlobInfoGrid.h>

template<typename FeatureExtType>
class IFeatureExtractor {
 public:
  IFeatureExtractor(FeatureExtType& fet) : feat_ext(fet), bigs(NULL) {}
  void setData(tesseract::BlobInfoGridSearch* bigs_) {
    bigs = bigs_;
  }
  vector<double> extractFeatures(tesseract::BLOBINFO* symbol) {
    return feat_ext.extractFeatures(symbol);
  }
  tesseract::BlobInfoGridSearch* bigs;
  FeatureExtType feat_ext;
};

#endif

