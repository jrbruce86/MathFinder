/**************************************************************************
 * File name:   NGramProfileGenerator.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 6:32:31 PM
 *              Modified Dec 18, 2016
 * ------------------------------------------------------------------------
 * Description: Takes a set of input document images as input and generates
 *              a set of ranked uni-grams, bi-grams, and tri-grams that
 *              have been counted the most in mathematical regions of the
 *              images. Matching non-mathematical N-Grams are subtracted
 *              from the math ones so that the ranking best represents the
 *              mathematical regions. Also stop-words, digits, and punctuation
 *              are removed. Since math and nonmath words are seen in different
 *              amounts the subtraction weights each matching non-math n-gram
 *              by the overall math to non-math word ratio. The resulting math
 *              n-grams are referred to as the NGram Profile for the training
 *              data.
 ***************************************************************************/
#ifndef NGRAMPROFILEGENERATOR_H
#define NGRAMPROFILEGENERATOR_H

//#define DBG_NGRAM_INIT // comment out to turn off debugging
//#define DBG_NGRAM_INIT_SHOW_SENTENCE_LABELS


#ifdef DBG_NGRAM_INIT
#include <Lept_Utils.h>
#include <fstream>
#endif

#include <FinderInfo.h>
#include <NGramRanker.h>
#include <NGram.h>
#include <SentenceData.h>
#include <BlobDataGrid.h>
#include <BlockData.h>

#include <baseapi.h>

#include <string>
#include <iostream>

class NGramProfileGenerator {
 public:

  NGramProfileGenerator(
      FinderInfo* const finderInfo,
      NGramRanker* const ngramRanker,
      const std::string& ngramdir);

  ~NGramProfileGenerator();

  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;

  // Returns the N-Gram Profile. Gets all the
  // N-Grams in an entire training set of images to generate the "N-Gram Profile" that
  // is used for feature extraction. A groundtruth is required so that it can be known
  // which sentences in the training set are math and non-math. The non-math N-Grams are
  // weighted based on the ratio of math to non-math unigrams in the training set and
  // matching non-math n-gram counts are subtracted from the math n-gram ones.
  RankedNGramVecs generateMathNGrams();

 private:

  FinderInfo* finderInfo;
  NGramRanker* ngramRanker;
  std::string ngramdir;
  std::string groundtruthFilePath;

  // use existing n-grams to avoid the time overhead when I
  // already have the n-grams I need and don't need to remake
  // them each time I run the program. This reads them in from
  // the file that they should have been written to by generateNewNGrams
  // in a previous run.
  RankedNGramVecs readInOldNGrams(const std::string& ngramdir);

  // create the n-gram profile from scratch using the training data
  RankedNGramVecs generateNewNGrams();

  // returns true if any region of the groundtruth overlaps
  // any linebox of the sentence
  bool isSentenceMath(TesseractSentenceData* sentence, int imgnum);

  // determines total number of unigrams in mathdir: m_uni
  // then determines total number unigrams in nonmathdir: nm_uni
  // return m_uni/nm_uni
  double findMathNonMathRatio(const std::string& mathdir, const std::string& nonmathdir);

#ifdef DBG_NGRAM_INIT
  // save and optionally display all the labels for sentence both
  // as the text and where that text resides in the image
  void dbgShowSentenceLabels(Pix* const curimg,
      BlobDataGrid* const grid,
      std::string img_name);
  // colors the foreground of all blobs belonging to a sentence
  // (for debugging)
  void colorSentenceBlobs(Pix* const im, const int sentencenum,
      TesseractBlockData* const block,
      BlobDataGrid* const grid, const LayoutEval::Color color);
  // writes the math and nonmath sentences each to their separate file
  void dbgWriteMathNonMathFiles(
      const GenericVector<TesseractSentenceData*>& math_s,
      const GenericVector<TesseractSentenceData*>& nonmath_s);
#endif

};

#endif
