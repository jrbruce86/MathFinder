/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext1.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 21, 2013 9:16:32 PM
 * ------------------------------------------------------------------------
 * Description: Implements the FeatureExtractor interface. Details on
 *              this particular implementation are described in the
 *              thesis.
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

#include <vector>
#include <NGramRanker.h>
#include <NGProfileGen.h>


using namespace tesseract;

enum Direction {LEFT, RIGHT, UP, DOWN};
enum SubSuperScript {SUB, SUPER};

class F_Ext1 {
 public:
  F_Ext1();

  virtual ~F_Ext1();

  void initFeatExtFull(TessBaseAPI* api, vector<string> tess_api_params,
      const string& groundtruth_path, const string& training_set_path,
      const string& ext, bool makenew);

  virtual void initFeatExtSinglePage();

  virtual std::vector<double> extractFeatures(tesseract::BLOBINFO* blob);

  inline void setImage(PIX* im) {
    curimg = im;
  }

  inline void setApi(TessBaseAPI* api_) {
    api = api_;
  }

  inline void setGrid(BlobInfoGrid* grid_) {
    grid = grid_;
  }

  inline void setDBGDir(const string& dbgdir_) {
    cout << "setting dbgdir to " << dbgdir_ << endl;
    dbgdir = dbgdir_;
  }

  virtual inline string getFeatExtName() {
    return (string)"F_Ext1";
  }

  virtual int numFeatures();

  void dbgAfterExtraction();

  void reset();

 protected :
  typedef GenericVector<NGramFrequency*> RankedNGramVec; // holds counts for just uni/bi/tri gram
  typedef GenericVector<RankedNGramVec> RankedNGramVecs; // holds counts for uni, bi and tri grams
  // cover feature
  int countCoveredBlobs(BLOBINFO* blob, Direction dir);
  bool isNeighborCovered(BLOBINFO* neighbor, BLOBINFO* blob, Direction dir,
      bool indbg, bool& dbgdontcare);

  // nested feature
  int countNestedBlobs(BLOBINFO* blob);

  // sub/super script feature
  void setBlobSubSuperScript(BLOBINFO* blob, SubSuperScript subsuper);

  // baseline feature
  double findBaselineDist(BLOBINFO* blob);

  // stacked blob feature
  // count stacked blobs either above or below depending on the direction
  int countStacked(BLOBINFO* blob, Direction dir);
  // returns true if the neighbor blob is at a distance <= half the curblob's
  // height/width if dir is vertical/horizontal respectively. if dimblob is
  // non-null then uses the same distance as with curblob but instead uses
  // dimblob's dimensions for the thresholding.
  bool isAdjacent(BLOBINFO* neighbor, BLOBINFO* curblob, Direction dir,
      BLOBINFO* dimblob=NULL);

  // average ocr confidence
  double avg_confidence;

  // N-Grams Feature
  //     Returns the n-gram feature for the given sentence (either the
  //     uni/bi/tri-gram depending on the gram argument (1-3). also
  //     normalizes the feature to [0,1] using the expNormalize method.
  //     assumes that the sentence has a RankedNGram vector already processed
  //     which holds the n-grams in the sentence and their counts. if it's
  //     NULL then throws an exception.
  double getNGFeature(Sentence* sentence, int gram);
  RankedNGramVecs math_ngrams; // the Math N-Gram Profile
  //     Looks for a match of the given ngram (either uni, bi, or tri based on the
  //     gram argument) and, if a match is found, returns the ngram's count multiplied
  //     by the matching profile count. If no match was found then returns 0.
  double findNGProfileMatch(NGramFrequency* ngramfreq, RankedNGramVec profile, int gram);
  //     scales the feature to [0,5] in the following manner:
  //     1. if the highest ranked ngram on the profile has a count <= 5
  //        then the count is kept the same but capped at 5.
  //     2. if the highest ranked ngram on the profile has a count  > 5
  //        then the count is divided by 10 and capped at 5.
  double scaleNGramFeature(double feature, RankedNGramVec profile);

  // normalization 1-e^-x
  double expNormalize(double feature);

  void dbgSubSuper(BLOBINFO* blob, BLOBINFO* neighbor, SubSuperScript subsuper);
  void dbgDisplayNGrams(const RankedNGramVecs& ngrams);
  void dbgDisplayBlob(BLOBINFO* blob);
  string dbgdir;

  GenericVector<string> mathwords;
  GenericVector<char*> stopwords;

  BlobInfoGrid* grid;
  double avg_blob_height;
  double avg_whr;
  bool bad_page;
  PIX* curimg;
  TessBaseAPI* api;
  string training_set_path;

  PIX* dbgim;
  virtual void resetDbgIm() {
    pixDestroy(&dbgim);
    dbgim = pixCopy(NULL, curimg);
    dbgim = pixConvertTo32(dbgim);
  }
};

#endif
