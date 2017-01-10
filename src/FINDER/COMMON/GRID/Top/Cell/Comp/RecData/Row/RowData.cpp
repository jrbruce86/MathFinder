/*
 * TesseractRowData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <RowData.h>

#include <SentenceData.h>
#include <BlockData.h>
#include <WordData.h>

TesseractRowData::TesseractRowData(ROW_RES* rowRes,
    TesseractBlockData* parentBlockData)
: hasValidTessWord(false), avg_baselinedist((double)0),
  rowIndex(-1), isConsideredNormal(true), valid_word_count(-1) {
  this->rowRes = rowRes;
  this->parentBlockData = parentBlockData;
}

TesseractRowData::~TesseractRowData() {
  rowRes = NULL;
  for(int i = 0; i < wordinfovec.length(); ++i) {
    if(wordinfovec[i] != NULL) {
      delete wordinfovec[i];
      wordinfovec[i] = NULL;
    }
  }
  wordinfovec.clear();
}

ROW_RES* TesseractRowData::getRowRes() {
  return rowRes;
}

WERD_RES_LIST* TesseractRowData::getWordResList() {
  return &(rowRes->word_res_list);
}

std::string TesseractRowData::getRowText() {
  std::string str;
  for(int i = 0; i < wordinfovec.length(); ++i) {
    if(wordinfovec[i]->wordstr() != NULL)
      str += (std::string)((wordinfovec[i]->wordstr()) + (std::string)" ");
  }
  return str;
}

// this provides the bounding box, baseline, and various other
// useful information about the row as found in the tesseract api.
ROW* TesseractRowData::row() {
  return rowRes->row;
}

PARA* TesseractRowData::getParagraph() {
  return row()->para();
}

TBOX TesseractRowData::getBoundingBox() {
  return row()->bounding_box();
}

int TesseractRowData::numValidWords() {
  assert(valid_word_count >= 0);
  return valid_word_count;
}

void TesseractRowData::setValidWordCount(const int wc) {
  valid_word_count = wc;
}

void TesseractRowData::setHasValidTessWord(bool hasValidTessWord) {
  this->hasValidTessWord = hasValidTessWord;
}


GenericVector<TesseractWordData*>& TesseractRowData::getTesseractWords() {
  return wordinfovec;
}

/**
 * True if this row has a valid word on it according to Tesseract
 */
bool TesseractRowData::getHasValidTessWord() {
  return hasValidTessWord;
}

void TesseractRowData::setIsConsideredNormal(bool isConsideredNormal) {
  this->isConsideredNormal = isConsideredNormal;
}

bool TesseractRowData::getIsConsideredNormal() {
  return isConsideredNormal;
}

TesseractBlockData* TesseractRowData::getParentBlock() {
  return parentBlockData;
}

