/*
 * BlobData.cpp
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#include <BlobData.h>

#include <BlobFeatExtData.h>
#include <WordData.h>
#include <CharData.h>
#include <RowData.h>
#include <BlockData.h>

BlobData::BlobData(TBOX box, PIX* blobImage, BlobDataGrid* parentGrid)
    : mathExpressionDetectionResult(false),
      tesseractCharData(NULL) {
  this->box = box;
  this->blobImage = blobImage;
  this->parentGrid = parentGrid;
}

BlobData::~BlobData() {
  // have to check for null because this ref might be shared by multiple blobs
  if(tesseractCharData != NULL) {
    delete tesseractCharData;
  }
}

TBOX BlobData::bounding_box() const {
  return box;
}

/**
 * Sets this blob's character recognition data found by Tesseract.
 * There is possibly a one-to-many relationship between a recognition
 * result of Tesseract and the blobs in the image. Thus several neighboring
 * characters may share this same value. This is because Tesseract may merge
 * multiple blobs together to form one character (as with broken characters
 * due to low image quality or symbols like '=').
 */
void BlobData::setCharacterRecognitionData(TesseractCharData* tesseractCharRecognitionData) {
  this->tesseractCharData = tesseractCharRecognitionData;
}

/**
 * Gets this blob's character recognition data found by Tesseract.
 * There is possibly a one-to-many relationship between a recognition
 * result of Tesseract and the blobs in the image. Thus several neighboring
 * characters may share this same value. This is because Tesseract may merge
 * multiple blobs together to form one character (as with broken characters
 * due to low image quality or symbols like '=').
 */
TesseractCharData* BlobData::getParentChar() {
  return tesseractCharData;
}

const char* BlobData::getParentWordstr() {
  if(getParentWord() == NULL) {
    return NULL;
  }
  return getParentWord()->wordstr();
}

TesseractWordData* BlobData::getParentWord() {
  if(getParentChar() == NULL) {
    return NULL;
  }
  return getParentChar()->getParentWord();
}

TesseractRowData* BlobData::getParentRow() {
  if(getParentWord() == NULL) {
    return NULL;
  }
  return getParentWord()->getParentRow();
}

TesseractBlockData* BlobData::getParentBlock() {
  if(getParentRow() == NULL) {
    return NULL;
  }
  return getParentRow()->getParentBlock();
}

/**
 * Gets the size of the variable data vector
 */
int BlobData::getVariableDataLength() {
  return variableExtractionData.size();
}

/**
 * Looks up an entry in the variable data vector and
 * returns it as a char*. It is up to the feature extractor
 * to know the index and cast the returned pointer appropriately.
 */
BlobFeatureExtractionData* BlobData::getVariableDataAt(const int& i) {
  return variableExtractionData.at(i);
}

/**
 * Adds new data to the variable data vector for this blob.
 * No more than one data entry should be added per feature extractor
 * and if a feature extractor adds a data entry for one blob it must
 * add one for all of them. This method returns the vector index at
 * which the data was placed for this blob. This return can be viewed
 * as the key for grabbing the data later when it's needed. If no
 * data need be retrieved for this blob later than no need to use
 * this method.
 */
int BlobData::appendNewVariableData(BlobFeatureExtractionData* const data) {
  variableExtractionData.push_back(data);
  return variableExtractionData.size() - 1;
}

/**
 * Returns a reference to an immutable version of this blob's bounding box
 */
const TBOX& BlobData::getBoundingBox() const {
  return box;
}

/**
 * Appends the provided vector of features to this blob entry's array
 * of features. Once finalized, this array should contain one or more
 * features for all feature extractions that were carried out.
 */
void BlobData::appendExtractedFeatures(std::vector<DoubleFeature*> extractedFeatures_) {
  for(int i = 0; i < extractedFeatures_.size(); ++i) {
    this->extractedFeatures.push_back(extractedFeatures_[i]);
  }
}

/**
 * Sets the result of math expression detection (should be set by the detector)
 */
void BlobData::setMathExpressionDetectionResult(bool mathExpressionDetectionResult) {
  this->mathExpressionDetectionResult = mathExpressionDetectionResult;
}

/**
 * Gets the result of math expression detection assuming it has been carried out.
 * Would return false no matter what if no detection has been carried out
 */
bool BlobData::getMathExpressionDetectionResult() {
  return mathExpressionDetectionResult;
}

std::vector<DoubleFeature*> BlobData::getExtractedFeatures() {
  return extractedFeatures;
}

bool BlobData::belongsToRecognizedWord() {
  if(getParentWord() == NULL) {
    return false;
  }
  return getParentWord()->getIsValidTessWord();
}

bool BlobData::belongsToRecognizedMathWord() {
  if(getParentWord() == NULL) {
    return false;
  }
  return getParentWord()->getResultMatchesMathWord();
}

bool BlobData::belongsToRecognizedStopword() {
  if(getParentWord() == NULL) {
    return false;
  }
  return getParentWord()->getResultMatchesStopword();
}

BlobMergeData& BlobData::getMergeData() {
  return mergeData;
}

bool BlobData::belongsToRecognizedNormalRow() {
  if(getParentRow() == NULL) {
    return false;
  }
  return getParentRow()->getIsConsideredNormal();
}

/**
 * Image of this blob (just the blob)
 */
Pix* BlobData::getBlobImage() {
  return blobImage;
}

BlobDataGrid* BlobData::getParentGrid() {
  return parentGrid;
}

