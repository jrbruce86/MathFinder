/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   Sample.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 2, 2013 4:21:19 PM
 * ------------------------------------------------------------------------
 * Description: Simple class for binary labeled image region corresponding to
 *              a Tesseract blob. A vector of these samples are fed into
 *              a binary classifier for training or prediction, in order
 *              to localize mathematical regions on the page.
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
#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>
#include <allheaders.h>
#include <Basic_Utils.h>
#include <assert.h>
#include <iomanip>
using namespace std;

namespace GT_Entry {
  enum GTEntryType {DISPLAYED, EMBEDDED, LABEL};
}

class GroundTruthEntry {
 public:
  ~GroundTruthEntry() {
    boxDestroy(&rect);
  }
  int image_index;
  GT_Entry::GTEntryType entry;
  Box* rect; // this is the rectangle for
             // the math region if applicable
             // (i.e. if the blob is inside one)
  bool operator==(const GroundTruthEntry& othergtentry) {
    if(image_index != othergtentry.image_index)
      return false;
    if(entry != othergtentry.entry)
      return false;
    int boxes_equal = 0;
    boxEqual(rect, othergtentry.rect, &boxes_equal);
    if(!boxes_equal)
      return false;
    return true;
  }
  bool operator!=(const GroundTruthEntry& othergtentry) {
    if(*this == othergtentry)
      return false;
    return true;
  }
};

// binary labeled sample
struct BLSample {
  vector<double> features;
  bool label;
  GroundTruthEntry* entry; // if this is NULL then the sample is non-math
  BOX* blobbox; // the sample's bounding box for debugging
  int image_index;

  // this operator overload is for debugging purposes only
  // assumes operands are expected to be equal. throws exception to say
  // where they aren't equal if unequal rather than returning false
  bool operator==(const BLSample& othersample) {
    assert(features.size() == othersample.features.size());
    for(int i = 0; i < features.size(); i++) {
   /*   if(features[i] != othersample.features[i]) {
        cout << "The sample features are different:\n";
        cout << "Sample 1: " << setprecision(20) << features[i] << endl;
        cout << "Sample 2: " << setprecision(20) << othersample.features[i] << endl;
      }*/
      assert(features[i] == othersample.features[i]);
    }
    assert(label == othersample.label);
    if(entry != NULL) {
      assert(othersample.entry != NULL);
      assert(*entry == *(othersample.entry));
    }
    else
      assert(othersample.entry == NULL);
    int boxes_equal = 0;
    boxEqual(blobbox, othersample.blobbox, &boxes_equal);
    assert(boxes_equal);
    assert(image_index == othersample.image_index);
    return true;
  }

  bool operator!=(const BLSample& othersample) {
    if(*this == othersample)
      return false;
    return true;
  }

  // destructor
  ~BLSample() {
    features.clear();
    if(entry != NULL) {
      delete entry;
      entry = NULL;
    }
    if(blobbox != NULL) {
      boxDestroy(&blobbox);
      blobbox = NULL;
    }
  }
};

#endif
