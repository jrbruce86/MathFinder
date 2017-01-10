/*
 * Sample.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: jake
 */

#include <Sample.h>

#include <allheaders.h>

#include <iomanip>
#include <assert.h>

GroundTruthEntry::GroundTruthEntry()
: image_index(-1), entry(GT_Entry::INVALID), rect(NULL) {}

GroundTruthEntry::~GroundTruthEntry() {
  boxDestroy(&rect);
}

bool GroundTruthEntry::operator==(const GroundTruthEntry& othergtentry) {
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

bool GroundTruthEntry::operator!=(const GroundTruthEntry& othergtentry) {
  if(*this == othergtentry)
    return false;
  return true;
}


BLSample::BLSample()
: label(false), entry(NULL), blobbox(NULL),
  imageIndex(-1), imageName("") {}

// this operator overload is for debugging purposes only
// assumes operands are expected to be equal. throws exception to say
// where they aren't equal if unequal rather than returning false
bool BLSample::operator==(const BLSample& othersample) {
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
  assert(imageIndex == othersample.imageIndex);
  assert(imageName == othersample.imageName);
  return true;
}

bool BLSample::operator!=(const BLSample& othersample) {
  if(*this == othersample)
    return false;
  return true;
}

// destructor
BLSample::~BLSample() {
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

