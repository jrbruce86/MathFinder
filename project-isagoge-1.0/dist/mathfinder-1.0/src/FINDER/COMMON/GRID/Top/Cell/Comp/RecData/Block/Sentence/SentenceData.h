/**************************************************************************
 * File name:   TesseractSentenceData.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 5:55:37 PM
 *              Modified Dec 18, 2016
 * ------------------------------------------------------------------------
 * Description: Data-structure to represent a sentence recognized by Tesseract
 *              and extracted from a grid. The sentence is owned by
 *              the grid from which it originates.
 ***************************************************************************/
#ifndef TESSERACTSENTENCEDATA_H
#define TESSERACTSENTENCEDATA_H

//#include <NGram.h>
class NGramFrequency;
class RankedNGram;
class TesseractBlockData;

#include <allheaders.h>
#include <baseapi.h>


class TesseractSentenceData {

 public:

  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;

  TesseractSentenceData(TesseractBlockData* parentBlock,
      const int startRowIndex, const int startWordIndex);

  ~TesseractSentenceData();

  /**
   * Sets the index for the end of this sentence within its block
   * (the row and word index within the parent block) and then
   * copies the words from the block into a text string using
   * the start indices set at construction and the end indices that
   * were set here.
   *
   * Asserts false if called more than once, so only call once
   */
  void readInBlockText(const int endRowIndex, const int endWordIndex);

  int getStartRowIndex();
  int getStartWordIndex();
  int getEndRowIndex();
  int getEndWordIndex();

  void setRowBoxes(Boxa* rowBoxes);
  Boxa* getRowBoxes();

  void setIsMath(bool isMath);
  // true if, during training, a blob exists within the sentence that is
  // within the bounds of a mathematical expression based on the groundtruth.
  // this is inconsequential, however, during prediction and will always be
  // false since during prediction it is unknown whether a sentence is math
  // or not.
  bool getIsMath();

  /**
   * The sentence text
   */
  char* getSentenceText();

  void setNGramCounts(RankedNGramVecs* ngrams);

  // the uni, bi, and tri grams in the sentence and their counts
  RankedNGramVecs* getNGramCounts();

  void setNGramFeatures(GenericVector<double> ngram_features);

  // assigned during feature extraction based on
  // comparison to a "Math N-Gram Profile".
  GenericVector<double> getNGramFeatures();


  TesseractBlockData* parentBlock;

  // indices within parent block
  int startRowIndex; // which row does the sentence start on
  int startWordIndex; // on what word on the start row does the sentence start
  int endRowIndex; // which row does the sentence end on
  int endWordIndex; // on what word on the end row does the sentence end



  bool isMath; // true if, during training, a blob exists within the sentence that is
               // within the bounds of a mathematical expression based on the groundtruth.
               // this is inconsequential, however, during prediction and will always be
               // false since during prediction it is unknown whether a sentence is math
               // or not.
  char* sentence_txt; // the sentence text
  RankedNGramVecs* ngrams; // the uni, bi, and tri grams in the sentence and their counts
  GenericVector<double> ngram_features; // assigned during feature extraction based on
                                        // comparison to a "Math N-Gram Profile".
  Boxa* lineboxes; // an array of boxes where each box is a line of text in the sentence
};

#endif
