/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   NGramProfileGenerator.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 6:32:31 PM
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
 * ------------------------------------------------------------------------
 * This file is part of Project Isagoge.
 *
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/
#ifndef NGRAMPROFILEGENERATOR_H
#define NGRAMPROFILEGENERATOR_H

#define DBG_NGRAM_INIT // comment out to turn off debugging
//#define DBG_NGRAM_INIT_SHOW_SENTENCE_LABELS

#ifdef DBG_NGRAM_INIT
#include <Lept_Utils.h>
#include <fstream>
#endif

#include <NGramRanker.h>
#include <BlobInfoGrid.h>

using namespace tesseract;

class NGramProfileGenerator {
 public:

  NGramProfileGenerator(const string& training_set_path);
  ~NGramProfileGenerator();

  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;

  // Returns the N-Gram Profile. Gets all the
  // N-Grams in an entire training set of images to generate the "N-Gram Profile" that
  // is used for feature extraction. A groundtruth is required so that it can be known
  // which sentences in the training set are math and non-math. The non-math N-Grams are
  // weighted based on the ratio of math to non-math unigrams in the training set and
  // matching non-math n-gram counts are subtracted from the math n-gram ones.
  RankedNGramVecs generateMathNGrams(TessBaseAPI& api, const string& groundtruth_path_,
      const string& training_set_path, const string& ext, bool make_new);


 private:
  // use existing n-grams to avoid the time overhead when I
  // already have the n-grams I need and don't need to remake
  // them each time I run the program. This reads them in from
  // the file that they should have been written to by generateNewNGrams
  // in a previous run.
  RankedNGramVecs readInOldNGrams(const string& ngramdir);

  // create the n-gram profile from scratch using the training data
  RankedNGramVecs generateNewNGrams(TessBaseAPI& api,
      const string& groundtruth_path_, const string& training_set_path,
      const string& ngramdir, const string& ext);

  // returns true if any region of the groundtruth overlaps
  // any linebox of the sentence
  bool isSentenceMath(Sentence* sentence, int imgnum);

  // determines total number of unigrams in mathdir: m_uni
  // then determines total number unigrams in nonmathdir: nm_uni
  // return m_uni/nm_uni
  double findMathNonMathRatio(const string& mathdir, const string& nonmathdir);

  NGramRanker r;
  TessBaseAPI api_;
  string groundtruth_path;
  RankedNGramVecs ranked_math;
  string training_set_path;
  GenericVector<char*> stopwords;

#ifdef DBG_NGRAM_INIT
  // save and optionally display all the labels for sentence both
  // as the text and where that text resides in the image
  void dbgShowSentenceLabels(string path, Pix* curimg,
      const GenericVector<Sentence*>& sentences, BlobInfoGrid* grid,
      string img_name);
  // colors the foreground of all blobs belonging to a sentence
  // (for debugging)
  void colorSentenceBlobs(Pix* im, int sentencenum,
      BlobInfoGrid* grid, LayoutEval::Color color);
  // writes the math and nonmath sentences each to their separate file
  void dbgWriteMathNonMathFiles(const string& training_path,
      const GenericVector<Sentence*>& math_s,
      const GenericVector<Sentence*>& nonmath_s);
  bool dbgfile_open; // for above function
  ofstream* mathfile; // for above function
  ofstream* nonmathfile; // for above function
#endif

};

#endif
