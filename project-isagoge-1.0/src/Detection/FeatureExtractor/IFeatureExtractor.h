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

#define DBG_AFTER_EXTRACTION

using namespace tesseract;

// types of feature extractors
#include <FE_Implementations.h>

template<typename FeatureExtType>
class IFeatureExtractor {
 public:
  IFeatureExtractor<FeatureExtType>() {}

  // Required initialization of the feature extractor for both training and prediction
  inline void initFeatExtFull(TessBaseAPI& api, const string& groundtruth_path,
      const string& training_set_path, const string& ext, bool makenew) {
    feat_ext.initFeatExtFull(api, groundtruth_path, training_set_path, ext, makenew);
  }


  // do feature extraction initializations
  // specific to a single page
  inline void initFeatExtSinglePage() {
    feat_ext.initFeatExtSinglePage();
  }

  inline string getFeatExtName() {
    return feat_ext.getFeatExtName();
  }

  // this is good for training purposes, just calling extractFeatures
  // is more useful for situations wherein I'm just wanting to extract
  // features for a single page. In training I'll typically be training
  // on the data coming from a set of 15 pages or so. Extracting all the
  // features at once makes life easier in this circumstance, so I can
  // then put all the sample data together into one vector up front and
  // then do the training from there
  tesseract::BlobInfoGrid* extractAllFeatures(tesseract::BlobInfoGrid* grid) {
    tesseract::BlobInfoGridSearch bigs(grid);
    bigs.StartFullSearch();
    tesseract::BLOBINFO* blob = NULL;
    while((blob = bigs.NextFullSearch()) != NULL)
      blob->features = extractFeatures(blob);
#ifdef DBG_AFTER_EXTRACTION
    feat_ext.dbgAfterExtraction();
#endif
    return grid;
  }

  // *This method is where all of the feature extraction actually occurs*
  // - This should be called during prediction. The output is then
  // fed into the binary classifier. usually for each blob in the
  // page in turn, I'll be sending the blob through the feature
  // extractor then send the resulting features through the
  // binary classifier (assuming it was trained on the very same
  // features).
  // - Also called by extractAllFeatures during training
  // repeatedly getting all the features on the page and storing
  // them in the grid.
  inline vector<double> extractFeatures(tesseract::BLOBINFO* blob) {
    return feat_ext.extractFeatures(blob);
  }

  inline void setImage(PIX* im) {
    feat_ext.setImage(im);
  }

  inline void setApi(const TessBaseAPI& api) {
    feat_ext.setApi(api);
  }

  inline void setGrid(BlobInfoGrid* grid) {
    feat_ext.setGrid(grid);
  }

  inline void setDBGDir(const string& dir) {
    feat_ext.setDBGDir(dir);
  }

  inline void reset() {
    feat_ext.reset();
  }

  inline int numFeatures() {
    return feat_ext.numFeatures();
  }

  FeatureExtType feat_ext;
};

#endif

