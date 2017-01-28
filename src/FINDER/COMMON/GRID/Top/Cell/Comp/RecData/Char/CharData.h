/*
 * TesseractCharData.h
 *
 *  Created on: Nov 20, 2016
 *      Author: jake
 */

#ifndef TESSERACTCHARDATA_H_
#define TESSERACTCHARDATA_H_

#include <baseapi.h>
#include <ratngs.h>
#include <vector>
#include <string>

class TesseractWordData;
class BlobData;

/**
 * Holds pointers to recognition data from Tesseract corresponding
 * to an individual recognized character. If a blob was determined
 * by Tesseract to be a part of a recognized character, then the blob will include
 * a pointer to an object of this type. Multiple blobs may map to one of object of
 * this type. It shouldn't go the other way around though (wouldn't make sense
 * for a blob to map to multiple characters).. at least I hope not!!! This object
 * includes the recognized character's unicode, Tesseract's confidence
 * in the character's recognition accuracy, it's confidence in the accuracy
 * of the word the character belongs to, and possibly other features.
 *
 * Regarding memory management, objects of this class are not designed to be
 * used unless the Tesseract api used for recognition is still in scope. It
 * mostly just holds immutable pointers and does not do any deep copying.
 * Just makes life easier that way. There may be some exceptions but deep
 * copies will be avoided where ever possible.
 */
class TesseractCharData {

 public:

  /**
   * Constructor
   * Takes a copy of the bounding box for the recognized character
   * as well as the recognized word this character belongs to based on Tesseract
   * All other fields are set after construction
   */
  TesseractCharData(TBOX charResultBoundingBox,
      TesseractWordData* const parentWord);

  ~TesseractCharData();

  /**
   * Setters (these only set the values once.. no-op if you try doing it again)
   */
  TesseractCharData* setRecognitionResultUnicode(const std::string recognitionResultUnicode);
  TesseractCharData* setCharResultInfo(BLOB_CHOICE* charResultInfo);
  TesseractCharData* setDistanceAboveRowBaseline(const double dist);

  /**
   * Getters
   */
  TBOX* getBoundingBox();
  std::string getUnicode();
  BLOB_CHOICE* getCharResultInfo();
  double getDistanceAboveRowBaseline();
  TesseractWordData* getParentWord();
  std::vector<BlobData*>& getBlobs();

  /**
   * Returns true if this character is the leftmost in its word
   */
  bool isLeftmostInWord();

 private:

  TBOX charResultBoundingBox;

  std::string recognitionResultUnicode; // unicode representation of character recognition result

  BLOB_CHOICE* charResultInfo; // tesseract info on the recognition result for this character

  double dist_above_baseline; // the char's distance above its row's baseline

  TesseractWordData* parentWord; // the word this character belongs to based on Tesseract recognition

  std::vector<BlobData*> blobs; // pointers to the raw connected components wrapped in my data structure
};


#endif /* BLOBTESSERACTRECOGNITIONDATA_H_ */
