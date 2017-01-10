/*
 * TesseractWordData.h
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#ifndef TESSERACTWORDDATA_H_
#define TESSERACTWORDDATA_H_

#include <stddef.h>
#include <baseapi.h>
#include <ratngs.h>
#include <vector>

class TesseractCharData;
class TesseractRowData;
class TesseractBlockData;

struct TesseractWordData {

  TesseractWordData(TBOX boundingBox,
      WERD_RES* const wordRes,
      TesseractRowData* const parentRowData);

  ~TesseractWordData();

  const char* wordstr();

  WERD_CHOICE* bestchoice();

  WERD* word();

  void setIsValidTessWord(bool isValidTessWord);

  bool getIsValidTessWord();

  std::vector<TesseractCharData*>& getTesseractChars();

  TBOX getBoundingBox();

  void setSentenceIndex(const int index);
  /**
   * Gets the index of the sentence recognized by Tesseract that this
   * word belongs to if it does belong to a sentence. This is the index
   * into the list of sentence contained in this word's parent block.
   */
  int getSentenceIndex();

  TesseractRowData* getParentRow();

  TesseractBlockData* getParentBlock();

  void setResultMatchesMathWord(bool matchesMathWord);
  bool getResultMatchesMathWord();

  WERD_RES* getWordRes();

  void setResultMatchesStopword(bool isStopword);
  bool getResultMatchesStopword();

  std::vector<TesseractCharData*> getChildChars();

 private:

  TBOX boundingBox; // the rectangular coords of the word on the page
  WERD_RES* wordRes;

  TesseractRowData* parentRow;

  std::vector<TesseractCharData*> tesseractChars; // top down access
  int sentenceIndex;
  bool isValidTessWord; // true if the word is considered valid according to Tesseract

  bool resultMatchesMathWord;
  bool resultMatchesStopword;
};


#endif /* TESSERACTWORDDATA_H_ */
