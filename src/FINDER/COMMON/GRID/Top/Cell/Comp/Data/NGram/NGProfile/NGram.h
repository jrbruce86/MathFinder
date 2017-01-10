/**************************************************************************
 * File name:   NGramsGenerator.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 18, 2013 8:59:46 PM
 *              Modified Dec 18, 2016
 * ------------------------------------------------------------------------
 * Description: Data-structure to contain an NGram and its frequency of
 *              occurrence
 ***************************************************************************/

#ifndef NGRAM_H
#define NGRAM_H

#include <iostream>

#include <baseapi.h>

#include <assert.h>

#include <Utils.h>

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
    for(int i = 0; i < words.length(); ++i) {
      if(!Utils::stringCompare(words[i], otherwords[i]))
        return false;
    }
    return true;
  }

  friend std::ostream& operator<< (std::ostream& out, NGram& ng)
  {
    GenericVector<char*> words = ng.words;
    for(int i = 0; i < words.length(); i++)
      out << words[i] << (i+1 == words.length() ? "" : " ");
    return out;
  }

  inline int length() {
    return words.length();
  }

  GenericVector<char*> words;
};

struct NGramFrequency {
  NGram* ngram;
  double frequency;
};

typedef GenericVector<NGramFrequency*> RankedNGramVec;

// Holds all n-grams in a sentence and how often they occur as follows:
 // GenericVector<RankedNGramVec> with 3 items in the following order:
 //    1. Unigram RankedNGramVec
 //    2. Bigram RankedNGramVec
 //    3. Trigram RankedNGramVec
 // The RankedNGramVec is a GenericVector<NGramFrequency*>
 // The NGramFrequency contains both the N-Gram as well as the number of
 // times it appears in the sentence.
typedef GenericVector<RankedNGramVec> RankedNGramVecs;

#endif
