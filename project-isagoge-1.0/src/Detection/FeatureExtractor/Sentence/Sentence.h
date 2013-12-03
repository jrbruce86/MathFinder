/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   NGramsGenerator.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 27, 2013 5:55:37 PM
 * ------------------------------------------------------------------------
 * Description: Data-structure to represent a sentence recognized by Tesseract
 *              and extracted from a grid. The sentence is owned by
 *              the grid from which it originates.
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
#ifndef SENTENCE_H
#define SENTENCE_H

#include <NGram.h>

struct Sentence {
  typedef GenericVector<NGramFrequency*> RankedNGramVec;
  typedef GenericVector<RankedNGramVec> RankedNGramVecs;
  Sentence() : start_line_num(-1), startchar_index(-1), startwrd_index(-1),
      end_line_num(-1), endchar_index(-1), endwrd_index(-1), ismath(false),
      sentence_txt(NULL), ngram_features(NULL), lineboxes(NULL), ngrams(NULL) {}

  ~Sentence() {
    if(ngram_features != NULL) {
      ngram_features->clear();
      delete ngram_features;
      ngram_features = NULL;
    }
    if(lineboxes != NULL) {
      boxaDestroy(&lineboxes);
      lineboxes = NULL;
    }
    if(sentence_txt != NULL) {
      delete [] sentence_txt;
      sentence_txt = NULL;
    }
    if(ngrams != NULL) {
      cout << "ERROR: The N-Grams in a sentence must be destroyed before the "
           << "sentence is destroyed!\n";
      exit(EXIT_FAILURE);
    }
  }

  int start_line_num; // which line does the sentence start on
  int startchar_index; // where on the start line does the sentence start
  int startwrd_index; // on what word on the start line does the sentence start
  int end_line_num; // which line does the sentence end on
  int endchar_index; // where on the end line does the sentence end
  int endwrd_index; // on what word on the end line does the sentence end
  bool ismath; // true if, during training, a blob exists within the sentence that is
               // within the bounds of a mathematical expression based on the groundtruth.
               // this is inconsequential, however, during prediction and will always be
               // false since during prediction it is unknown whether a sentence is math
               // or not.
  char* sentence_txt; // the sentence text
  RankedNGramVecs* ngrams; // the uni, bi, and tri grams in the sentence and their counts
  GenericVector<double>* ngram_features; // assigned during feature extraction based on
                                        // comparison to a "Math N-Gram Profile".
  Boxa* lineboxes; // an array of boxes where each box is a line of text in the sentence
};

#endif
