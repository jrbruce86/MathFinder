/*
 * BlobData.h
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#ifndef BLOBDATA_H_
#define BLOBDATA_H_

#include <BlobMergeData.h>


#include <BlobDataGrid.h>
#include <BlobFeatExtData.h>

#include <baseapi.h>
#include <tesseractclass.h>

#include <vector>

#include <CharData.h>

class TesseractWordData;
class TesseractRowData;
class TesseractBlockData;

/**
 * A container for extracted data corresponding to an individual blob (group of connected
 * pixels). While the contents of this container will vary depending on which features are
 * extracted, every instance of this object will contain information on the corresponding
 * blob's bounding box dimensions, recognition results from Tesseract, location on the
 * page on which it originated, and extracted features as a vector of doubles.
 */
class BlobData;
ELISTIZEH (BlobData)
class BlobData: public ELIST_LINK {
 public:
  BlobData(TBOX box, PIX* blobImage, BlobDataGrid* parentGrid);

  /**
   * Deconstructor
   * Must delete the following:
   * - characterRecognitionData
   */
  ~BlobData();

  // Required for displaying the grid
  ScrollView::Color BoxColor() const {
    return ScrollView::WHITE;
  }

  // required by tesseract
  inline inT16 left() const {
    return box.left();
  }
  inline inT16 bottom() const {
    return box.bottom();
  }
  inline inT16 right() const {
    return box.right();
  }
  inline inT16 top() const {
    return box.top();
  }

  /**
   * Sets this blob's character recognition data found by Tesseract.
   * There is possibly a one-to-many relationship between a recognition
   * result of Tesseract and the blobs in the image. Thus several neighboring
   * characters may share this same value. This is because Tesseract may merge
   * multiple blobs together to form one character (as with broken characters
   * due to low image quality or symbols like '=').
   */
  void setCharacterRecognitionData(TesseractCharData* const tesseractCharRecognitionData);

  /**
   * Gets this blob's character recognition data found by Tesseract.
   * There is possibly a one-to-many relationship between a recognition
   * result of Tesseract and the blobs in the image. Thus several neighboring
   * characters may share this same value. This is because Tesseract may merge
   * multiple blobs together to form one character (as with broken characters
   * due to low image quality or symbols like '=').
   */
  TesseractCharData* getParentChar();
  TesseractWordData* getParentWord();
  const char* getParentWordstr();
  TesseractRowData* getParentRow();
  TesseractBlockData* getParentBlock();

  /**
   * Gets the features extracted by all feature extractors that
   * were run on this blob. There should be no need to call this
   * method until all of the desired feature extractors have been run.
   */
  std::vector<DoubleFeature*> getExtractedFeatures();

  /**
   * Gets the size of the variable data vector
   */
  int getVariableDataLength();

  /**
   * Looks up an entry in the variable data vector and
   * returns it as a char*. It is up to the feature extractor
   * to know the index and cast the returned pointer appropriately.
   */
  BlobFeatureExtractionData* getVariableDataAt(const int& i);

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
  int appendNewVariableData(BlobFeatureExtractionData* const data);

  /**
   * Appends the provided vector of features to this blob entry's array
   * of features. Once finalized, this array should contain features for
   * all feature extractions that were carried out.
   */
  void appendExtractedFeatures(std::vector<DoubleFeature*> features);

  /**
   * Returns reference to an immutable version of this blob's bounding box
   */
  const TBOX& getBoundingBox() const;

  /**
   * Sets the result of math expression detection (should be set by the detector)
   */
  void setMathExpressionDetectionResult(bool mathExpressionDetectionResult);

  /**
   * Gets the result of math expression detection assuming it has been carried out.
   * Would return false no matter what if no detection has been carried out
   */
  bool getMathExpressionDetectionResult();

  /**
   * Returns true if this blob belongs to a recognized character belonging to a word
   * recognized as being valid by Tesseract during OCR
   */
  bool belongsToRecognizedWord();

  /**
   * Returns true if this blob belongs to a recognized character belonging to a word
   * recognized as being valid by Tesseract during OCR that also matches a word from a
   * finite list of math words.
   */
  bool belongsToRecognizedMathWord();

  /**
   * Returns true if this blob belongs to a recognized character belonging to a word
   * recognized as beling valid by Tesseract during OCR that also matches a word from a
   * finite list of stopwords.
   */
  bool belongsToRecognizedStopword();

  /**
   * Returns true if this blob belongs to a row of text recognized during
   * OCR is being part of a paragraph or other body of text as opposed to
   * being displayed separately.
   */
  bool belongsToRecognizedNormalRow();

  /**
   * Returns true if this blob was recognized by Tesseract as belonging to the
   * right-most character in a word that is considered 'valid' based on Tesseract's
   * dictionary (has a matching entry in the dictionary).
   */
  bool isRightmostInWord();

  /**
   * Returns true if this blob was recognized by Tesseract as belonging to the
   * left-most character in a word that is considered 'valid' based on Tesseract's
   * dictionary (has a matching entry in the dictionary).
   */
  bool isLeftmostInWord();

  BlobMergeData& getMergeData();

  TBOX bounding_box() const;

  Pix* getBlobImage();

  BlobDataGrid* getParentGrid();

  /**
   * The confidence Tesseract has in the character result that it recognized
   * this blob as being a part of
   */
  float getCharRecognitionConfidence();

  /**
   * The confidence Tesseract has in the word result that it recognized
   * this blob as being a part of (this is the lowest confidence for all
   * character results in the word
   */
  float getWordRecognitionConfidence();

  /**
   * The average confidence among all character results in the word that
   * this blob was recognized as being a part of.
   */
  float getWordAvgRecognitionConfidence();

  /**
   * Gets font info from the parent if that info exists.
   * If not returns NULL.
   */
  FontInfo* getFontInfo();

  // Markers solely for during grid creation stage and/or debugging
  void markForDeletion();
  bool isMarkedForDeletion();
  void markAsTesseractSplit();
  bool isMarkedAsTesseractSplit();

 private:
  TBOX box;

  Pix* blobImage;

  BlobDataGrid* parentGrid;

  BlobMergeData mergeData;

  TesseractCharData* tesseractCharData;

  // Data structures which vary depending on the features extracted
  // Based on feature extractor used, each entry is cast to its class type
  std::vector<BlobFeatureExtractionData*> variableExtractionData;

  // This blob entry's array of extracted features. There should be at least
  // one entry for each feature extractor run on this blob (some feature extractors
  // extract more than one feature).
  // WARNING: This is only to be modified from the main feature extractor. The
  //          blob feature extractors should never modify this data. They must add
  //          their features to blob's data entry which they create. The main feature
  //          extractor pulls the features out of each of those and compiles them into
  //          a complete list that is sent into the binary classification.
  std::vector<DoubleFeature*> extractedFeatures;

  bool mathExpressionDetectionResult;

  // the lowest possible certainty of a blob defined by Tesseract
  // (should be defined to -20 in the init list)
  const float minTesseractCertainty;

  // markers for grid creation/debugging
  bool markedAsTesseractSplit;
  bool markedForDeletion;
};

#endif /* BLOBDATA_H_ */
