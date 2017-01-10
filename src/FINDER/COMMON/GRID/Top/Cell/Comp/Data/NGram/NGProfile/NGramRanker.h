/**************************************************************************
 * File name:   NGramRanker.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 18, 2013 8:59:46 PM
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
 *              by the overall math to non-math word ratio
 ***************************************************************************/
#ifndef NGRAMRANKER_H
#define NGRAMRANKER_H

#include <SentenceData.h>
#include <NGram.h>
#include <StopwordHelper.h>
#include <fstream>

class NGramRanker {
 public:

  NGramRanker(StopwordFileReader* stopwordHelper);

  ~NGramRanker();

  // Returns all n-grams in a sentence and how often they occur as follows:
  // Returns GenericVector<RankedNGramVec> with 3 items in the following order:
  //    1. Unigram RankedNGramVec
  //    2. Bigram RankedNGramVec
  //    3. Trigram RankedNGramVec
  // The RankedNGramVec is a GenericVector<NGramFrequency*>
  // The NGramFrequency contains both the N-Gram as well as the number of
  // times it appears in the sentence.
  RankedNGramVecs generateSentenceNGrams(
      TesseractSentenceData* sentence,
      const std::string& ngramdir);

  static void destroyNGramVecs(RankedNGramVecs& vecs);

  // writes the uni, bi, and tri, grams for all of the sentences
  // to their respective files under the given path
  void writeNGramFiles(const GenericVector<TesseractSentenceData*>& sentences,
      const std::string& path, std::ofstream* streams);

  // ranks the uni, bi, and tri grams located within the given path
  // assuming the grams each have their own separate file and are named
  // as expected.
  RankedNGramVecs rankNGrams(const std::string& path);

  // Subtracts the counts of matching non-math uni, bi, and tri grams from
  // the corresponding math counts and reranks the math n-grams based on
  // their updated counts. Returns the updated uni, bi, and tri gram math
  // vectors.
  RankedNGramVecs subtractAndReRankNGrams(RankedNGramVecs& math,
      const RankedNGramVecs& nonmath, const std::string& mathdir,
      const double& nonmath_weight);

  static std::string getNGramFileName(const int& gram);

 private:
  // writes just one ngram file (either uni, bi, or tri depending
  // on first argument
  void writeNGramFile(
      const int& gram,
      const GenericVector<TesseractSentenceData*>& sentences,
      const std::string& path, std::ofstream* stream);

  // ranks just one ngram file (either uni, bi, or tri depending
  // on first argument)
  RankedNGramVec rankNGram(int gram, const std::string& path);

  // Subtracts the counts of matching non-math ngrams from the math ngram
  // counts for either uni, bi, or tri grams depending on the first argument
  // Returns the updated math ngrams and their counts
  RankedNGramVec subtractAndReRankNGram(int gram, RankedNGramVec& math,
      const RankedNGramVec& nonmath, const std::string& mathdir,
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
  GenericVector<NGram*> readNGramFile(const std::string& filepath, int gram);

  // returns vector of ngrams where each ngram is unique and
  // also has a frequency count associated with it found from
  // counting the occurences of each
  RankedNGramVec countNGramFrequencies(
      const GenericVector<NGram*>& ngrams);

  // print the counts to a file for analysis
  void writeNGramCounts(const GenericVector<NGramFrequency*> ngfs,
      const std::string& filepath);

  inline void nGramReadError(const std::string& filepath) {
    std::cout << "ERROR: Invalid ngram detected in " << filepath << std::endl;
    assert(false);
  }

  //destroys all the ngrams in the vector
  static void destroyNGramVec(GenericVector<NGramFrequency*> ngramcounts);

  RankedNGramVecs ranked_math; // ranked math n-grams (output of initFeatExtFull)

  StopwordFileReader* stopwordHelper;
};

#endif
