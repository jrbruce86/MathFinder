/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext1.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 21, 2013 9:16:32 PM
 * ------------------------------------------------------------------------
 * Description: Implements the FeatureExtractor interface.
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
#ifndef F_EXT1_H
#define F_EXT1_H

#define DBG_NGRAM_INIT // comment out to turn off debugging
//#define DBG_NGRAM_INIT_SHOW_SENTENCE_LABELS

#include <IFeatureExtractor.h>
#include <vector>

#ifdef DBG_NGRAM_INIT
#include <Lept_Utils.h>
#include <fstream>
#endif

using namespace tesseract;


class NGram {
 public:
  ~NGram() {
    for(int i = 0; i < words.length(); i++) {
      char* wrd = words[i];
      if(wrd != NULL) {
        delete [] wrd;
        wrd = NULL;
      }
    }
    words.clear();
  }

  inline bool operator ==(const NGram& other) {
    GenericVector<char*> otherwords = other.words;
    assert(words.length() == otherwords.length());
    bool are_equal = true;
    for(int i = 0; i < words.length(); ++i) {
      if(!Basic_Utils::stringCompare(words[i], otherwords[i]))
        return false;
    }
    return true;
  }

  friend ostream& operator<< (ostream& out, NGram& ng)
  {
    GenericVector<char*> words = ng.words;
    for(int i = 0; i < words.length(); i++)
      out << words[i] << (i+1 == words.length() ? "" : " ");
    return out;
  }

  GenericVector<char*> words;
};

struct NGramFrequency {
  NGram* ngram;
  double frequency;
};


class F_Ext1 {
 public:

  F_Ext1();
  void initFeatExtFull(TessBaseAPI& api, const string& groundtruth_path,
      const string& training_set_path, const string& ext);
  void initFeatExtSinglePage();
  std::vector<double> extractFeatures(tesseract::BLOBINFO* blob,
      tesseract::BlobInfoGrid* big_);
 private :
  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;

  // returns true if any region of the groundtruth overlaps
  // any linebox of the sentence
  bool isSentenceMath(Sentence* sentence, int imgnum);

  // writes the uni, bi, and tri, grams for all of the sentences
  // to their respective files under the given path
  void writeNGramFiles(const GenericVector<Sentence*>& sentences,
      const string& path, ofstream* streams);

  // writes just one ngram file (either uni, bi, or tri depending
  // on first argument
  void writeNGramFile(int gram, const GenericVector<Sentence*>& sentences,
      const string& path, ofstream* stream);

  // ranks the uni, bi, and tri grams located within the given path
  // assuming the grams each have their own separate file and are named
  // as expected.
  RankedNGramVecs rankNGrams(const string& path);

  // ranks just one ngram file (either uni, bi, or tri depending
  // on first argument)
  RankedNGramVec rankNGram(int gram, const string& path);

  // determines total number of unigrams in mathdir: m_uni
  // then determines total number unigrams in nonmathdir: nm_uni
  // return m_uni/nm_uni
  double findMathNonMathRatio(const string& mathdir, const string& nonmathdir);

  // Subtracts the counts of matching non-math uni, bi, and tri grams from
  // the corresponding math counts and reranks the math n-grams based on
  // their updated counts. Returns the updated uni, bi, and tri gram math
  // vectors.
  RankedNGramVecs subtractAndReRankNGrams(RankedNGramVecs& math,
      const RankedNGramVecs& nonmath, const string& mathdir,
      const double& nonmath_weight);

  // Subtracts the counts of matching non-math ngrams from the math ngram
  // counts for either uni, bi, or tri grams depending on the first argument
  // Returns the updated math ngrams and their counts
  RankedNGramVec subtractAndReRankNGram(int gram, RankedNGramVec& math,
      const RankedNGramVec& nonmath, const string& mathdir,
      const double& nonmath_weight);

  // comparator used in sorting n-grams in increasing count order
  inline static int sortcmp(const void* ngf1, const void* ngf2) {
    NGramFrequency* n1 = *((NGramFrequency**)ngf1);
    NGramFrequency* n2 = *((NGramFrequency**)ngf2);
    if(n1->frequency < n2->frequency)
      return 1;
    else if(n1->frequency > n2->frequency)
      return -1;
    else
      return 0;
  }

  // reads in all the n-grams in a file assumes a file only has
  // a list of one n-gram type (uni/bi/tri). throws exception if
  // an unexpected number of grams are detected. returns the vector
  // of n-grams, each n-gram is a vector of words (no more than three)
  GenericVector<NGram*> readNGramFile(const string& filepath, int gram);

  // returns vector of ngrams where each ngram is unique and
  // also has a frequency count associated with it found from
  // counting the occurences of each
  RankedNGramVec countNGramFrequencies(
      const GenericVector<NGram*>& ngrams);

  // print the counts to a file for analysis
  void writeNGramCounts(const GenericVector<NGramFrequency*> ngfs,
      const string& filepath);

  inline void nGramReadError(const string& filepath) {
    cout << "ERROR: Invalid ngram detected in " << filepath << endl;
    exit(EXIT_FAILURE);
  }

  inline string getNGramFileName(int gram) {
    string ngramfile = "";
    if(gram == 1)
      ngramfile = unigram_filename;
    else if(gram == 2)
      ngramfile = bigram_filename;
    else if(gram == 3)
      ngramfile = trigram_filename;
    else {
      cout << "ERROR: Attempt made to find n-grams other than the uni, bi, and tri"
           << " which are the only ones supported.\n";
      exit(EXIT_FAILURE);
    }
    return ngramfile;
  }

  string unigram_filename;
  string bigram_filename;
  string trigram_filename;

  //destroys all the ngrams in the vector
  void destroyNGramVec(GenericVector<NGramFrequency*> ngramcounts);

#ifdef DBG_NGRAM_INIT
  // save and optionally display all the labels for sentence both
  // as the text and where that text resides in the image
  void dbgShowSentenceLabels(Pix* curimg,
      const GenericVector<Sentence*>& sentences, BlobInfoGrid* grid,
      string img_name);
  // colors the foreground of all blobs belonging to a sentence
  // (for debugging)
  void colorSentenceBlobs(Pix* im, int sentencenum,
      BlobInfoGrid* grid, LayoutEval::Color color);
  // writes the math and nonmath sentences each to their separate file
  void dbgWriteMathNonMathFiles(const GenericVector<Sentence*>& math_s,
      const GenericVector<Sentence*>& nonmath_s);
  bool dbgfile_open; // for above function
  ofstream* mathfile; // for above function
  ofstream* nonmathfile; // for above function
#endif

  string groundtruth_path;
};

#endif
