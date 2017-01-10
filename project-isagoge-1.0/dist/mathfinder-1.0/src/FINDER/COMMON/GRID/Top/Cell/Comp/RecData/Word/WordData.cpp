/*
 * TesseractWordData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <WordData.h>
#include <RowData.h>
#include <CharData.h>
#include <BlockData.h>

#include <stddef.h>

TesseractWordData::TesseractWordData(TBOX boundingBox,
    WERD_RES* const wordRes,
    TesseractRowData* const parentRowData)
: sentenceIndex(-1), isValidTessWord(false),
  resultMatchesMathWord(false) {
  this->boundingBox = boundingBox;
  this->wordRes = wordRes;
  this->parentRow = parentRowData;
}

TesseractWordData::~TesseractWordData() {
  parentRow = NULL;
  wordRes = NULL;
  for(int i = 0; i < tesseractChars.size(); ++i)
    tesseractChars[i] = NULL;
  tesseractChars.clear();
}

const char* TesseractWordData::wordstr() {
  if(bestchoice() != NULL)
    return bestchoice()->unichar_string().string();
  return NULL;
}

WERD_CHOICE* TesseractWordData::bestchoice() {
  if(wordRes != NULL)
    return wordRes->best_choice;
  return NULL;
}

WERD* TesseractWordData::word() {
  if(!wordRes)
    return NULL;
  return wordRes->word;
}


void TesseractWordData::setIsValidTessWord(bool isValidTessWord) {
  this->isValidTessWord = isValidTessWord;
}

bool TesseractWordData::getIsValidTessWord() {
  return isValidTessWord;
}

std::vector<TesseractCharData*>& TesseractWordData::getTesseractChars() {
  return tesseractChars;
}

TBOX TesseractWordData::getBoundingBox() {
  return boundingBox;
}

void TesseractWordData::setSentenceIndex(const int sentenceIndex) {
  this->sentenceIndex = sentenceIndex;
}

int TesseractWordData::getSentenceIndex() {
  return sentenceIndex;
}

TesseractRowData* TesseractWordData::getParentRow() {
  return parentRow;
}

TesseractBlockData* TesseractWordData::getParentBlock() {
  if(parentRow == NULL) {
    return NULL;
  }
  return parentRow->getParentBlock();
}

void TesseractWordData::setResultMatchesMathWord(bool matchesMathWord) {
  this->resultMatchesMathWord = matchesMathWord;
}

/**
 * True if this word matches up with a word known to be math-oriented
 */
bool TesseractWordData::getResultMatchesMathWord() {
  return resultMatchesMathWord;
}

WERD_RES* TesseractWordData::getWordRes() {
  return wordRes;
}

void TesseractWordData::setResultMatchesStopword(bool resultMatchesStopword) {
  this->resultMatchesStopword = resultMatchesStopword;
}

bool TesseractWordData::getResultMatchesStopword() {
  return resultMatchesStopword;
}

std::vector<TesseractCharData*> TesseractWordData::getChildChars() {
  return tesseractChars;
}

