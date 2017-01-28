/*
 * TesseractCharData.cpp
 *
 *  Created on: Nov 20, 2016
 *      Author: jake
 */

#include <stddef.h>

#include <CharData.h>
#include <WordData.h>

/**
 * Constructor
 * Takes a copy of the bounding box for the recognized character
 * All other fields are set after construction
 */
TesseractCharData::TesseractCharData(
    TBOX charResultBoundingBox,
    TesseractWordData* parentWord):
   recognitionResultUnicode(""), charResultInfo(NULL),
   dist_above_baseline(0) {
  this->charResultBoundingBox = charResultBoundingBox;
  this->parentWord = parentWord;
}

TesseractCharData::~TesseractCharData() {
  // Blobs are deleted by the grid. Don't think there's anything to delete here?
}

/**
 * Setters
 */
TesseractCharData* TesseractCharData::
setRecognitionResultUnicode(const std::string recognitionResultUnicode) {
  if(this->recognitionResultUnicode.empty()) {
    this->recognitionResultUnicode = recognitionResultUnicode; // passes in a copy
  }
  return this;
}
TesseractCharData* TesseractCharData::
setCharResultInfo(BLOB_CHOICE* charResultInfo) {
  if(this->charResultInfo == NULL) {
    this->charResultInfo = charResultInfo;
  }
  return this;
}
TesseractCharData* TesseractCharData::
setDistanceAboveRowBaseline(const double dist) {
  this->dist_above_baseline = dist;
  return this;
}

/**
 * Getters (just returning all pointers to be consistent.. don't ever use this outside its scope
 * and always reference with const pointer, e.g. TBOX* const box)
 */
TBOX* TesseractCharData::getBoundingBox() {
  return &charResultBoundingBox;
}
std::string TesseractCharData::getUnicode() {
  return recognitionResultUnicode;
}
BLOB_CHOICE* TesseractCharData::getCharResultInfo() {
  return charResultInfo;
}
double TesseractCharData::getDistanceAboveRowBaseline() {
  return dist_above_baseline;
}
TesseractWordData* TesseractCharData::getParentWord() {
  return parentWord;
}
std::vector<BlobData*>& TesseractCharData::getBlobs() {
  return blobs;
}

bool TesseractCharData::isLeftmostInWord() {
  return parentWord->getTesseractChars()[0] == this;
}
