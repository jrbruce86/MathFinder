/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   NGramsGenerator.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 18, 2013 8:59:46 PM
 * ------------------------------------------------------------------------
 * Description: Data-structure to contain an NGram and its frequency of
 *              occurrence
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

#ifndef NGRAM_H
#define NGRAM_H

#include <iostream>
using namespace std;

#include <baseapi.h>
#include <Basic_Utils.h>
#include <assert.h>

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

  inline int length() {
    return words.length();
  }

  GenericVector<char*> words;
};

struct NGramFrequency {
  NGram* ngram;
  double frequency;
};

#endif
