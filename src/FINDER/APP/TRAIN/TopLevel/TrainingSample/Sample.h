/**************************************************************************
 * File name:   Sample.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 2, 2013 4:21:19 PM
 *              Modified Dec 17, 2016
 * ------------------------------------------------------------------------
 * Description: Simple class for binary labeled image region corresponding to
 *              a Tesseract blob. A vector of these samples are fed into
 *              a binary classifier for training or prediction, in order
 *              to localize mathematical regions on the page.
 ***************************************************************************/
#ifndef SAMPLE_H
#define SAMPLE_H

#include <Utils.h>
#include <DoubleFeature.h>

#include <allheaders.h>

#include <vector>
#include <assert.h>

namespace GT_Entry {
  enum GTEntryType {DISPLAYED, EMBEDDED, LABEL, INVALID};
}

struct GroundTruthEntry {

  GroundTruthEntry();

  ~GroundTruthEntry();

  bool operator==(const GroundTruthEntry& othergtentry);

  bool operator!=(const GroundTruthEntry& othergtentry);

  int image_index;
  GT_Entry::GTEntryType entry;
  Box* rect; // this is the rectangle for
             // the math region if applicable
             // (i.e. if the blob is inside one)
};

// binary labeled sample
struct BLSample {

  BLSample();

  // destructor
  ~BLSample();

  // this operator overload is for debugging purposes only
  // assumes operands are expected to be equal. throws exception to say
  // where they aren't equal if unequal rather than returning false
  bool operator==(const BLSample& othersample);

  bool operator!=(const BLSample& othersample);

  std::vector<DoubleFeature*> features;
  bool label;
  GroundTruthEntry* entry; // if this is NULL then the sample is non-math
  BOX* blobbox; // the sample's bounding box for debugging
  std::string imageName;
  int imageIndex;
};

#endif
