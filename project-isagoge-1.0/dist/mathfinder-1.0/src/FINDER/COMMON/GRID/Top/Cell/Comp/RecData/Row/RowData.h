/*
 * TesseractRowData.h
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#ifndef TESSERACTROWDATA_H_
#define TESSERACTROWDATA_H_

#include <stddef.h>

#include <assert.h>

#include <pageres.h>
#include <baseapi.h>
#include <string>

class TesseractWordData;
class TesseractBlockData;

//enum ROW_TYPE {NORMAL, ABNORMAL};
struct TesseractRowData {

  TesseractRowData(ROW_RES* rowRes, TesseractBlockData* parentBlockData);

  ~TesseractRowData();

  std::string getRowText();

  // this provides the bounding box, baseline, and various other
  // useful information about the row as found in the tesseract api.
  ROW* row();

  PARA* getParagraph();

  TBOX getBoundingBox();

  int numValidWords();

  void setValidWordCount(const int wc);

  void setHasValidTessWord(bool hasValidTessWord);

  bool getHasValidTessWord();

  void setIsConsideredNormal(bool isConsideredNormal);

  bool getIsConsideredNormal();

  ROW_RES* getRowRes();

  // unmodified words found directly by the tesseract api
  WERD_RES_LIST* getWordResList();

  GenericVector<TesseractWordData*>& getTesseractWords();

  TesseractBlockData* getParentBlock();

  double avg_baselinedist; // average distance of a blob from
                           // the row's baseline (only non-zero if
                           // row has at leat one valid word.

  int rowIndex; // index of this row within its block

  bool hasValidTessWord;


  GenericVector<TesseractWordData*> wordinfovec;


  bool isConsideredNormal;
  int valid_word_count; // number of valid words on this row

  ROW_RES* rowRes; // tesseract api information on the row
  TesseractBlockData* parentBlockData;

 private:


};



#endif /* TESSERACTROWDATA_H_ */
