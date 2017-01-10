/*
 * TesseractSentenceData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <SentenceData.h>

#include <RowData.h>
#include <WordData.h>
#include <BlockData.h>
#include <NGramRanker.h>

#include <assert.h>

TesseractSentenceData::TesseractSentenceData(TesseractBlockData* parentBlock,
    const int startRowIndex, const int startWordIndex)
: endRowIndex(-1), endWordIndex(-1), isMath(false),
  sentence_txt(NULL), ngrams(NULL),
  lineboxes(NULL) {
  this->parentBlock = parentBlock;
  this->startRowIndex = startRowIndex;
  this->startWordIndex = startWordIndex;
}

TesseractSentenceData::~TesseractSentenceData() {
  if(lineboxes != NULL) {
    boxaDestroy(&lineboxes);
    lineboxes = NULL;
  }
  if(sentence_txt != NULL) {
    delete [] sentence_txt;
    sentence_txt = NULL;
  }
  if(ngrams != NULL) {
    NGramRanker::destroyNGramVecs(*ngrams);
    delete ngrams;
    ngrams = NULL;
  }
}

void TesseractSentenceData::readInBlockText(const int endRowIndex, const int endWordIndex) {
  assert(this->endRowIndex == -1 && this->endWordIndex == -1); // Prevent calling more than once
  this->endRowIndex = endRowIndex;
  this->endWordIndex = endWordIndex;

  // determine the size
  const int startln = startRowIndex;
  const int endln = endRowIndex;
  const int startwrd = startWordIndex;
  const int endwrd = endWordIndex;
  assert(startln > -1 && endln > -1 && startwrd > -1
      && endwrd > -1 && sentence_txt == NULL);
  int charcount = 0;
  // first need to count the characters
  for(int i = startln; i <= endln; ++i) {
    TesseractRowData* const rowinfo = parentBlock->getTesseractRows()[i];
    GenericVector<TesseractWordData*>& words = rowinfo->getTesseractWords();
    int start = 0, end = words.length() - 1;
    if(i == startln)
      start = startwrd;
    if(i == endln)
      end = endwrd;
    for(int j = start; j <= end; ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      charcount += strlen(wordstr);
      // add room for space and new line
      ++charcount;
    }
  }

  // allocate the memory
  sentence_txt = new char[charcount+1];

  // copy everything over
  int charindex = 0;
  for(int i = startln; i <= endln; ++i) {
    TesseractRowData* const rowinfo = parentBlock->getTesseractRows()[i];
    GenericVector<TesseractWordData*>& words = rowinfo->getTesseractWords();
    int start = 0, end = words.length() - 1;
    if(i == startln)
      start = startwrd;
    if(i == endln)
      end = endwrd;
    for(int j = start; j <= end; ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      for(int k = 0; k < strlen(wordstr); ++k) {
        sentence_txt[charindex++] = wordstr[k];
      }
      if(j != end)
        sentence_txt[charindex++] = ' ';
      else
        sentence_txt[charindex++] = '\n';
    }
  }
  sentence_txt[charcount] = '\0';
}

int TesseractSentenceData::getStartRowIndex() {
  return startRowIndex;
}

int TesseractSentenceData::getStartWordIndex() {
  return startWordIndex;
}

int TesseractSentenceData::getEndRowIndex() {
  return endRowIndex;
}

int TesseractSentenceData::getEndWordIndex() {
  return endWordIndex;
}

void TesseractSentenceData::setRowBoxes(Boxa* const rowBoxes) {
  this->lineboxes = rowBoxes;
}

Boxa* TesseractSentenceData::getRowBoxes() {
  return lineboxes;
}

void TesseractSentenceData::setIsMath(bool isMath) {
  this->isMath = isMath;
}

bool TesseractSentenceData::getIsMath() {
  return isMath;
}

char* TesseractSentenceData::getSentenceText() {
  return sentence_txt;
}

void TesseractSentenceData::setNGramCounts(RankedNGramVecs* ngrams) {
  this->ngrams = ngrams;
}

// the uni, bi, and tri grams in the sentence and their counts
RankedNGramVecs* TesseractSentenceData::getNGramCounts() {
  return ngrams;
}

void TesseractSentenceData::setNGramFeatures(GenericVector<double> ngram_features) {
  this->ngram_features = ngram_features;
}

// assigned during feature extraction based on
// comparison to a "Math N-Gram Profile".
GenericVector<double> TesseractSentenceData::getNGramFeatures() {
  return ngram_features;
}
