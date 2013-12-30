/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   NGramRanker.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 18, 2013 8:59:46 PM
 * ------------------------------------------------------------------------
 * Description: Takes a set of input document images as input and generates
 *              a set of ranked uni-grams, bi-grams, and tri-grams that
 *              have been counted the most in mathematical regions of the
 *              images. Matching non-mathematical N-Grams are subtracted
 *              from the math ones so that the ranking best represents the
 *              mathematical regions. Also stop-words, digits, and punctuation
 *              are removed. Since math and nonmath words are seen in different
 *              amounts the subtraction weights each matching non-math n-gram
 *              by the overall math to non-math word ratio
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
#ifndef NGRAMRANKER_H
#define NGRAMRANKER_H

#include <Sentence.h>
#include <NGram.h>
#include <fstream>

using namespace tesseract;

class NGramRanker {
 public:
  NGramRanker() : unigram_filename("uni-grams"),
      bigram_filename("bi-grams"), trigram_filename("tri-grams"),
      api_(NULL), owns_stop_words(true) {};

//  NGramRanker(const string&); // TODO: remove this

  NGramRanker(const string&, GenericVector<char*>);

  ~NGramRanker();

  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;

  // Returns all n-grams in a sentence and how often they occur as follows:
  // Returns GenericVector<RankedNGramVec> with 3 items in the following order:
  //    1. Unigram RankedNGramVec
  //    2. Bigram RankedNGramVec
  //    3. Trigram RankedNGramVec
  // The RankedNGramVec is a GenericVector<NGramFrequency*>
  // The NGramFrequency contains both the N-Gram as well as the number of
  // times it appears in the sentence.
  RankedNGramVecs generateSentenceNGrams(Sentence* sentence);



  inline void destroyNGramVecs(RankedNGramVecs& vecs) {
    for(int i = 0; i < vecs.length(); i++)
      destroyNGramVec(vecs[i]);
    vecs.clear();
  }

  inline void setTessAPI(TessBaseAPI* api) {
    api_ = api;
  }

  // writes the uni, bi, and tri, grams for all of the sentences
  // to their respective files under the given path
  void writeNGramFiles(const GenericVector<Sentence*>& sentences,
      const string& path, ofstream* streams);

  // ranks the uni, bi, and tri grams located within the given path
  // assuming the grams each have their own separate file and are named
  // as expected.
  RankedNGramVecs rankNGrams(const string& path);

  // Subtracts the counts of matching non-math uni, bi, and tri grams from
  // the corresponding math counts and reranks the math n-grams based on
  // their updated counts. Returns the updated uni, bi, and tri gram math
  // vectors.
  RankedNGramVecs subtractAndReRankNGrams(RankedNGramVecs& math,
      const RankedNGramVecs& nonmath, const string& mathdir,
      const double& nonmath_weight);

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


  void readInStopWords(const string& training_set_path);

  // returns a new copy of the stop words which will not be owned
  // by this class. destruction of this copy is required by the
  // caller.
  inline GenericVector<char*> getStopWordsCopy() {
    GenericVector<char*> newstopwords;
    assert(stopwords.length() > 0); // if this is empty then caller did something wrong
    for(int i = 0; i < stopwords.length(); ++i) {
      char* newstpwrd = Basic_Utils::strCopy(stopwords[i]);
      newstopwords.push_back(newstpwrd);
    }
    return newstopwords;
  }

  // simply returns the pointers without copying, if this is
  // used then the caller has no ownership but can't access the
  // stop words after the ranker's destruction.
  inline GenericVector<char*> getStopWords() {
    return stopwords;
  }

  inline bool isStrStopWord(const char* const str) {
    if(!str)
      return false;
    return isStopWord(str, 1);
  }

 private:
  // writes just one ngram file (either uni, bi, or tri depending
  // on first argument
  void writeNGramFile(int gram, const GenericVector<Sentence*>& sentences,
      const string& path, ofstream* stream);

  // returns true if gram == 1 (unigram) and the word is a
  // stopword on the stopword list
  bool isStopWord(const char* const word, int gram);

  // ranks just one ngram file (either uni, bi, or tri depending
  // on first argument)
  RankedNGramVec rankNGram(int gram, const string& path);

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

  string unigram_filename;
  string bigram_filename;
  string trigram_filename;

  //destroys all the ngrams in the vector
  void destroyNGramVec(GenericVector<NGramFrequency*> ngramcounts);

  string groundtruth_path;

  GenericVector<char*> stopwords;
  bool owns_stop_words; // this class may or may not own the stop words depending on how it is used

  TessBaseAPI* api_;

  RankedNGramVecs ranked_math; // ranked math n-grams (output of initFeatExtFull)

  string training_set_path;
};

#endif
