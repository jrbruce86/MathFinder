/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext1.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 21, 2013 9:16:32 PM
 * ------------------------------------------------------------------------
 * Description: Implements the feature extraction interface. TODO: More details
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


#include <F_Ext1.h>

#define NUM_FEATURES 21

#define DBG_F_EXT1 // uncomment to enable debugging
//#define DBG_COVER_FEATURE
//#define DBG_NESTED_FEATURE
//#define DBG_SHOW_NESTED
//#define DBG_SUB_SUPER
//#define DBG_SHOW_SUB_SUPER
//#define DBG_FEAT1
//#define DBG_FEAT2
//#define DBG_FEAT3
//#define DBG_AVG
//#define DBG_DRAW_BASELINES
//#define DBG_SHOW_STACKED_FEATURE
//#define DBG_STACKED_FEATURE_ALOT
//#define DBG_SHOW_MATHWORDS
//#define DBG_SHOW_ITALIC
//#define DBG_CERTAINTY
//#define DBG_SHOW_NGRAMS
//#define DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE

typedef GenericVector<NGramFrequency*> RankedNGramVec;
typedef GenericVector<RankedNGramVec> RankedNGramVecs;

F_Ext1::F_Ext1() : curimg(NULL), grid(NULL), avg_blob_height(-1),
    bad_page(false), avg_whr(-1), avg_confidence(0) {}

F_Ext1::~F_Ext1() {
  avg_baseline_dist.clear();
  NGramRanker ng;
  ng.destroyNGramVecs(math_ngrams);
}

void F_Ext1::reset() {
  avg_baseline_dist.clear();
}

void F_Ext1::initFeatExtFull(TessBaseAPI& api, const string& groundtruth_path_,
    const string& training_set_path_, const string& ext, bool makenew) {
  NGramProfileGenerator ng_gen(training_set_path_);
  math_ngrams = ng_gen.generateMathNGrams(api, groundtruth_path_,
      training_set_path_, ext, makenew);
  training_set_path = training_set_path_;
  string mathwordsfile = training_set_path_ + (string)"../../mathwords";
  ifstream mathwordstream(mathwordsfile.c_str());
  if(!mathwordstream.is_open()) {
    cout << "ERROR: Could not open mathwords file at " << mathwordsfile << endl;
    exit(EXIT_FAILURE);
  }
  int maxlen = 55;
  char mathword[maxlen];
  while(!mathwordstream.eof()) {
    mathwordstream.getline(mathword, maxlen);
    string word = (string)mathword;
    if(word.empty())
      continue;
    mathwords.push_back(word);
  }
}

// All features or calculations which require evaluating the entire
// page rather than just one blob at a time are found here
void F_Ext1::initFeatExtSinglePage() {
  M_Utils m;
  // search initialization
  BlobInfoGridSearch bigs(grid);
  BLOBINFO* blob = NULL;

#ifdef DBG_F_EXT1
  grid->setFeatExtFormat(training_set_path, "F_Ext1", (int)NUM_FEATURES);
#endif

  // Determine the average height and width/height ratio of normal text on the page
  bigs.StartFullSearch();
  double avgheight = 0;
  double avgwhr = 0;
  double count = 0;
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->validword) {
      avgheight += (double)blob->height();
      avgwhr += ((double)blob->width() / (double)blob->height());
      ++count;
    }
  }
  if(count > 0) {
    avg_blob_height = avgheight / count;
    avg_whr = avgwhr / count;
  }
  else
    bad_page = true;
#ifdef DBG_AVG
  if(bad_page)
    cout << "The page has no valid words!!\n";
  else {
    cout << "Average normal text height: " << avg_blob_height << endl;
    cout << "Average width to height ratio for normal text: " << avg_whr << endl;
  }
#endif

  // Determine the sub/super-script feature for each blob in the grid
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    setBlobSubSuperScript(blob, SUB);
    setBlobSubSuperScript(blob, SUPER);
  }
#ifdef DBG_SHOW_SUB_SUPER
  PIX* dbgss_im = pixCopy(NULL, curimg);
  dbgss_im = pixConvertTo32(dbgss_im);
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->has_sup || blob->has_sub)
      m.drawHlBlobInfoRegion(blob, dbgss_im, RED);
    else if(blob->is_sup)
      m.drawHlBlobInfoRegion(blob, dbgss_im, GREEN);
    else if(blob->is_sub)
      m.drawHlBlobInfoRegion(blob, dbgss_im, BLUE);
  }
  pixDisplay(dbgss_im, 100, 100);
  pixWrite((dbgdir + (string)"subsuper.png").c_str(), dbgss_im, IFF_PNG);
  pixDestroy(&dbgss_im);
  m.waitForInput();
#endif

  // --- Baseline distance feature ---
  // for each row, compute the average vertical distance from the baseline for all
  // blobs belonging to normal words
  GenericVector<ROW*> rows = grid->getRows();
  for(int i = 0; i < rows.length(); i++) {
    double avg_baseline_dist_ = 0;
    double count = 0;
    ROW* row = rows[i];
    BlobInfoGridSearch bigs(grid);
    bigs.StartFullSearch();
    BLOBINFO* blob;
    while((blob = bigs.NextFullSearch()) != NULL) {
      if(blob->row == row) {
        if(blob->validword) {
          double dist = findBaselineDist(blob);
          avg_baseline_dist_ += dist;
          ++count;
        }
      }
    }
    if(avg_baseline_dist_ == 0 || count == 0)
      avg_baseline_dist_ = 0;
    else
      avg_baseline_dist_ /= count;
#ifdef DBG_DRAW_BASELINE
    cout << "row " << i << " average baseline dist: " << avg_baseline_dist_ << endl;
#endif
    avg_baseline_dist.push_back(avg_baseline_dist_);
    grid->appendAvgBaseline(avg_baseline_dist_);
  }
  assert(avg_baseline_dist.length() == grid->getRows().length());
#ifdef DBG_DRAW_BASELINES
  PIX* dbgim = pixCopy(NULL, curimg);
  dbgim = pixConvertTo32(dbgim);
  for(int i = 0; i < rows.length(); i++) {
    ROW* row = rows[i];
    // find left-most and rightmost blobs on that row
    BlobInfoGridSearch bigs(grid);
    bigs.StartFullSearch();
    BLOBINFO* b;
    int left = INT_MAX;
    int right = INT_MIN;
    while((b = bigs.NextFullSearch()) != NULL) {
      if(b->row == row) {
        if(b->left() < left)
          left = b->left();
        if(b->right() > right)
          right = b->right();
      }
    }
    if(left == INT_MAX || right == INT_MIN) {
      cout << "WARNING::ROW EMPTY!!\n";
      continue;
    }
    Lept_Utils lu;
    for(int j = left; j < right; j++) {
      inT32 y = curimg->h - (inT32)row->base_line((float)j);
      lu.drawAtXY(dbgim, j, y, GREEN);
    }
  }
  pixDisplay(dbgim, 100, 100);
  pixWrite((dbgdir + "baselines.png").c_str(), dbgim, IFF_PNG);
  pixDestroy(&dbgim);
#endif

  // count of stacked characters at character position (coscacp)
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->cosbabp_processed)
      continue;
    int stacked_count = 0;
    stacked_count += countStacked(blob, UP);
    stacked_count += countStacked(blob, DOWN);
    blob->cosbabp_processed = true;
    blob->cosbabp = stacked_count;
  }
#ifdef DBG_SHOW_STACKED_FEATURE
  bigs.StartFullSearch();
  PIX* dbgim2 = pixCopy(NULL, curimg);
  dbgim2 = pixConvertTo32(dbgim2);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->cosbabp == 0)
      continue;
    if(blob->cosbabp < 0) {
      cout << "ERROR: stacked character count < 0 >:-[\n";
      exit(EXIT_FAILURE);
    }
    SimpleColor color;
    if(blob->cosbabp == 1)
      color = RED;
    if(blob->cosbabp == 2)
      color = GREEN;
    if(blob->cosbabp > 2)
      color = BLUE;
    m.drawHlBlobInfoRegion(blob, dbgim2, color);
  }
  pixDisplay(dbgim2, 100, 100);
  pixWrite((dbgdir + "stacked_blobs.png").c_str(), dbgim2, IFF_PNG);
  pixDestroy(&dbgim2);
  m.waitForInput();
#endif

  // determine whether or not each blob belongs to a "math word"
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->wordstr == NULL)
      continue;
    string blobword = (string)blob->wordstr;
    for(int i = 0; i < mathwords.length(); i++) {
      if(blobword == mathwords[i]) {
        blob->ismathword = true;
        break;
      }
    }
  }
#ifdef DBG_SHOW_MATHWORDS
  bigs.StartFullSearch();
  PIX* mathwordim = pixCopy(NULL, curimg);
  mathwordim = pixConvertTo32(mathwordim);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->ismathword) {
      m.drawHlBlobInfoRegion(blob, mathwordim, RED);
    }
  }
  pixDisplay(mathwordim, 100, 100);
  pixWrite((dbgdir + (string)"mathwords.png").c_str(), mathwordim, IFF_PNG);
  pixDestroy(&mathwordim);
  m.waitForInput();
#endif
#ifdef DBG_SHOW_ITALIC
  bigs.StartFullSearch();
  PIX* boldital_img = pixCopy(NULL, curimg);
  boldital_img = pixConvertTo32(boldital_img);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->is_italic)
      m.drawHlBlobInfoRegion(blob, boldital_img, RED);
  }
  pixDisplay(boldital_img, 100, 100);
  pixWrite((dbgdir + (string)"bolditalics.png").c_str(), boldital_img, IFF_PNG);
  pixDestroy(&boldital_img);
  m.waitForInput();
#endif

  // determine the average ocr confidence for valid words on the page,
  // if there are no valid words the bad_page flag is set to true
  bigs.StartFullSearch();
  double validblobcount = 0;
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->validword) {
      avg_confidence += blob->certainty;
      ++validblobcount;
    }
  }
  avg_confidence /= validblobcount;
#ifdef DBG_CERTAINTY
  cout << "Average valid word certainty: " << avg_confidence << endl;
#endif

  // Determine the N-Gram features for each sentence
  // -- first get the ranked ngram vectors for each sentence
  GenericVector<Sentence*> page_sentences = grid->getSentences();
  NGramRanker ng(training_set_path);
  ng.setTessAPI(&api);
  for(int i = 0; i < page_sentences.length(); i++) {
    Sentence* cursentence = page_sentences[i];
    RankedNGramVecs* sentence_ngrams = new RankedNGramVecs;
    *sentence_ngrams = ng.generateSentenceNGrams(cursentence);
    cursentence->ngrams = sentence_ngrams; // store the n-grams in the sentence struct
#ifdef DBG_SHOW_NGRAMS
    cout << "Displaying the N-Grams found for the following sentence:\n"
         << cursentence->sentence_txt << endl;
    dbgDisplayNGrams(*(cursentence->ngrams));
    M_Utils m;
    m.waitForInput();
#endif
  }
  // -- now use the ranked ngram vectors and compare them to the n-gram profile
  //    to get the actual feature values.
  for(int i = 0; i < page_sentences.length(); i++) {
    Sentence* cursentence = page_sentences[i];
    GenericVector<double>* ng_features = new GenericVector<double>;
    for(int j = 0; j < 3; j++)
      ng_features->push_back(getNGFeature(cursentence, j+1));
    cursentence->ngram_features = ng_features;
  }

}

/* Carries out feature extraction on the given blob returning the
 resulting feature vector, with each feature normalized to [0,1] */
vector<double> F_Ext1::extractFeatures(tesseract::BLOBINFO* blob) {
  // The following 20 features are extracted:
  // I. Blob Spatial Features (12 features total):
  //  1. Number of Horizontally or Vertically Aligned Characters
  //     - # rightward horizontally adjacent blobs covered (rhabc) -f1
  //     - # upward vertically adjacent blobs covered (uvabc) -f2
  //     - # downward vertically adjacent blobs covered (dvabc) -f3
  //  2. Number of Completely Nested Characters (cn) -f4
  //  3. Sub-scripts or Super-scripts (4 binary features)
  //     - has a superscript (has_sup) -f5
  //     - has a subscript (has_sub) -f6
  //     - is superscript (is_sup) -f7
  //     - is subscript (is_sub) -f8
  //  4. Height (h) -f9
  //     - blob height / average normal text blob height
  //     - if no normal text on the page then just use the height
  //       and set the bad_page feature to 1
  //  5. Character Width/Height Ratio (whr) -f10
  //     - blob whr / average normal text whr
  //     - if no normal text on the page then just use whr and set
  //       the bad_page feature to 1
  //  6. Vertical Distance Above Row Baseline (vdarb) -f11
  //     - 0 if the current row has no valid words or blob bottom is below baseline
  //       otherwise this feature is (blob_bottom - baseline)/rowheight.
  //  7. Count of Stacked Blobs at Blob Position (cosbabp) -f12
  //     - 0 if blob belongs to valid word, otherwise counts the number of
  //       "vertically stacked" blobs (including the blob itself) at the given
  //       blob's position. Blob is vertically stacked if it is within a vertical distance
  //       <= half the current blob's height.

  // II. Blob Recognition Features:
  //  1. Recognized Math Symbols or Words (is in math word = imw) -f13
  //     - 1 if the symbol is part of a word which matches a word in the mathwords file
  //       (located in the training directory) otherwise is zero
  //  2. Italicized Text (1 binary feature)
  //     - is italic (is_italic) -f14
  //  3. OCR Confidence Rating (ocr_conf) -f15
  //     - blob confidence divided by average normal text confidence on page.
  //       if no normal text on page then just the negative of blob confidence
  //  4. N-Grams (3 features) - see thesis for description
  //     - unigram feature (unigram) -f16
  //     - bigram feature (bigram) -f17
  //     - trigram feature (trigram) -f18
  //  5. Blob belongs to normal text (in_valid_word) -f19
  //  6. Blob belongs to row that has normal text (in_valid_row) -f20
  //  7. Page Doesn't Have Normal Text (bad_page) -f21
  //     - 1 if the page doesn't have normal text based on
  //       Tesseract's dictionary, otherwise is 0
  const int num_features = NUM_FEATURES;

  // feature vector
  vector<double> fv;

  /******** Features I.1 ********/
  double rhabc = (double)0, uvabc = (double)0, dvabc = (double)0;
  rhabc = countCoveredBlobs(blob, RIGHT);
  uvabc = countCoveredBlobs(blob, UP);
  dvabc = countCoveredBlobs(blob, DOWN);

#ifdef DBG_FEAT1
  if(rhabc > 1)
    cout << "Displayed blob has rhabc = " << rhabc << endl;
  if(uvabc > 1)
    cout << "Displayed blob has uvabc = " << uvabc << endl;
  if(dvabc > 1)
    cout << "Displayed blob has dvabc = " << dvabc << endl;
  if(rhabc > 1 || uvabc > 1 || dvabc > 1)
    dbgDisplayBlob(blob);
#endif

  rhabc = expNormalize(rhabc);
  uvabc = expNormalize(uvabc);
  dvabc = expNormalize(dvabc);
  fv.push_back(rhabc);
  fv.push_back(uvabc);
  fv.push_back(dvabc);

  /******** Features I.2 ********/
  double cn = (double)0;
  cn = countNestedBlobs(blob);

#ifdef DBG_FEAT2
  if(cn > 0) {
    cout << "Displayed blob has " << cn << " nested blobs\n";
    dbgDisplayBlob(blob);
  }
#endif

  cn = expNormalize(cn);
  fv.push_back(cn);

  /******** Features I.3 ********/
  double has_sup = (double)0, has_sub = (double)0,
      is_sup = (double)0, is_sub = (double)0;
  if(blob->has_sup)
    has_sup = 1;
  if(blob->has_sub)
    has_sub = 1;
  if(blob->is_sup)
    is_sup = 1;
  if(blob->is_sub)
    is_sub = 1;
#ifdef DBG_FEAT3
  if(has_sup || has_sub || is_sup || is_sub) {
    cout << "The displayed blob has/is a sub/superscript!\n";
    cout << "has_sup: " << has_sup << ", has_sub: " << has_sub
         << ", is_sup: " << is_sup << ", is_sub: "  << is_sub << endl;
    dbgDisplayBlob(blob);
  }
#endif
  fv.push_back(has_sup);
  fv.push_back(has_sub);
  fv.push_back(is_sup);
  fv.push_back(is_sub);

  /******** Features I.4 ********/
  double h = (double)0;
  if(!bad_page)
    h = (double)blob->height() / avg_blob_height;
  else
    h = (double)blob->height();
  h = expNormalize(h);
  fv.push_back(h);

  /******** Features I.5 ********/
  double whr = (double)blob->width() / (double)blob->height();
  if(!bad_page)
    whr = whr / avg_whr;
  whr = expNormalize(whr);
  fv.push_back(whr);

  /******** Features I.6 ********/
  double vdarb = (double)0;
  if(blob->row_has_valid) {
    double rowheight = -1;
    double avg_baseline_dist_ = (double)-1;
    // find the average baseline for the blob's row
    GenericVector<ROW*> rows = grid->getRows();
    assert(rows.length() == avg_baseline_dist.length());
    for(int i = 0; i < rows.length(); i++) {
      if(rows[i] == blob->row) {
        avg_baseline_dist_ = avg_baseline_dist[i];
        rowheight = rows[i]->bounding_box().height();
      }
    }
    if(avg_baseline_dist_ == (double)-1) {
      cout << "ERROR: The blob row vector has been corrupted >:-[\n";
      exit(EXIT_FAILURE);
    }
    double baseline_dist = findBaselineDist(blob);
    blob->dist_above_baseline = baseline_dist;
    assert(avg_baseline_dist_ >= 0);
    vdarb = baseline_dist - avg_baseline_dist_;
    if(vdarb < 0)
      vdarb = (double)0;
    else {
      assert(rowheight > 0);
      vdarb /= rowheight;
      vdarb = expNormalize(vdarb);
    }
  }
  fv.push_back(vdarb);

  /******** Features I.7 ********/
  double coscacp = (double)blob->cosbabp;
  coscacp = expNormalize(coscacp);
  fv.push_back(coscacp);

  /******** Features II.1 ********/
  double imw = (double)0;
  if(blob->ismathword)
    imw = (double)1;
  fv.push_back(imw);

  /******** Features II.2 ********/
  double is_italic = (double)0;
  if(blob->is_italic)
    is_italic = (double)1;
  fv.push_back(is_italic);

  /******** Features II.3 ********/
  double ocr_conf = (double)blob->certainty;
  if(ocr_conf > avg_confidence)
    ocr_conf = avg_confidence; // don't punish for having more confidence
  ocr_conf /= avg_confidence;
  ocr_conf = expNormalize(ocr_conf);
  fv.push_back(ocr_conf);

  /******** Features II.4 ********/
  double unigram = (double)0, bigram = (double)0, trigram = (double)0;
  Sentence* blob_sentence = grid->getBlobSentence(blob);
  if(blob_sentence != NULL) {
    GenericVector<double> sentence_ngram_features = *(blob_sentence->ngram_features);
    unigram = sentence_ngram_features[0];
    bigram = sentence_ngram_features[1];
    trigram = sentence_ngram_features[2];
  }
  fv.push_back(unigram);
  fv.push_back(bigram);
  fv.push_back(trigram);

  /******** Features II.5 ********/
  double in_valid_row = (double)0;
  if(blob->row_has_valid)
    in_valid_row = (double)1;
  fv.push_back(in_valid_row);

  /******** Features II.6 ********/
  double in_valid_word = (double)0;
  if(blob->validword)
    in_valid_word = (double)1;
  fv.push_back(in_valid_word);

  /******** Features II.7 ********/
  double bad_page_ = (double)0;
  if(bad_page)
    bad_page_ = (double)1;
  fv.push_back(bad_page_);

  /******** Done extracting features! ********/
  // return all the features, make sure there's the right amount
  assert(num_features == fv.size());
  blob->features_extracted = true;
  return fv;
}

int F_Ext1::countCoveredBlobs(BLOBINFO* blob, Direction dir) {
  BlobInfoGridSearch bigs(grid);
  GenericVector<BLOBINFO*> covered_blobs;
  int count = 0;
#ifdef DBG_COVER_FEATURE
  inT16 dbgleft = -1;
  inT16 dbgbottom = -1;
  inT16 dbgright = -1;
  inT16 dbgtop = -1;
  TBOX dbgbox(dbgleft, dbgbottom, dbgright, dbgtop);
  TBOX blobbox = blob->bounding_box();
  bool indbg = false;
  if(blobbox == dbgbox) {
    cout << "found it!\n";
    M_Utils m;
    m.dispBlobInfoRegion(blob, curimg);
    m.dispHlBlobInfoRegion(blob, curimg);
    m.waitForInput();
    indbg = true;
  }
#endif
  // if the blob in question belongs to a valid word then
  // I discard it immediately, assigning the feature value to zero
  if(blob->validword)
    return 0;
  int range;
  if(dir == RIGHT) {
    range = blob->height();
  }
  else if(dir == UP || dir == DOWN) {
    range = blob->width();
  }
  else {
    cout << "ERROR: countCoveredBlobs only "
         << "supports upward, downward, and rightward searches\n";
    exit(EXIT_FAILURE);
  }
#ifdef DBG_COVER_FEATURE
  BLOBINFO* pn = NULL;
#endif
  // do beam searches to look for covered blobs
  // TODO: For increased efficiency do fewer searches (come up with reasonable empirical
  //       way of dividing the current blob so enough searches are done but it isn't excessive
  for(int i = 0; i < range; i++) {
    BLOBINFO* n;
    if(dir == RIGHT) {
      bigs.StartSideSearch(blob->right() + 1, blob->bottom()+i, blob->bottom()+i+1);
      n = bigs.NextSideSearch(false); // this starts with the current blob
    }
    else {
      bigs.StartVerticalSearch(blob->left()+i, blob->left()+i+1,
          (dir == UP) ? blob->top() : blob->bottom());
      n = bigs.NextVerticalSearch((dir == UP) ? false : true);
    }
    if(n == NULL)
      continue;
    bool nothing = false;
    while(n->bounding_box() == blob->bounding_box()
        || ((dir == RIGHT) ? (n->right() <= blob->left())
            : ((dir == UP) ? n->top() <= blob->bottom()
                : n->bottom() >= blob->top()))) {
      if(dir == RIGHT)
        n = bigs.NextSideSearch(false);
      else
        n = bigs.NextVerticalSearch(dir == UP ? false : true);
      if(n == NULL) {
        nothing = true;
        break;
      }
    }
    if(nothing)
      continue;
#ifdef DBG_COVER_FEATURE
    if(indbg && n != pn) {
      cout << "displaying element found to the right\n";
      M_Utils m;
      m.dispHlBlobInfoRegion(n, curimg);
      m.dispBlobInfoRegion(n, curimg);
      cout << "at i: " << i << endl;
      cout << "point: " << blob->bottom() + i << endl;
      m.waitForInput();
    }
    pn = n;
#endif
    // if the neighbor is covered and isn't already on the list
    // then add it to the list (in ascending order)
    if(!covered_blobs.bool_binary_search(n) && n != blob) {
      if(isNeighborCovered(n, blob, dir)) {
        covered_blobs.push_back(n);
        covered_blobs.sort();
        count++;
      }
    }
  }

#ifdef DBG_COVER_FEATURE
  if(count > 1) {
    M_Utils m;
    cout << "the highlighted blob is the one being evaluated and has " << count
         << " covered blobs " << "in the " << ((dir == RIGHT) ? " rightward "
             : ((dir == UP) ? " upward " : " downward ")) << "direction\n";
    m.dispHlBlobInfoRegion(blob, curimg);
    m.dispBlobInfoRegion(blob, curimg);
    m.waitForInput();
    for(int j = 0; j < covered_blobs.length(); j++) {
      cout << "the highlighted blob is covered by the blob previously shown\n";
      m.dispHlBlobInfoRegion(covered_blobs[j], curimg);
      m.dispBlobInfoRegion(covered_blobs[j], curimg);
      m.waitForInput();
    }
  }
#endif

  covered_blobs.clear();
  return count;
}

bool F_Ext1::isNeighborCovered(BLOBINFO* neighbor, BLOBINFO* blob, Direction dir) {
  inT16 neighbor_dist;
  inT16 dist_threshold;
  inT16 blob_upper;
  inT16 blob_lower;
  inT16 neighbor_center;
  if(dir == RIGHT) {
    blob_lower = blob->bottom();
    blob_upper = blob->top();
    dist_threshold = blob->height() / 2;
    neighbor_center = neighbor->centery();
    neighbor_dist = neighbor->left() - blob->right();
  }
  else if(dir == UP || dir == DOWN) {
    blob_lower = blob->left();
    blob_upper = blob->right();
    dist_threshold = blob->width() / 2;
    neighbor_center = neighbor->centerx();
    if(dir == UP)
      neighbor_dist = neighbor->bottom() - blob->top();
    else
      neighbor_dist = blob->bottom() - neighbor->top();
  }
  else {
    cout << "ERROR: isNeighborCovered only supports RIGHT, UP, and DOWN directions\n";
    exit(EXIT_FAILURE);
  }
  // it the neighbor covered?
  if(neighbor_center >= blob_lower && neighbor_center <= blob_upper) {
    // is the neighbor adjacent?
    if(neighbor_dist <= dist_threshold)
      return true;
  }
  return false;
}

int F_Ext1::countNestedBlobs(BLOBINFO* blob) {
  int nested = 0;
  if(blob->validword)
    return 0;
  BlobInfoGridSearch bigs(grid);
  bigs.StartRectSearch(blob->bounding_box());
  BLOBINFO* nestblob;
  while((nestblob = bigs.NextRectSearch()) != NULL) {
    if(nestblob == blob ||
        (nestblob->bounding_box() == blob->bounding_box()))
      continue;
    else {
      // make sure the nestblob is entirely contained within
      // the blob
      TBOX nestbox = nestblob->bounding_box();
      TBOX blobbox = blob->bounding_box();
      if(!blobbox.contains(nestbox))
        continue;
#ifdef DBG_NESTED_FEATURE
      cout << "showing the blob which has a nested element!\n";
      dbgDisplayBlob(blob);
      cout << "displaying the nested element\n";
      dbgDisplayBlob(nestblob);
#endif
      blob->has_nested = true;
      nested++;
    }
  }
  return nested;
}

// Determines whether or not the blob in question has a super/subscript
// and if it does have a super/subscript then its has_sup/has_sub features
// are set and its sub/superscript blob's is_sup/is_sub feature is set also
void F_Ext1::setBlobSubSuperScript(BLOBINFO* blob, SubSuperScript subsuper) {
  // the blob belongs to a valid word, the subscript or superscript can only
  // reside on the last blob of that word
  if(blob->validword) {
    if(blob->blobindex_inword == -1) {
      cout << "ERROR: blob index in a valid word was not set!\n";
      exit(EXIT_FAILURE);
    }
    if(blob->blobindex_inword != blob->word_lastblob)
      return;
  }

  // if the word this blob belongs to ends in punctuation then don't bother
  // looking for the subscript, the punctuation is all that will be found
  if(subsuper == SUB) {
    char* word = blob->wordstr;
    if(word != NULL) {
      int lastchar = strlen(word) - 1;
      if(word[lastchar] == '.' || word[lastchar] == ',' || word[lastchar] == '?')
        return;
    }
  }
  inT16 h_adj_thresh = blob->width() / 2;
  inT32 area_thresh = blob->bounding_box().area() / 8;
  inT16 blob_center_y = blob->centery();
  inT16 blob_right = blob->right() + 1;
  // do repeated beam searches starting from the vertical center
  // of the current blob, going downward for subscripts and upward for superscripts
  // TODO: Make this more efficient (do fewer beam searches)
  BlobInfoGridSearch beamsearch(grid);
  for(int i = 0; i < blob->height() / 2; i++) {
    int j = (subsuper == SUPER) ? i : -i;
    beamsearch.StartSideSearch(blob_right, blob_center_y + j, blob_center_y + j + 1);
    BLOBINFO* neighbor = beamsearch.NextSideSearch(false);
    if(neighbor == NULL)
      continue;
    bool nothing = false;
    while(neighbor->bounding_box() == blob->bounding_box()
        || neighbor->right() <= blob->right()
        || neighbor->left() <= (blob->right() - h_adj_thresh)
        || neighbor->bottom() > blob->top()
        || neighbor->top() < blob->bottom()) {
      neighbor = beamsearch.NextSideSearch(false);
      if(neighbor == NULL) {
        nothing = true;
        break;
      }
    }
    if(nothing)
      continue;
    if(neighbor->bounding_box().area() < area_thresh)
      continue; // too small
    inT16 h_dist = neighbor->left() - blob->right();
    if(h_dist > h_adj_thresh)
      continue; // too far away
    if(neighbor->validword) {
      if(neighbor->blobindex_inword != 0)
        continue; // if the super/subscript is in a valid word, it has to be at the first letter
    }
    if(subsuper == SUPER) {
      if(neighbor->bottom() > (blob_center_y - ((blob_center_y - blob->bottom())/2))) {
#ifdef DBG_SUB_SUPER
        dbgSubSuper(blob, neighbor, subsuper);
#endif
        blob->has_sup = true;
        neighbor->is_sup = true;
        break;
      }
    }
    else if(subsuper == SUB) {
      if(neighbor->top() < blob_center_y) {
#ifdef DBG_SUB_SUPER
        dbgSubSuper(blob, neighbor, subsuper);
#endif
        blob->has_sub = true;
        neighbor->is_sub = true;
        break;
      }
    }
    else {
      cout << "ERROR: Invalid setBlobSubSuperScript option\n";
      exit(EXIT_FAILURE);
    }
  }
}

double F_Ext1::findBaselineDist(BLOBINFO* blob) {
  // calculate the current blob's distance from the baseline
  double blob_baseline = (double)blob->row->base_line(blob->centerx());
  double blob_bottom = (double)blob->bottom();
  double baseline_dist = blob_bottom - blob_baseline;
  if(baseline_dist < 0) {
    if((double)blob->top() < blob_baseline)
      baseline_dist = -baseline_dist;
    else
      baseline_dist = 0;
  }
  return baseline_dist;
}

int F_Ext1::countStacked(BLOBINFO* blob, Direction dir) {
  int count = 0;
  // if the blob belongs to a valid word then the feature is zero
#ifdef DBG_STACKED_FEATURE_ALOT
  int left=1773, top=1243, right=1810, bottom=1210;
  TBOX box(left, bottom, right, top);
#endif
  if(blob->validword) {
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob which is part of a valid word and thus can't have stacked elements\n";
        dbgDisplayBlob(blob);
      }
#endif
    return 0;
  }
  // go to first element above or below depending on the direction
  BlobInfoGridSearch vsearch(grid);
  vsearch.StartVerticalSearch(blob->left(), blob->right(),
      (dir == UP) ? blob->top() : blob->bottom());
  BLOBINFO* central_blob = blob;
  BLOBINFO* stacked_blob;
  BLOBINFO* prev_stacked_blob = central_blob;
  stacked_blob = vsearch.NextVerticalSearch((dir == UP) ? false : true);
  while(true) {
    while(stacked_blob->bounding_box() == prev_stacked_blob->bounding_box()
        || ((dir == UP) ? (stacked_blob->bottom() < prev_stacked_blob->top())
            : (stacked_blob->top() > prev_stacked_blob->bottom()))
        || stacked_blob->left() >= prev_stacked_blob->right()
        || stacked_blob->right() <= prev_stacked_blob->left()) {
      stacked_blob = vsearch.NextVerticalSearch((dir == UP) ? false : true);
      if(stacked_blob == NULL)
        break;
    }
    if(stacked_blob == NULL) {
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob which has no more adjacent blobs and " << count << " stacked elements\n";
        dbgDisplayBlob(blob);
      }
#endif
      break;
    }
    // found the element above/below the prev_stacked_blob.
    // is it adjacent based on the prev_stacked blob's location and central blob's height?
    if(stacked_blob->validword) { // if its part of a valid word then it's discarded here
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob whose stacked element belongs to a valid word and thus can't be stacked\n";
        dbgDisplayBlob(blob);
        cout << "showing the candidate which can't be stacked\n";
        dbgDisplayBlob(stacked_blob);
      }
#endif
      return count;
    }
    if(isAdjacent(stacked_blob, prev_stacked_blob, dir, central_blob)) {
#ifdef DBG_STACKED_FEATURE_ALOT
      int dbgall = false;
      if(blob->bounding_box() == box || dbgall) {
        cout << "showing the blob being measured to have " << count+1 << " stacked items\n";
        dbgDisplayBlob(blob);
        cout << "showing the stacked blob\n";
        cout << "here is the stacked blob's bottom and top y coords: "
             << stacked_blob->bottom() << ", " << stacked_blob->top() << endl;
        cout << "here is the bottom and top y coords of the blob being measured: "
             << central_blob->bottom() << ", " << central_blob->top() << endl;
        cout << "here is the bottom and top y coords of the current blob above/below the stacked blob: "
             << prev_stacked_blob->bottom() << ", " << prev_stacked_blob->top() << endl;
        dbgDisplayBlob(stacked_blob);
      }
#endif
      ++count;
      prev_stacked_blob = stacked_blob;
      assert(prev_stacked_blob->bounding_box() == stacked_blob->bounding_box());
    }
    else {
#ifdef DBG_STACKED_FEATURE_ALOT
      if(blob->bounding_box() == box) {
        cout << "showing a blob which has no (or no more) stacked features\n";
        dbgDisplayBlob(blob);
        cout << "showing the candidate which was not adjacent\n";
        dbgDisplayBlob(stacked_blob);
      }
#endif
      break;
    }
  }
  return count;
}

bool F_Ext1::isAdjacent(BLOBINFO* neighbor, BLOBINFO* curblob,
    Direction dir, BLOBINFO* dimblob) {
  if(dimblob == NULL)
    dimblob = curblob;
  double cutoff = (double)dimblob->bounding_box().area() / (double)16;
  if((double)(neighbor->bounding_box().area()) < cutoff)
    return false;
  bool vert = (dir == UP || dir == DOWN) ? true : false;
  bool ascending = (vert ? ((dir == UP) ? true : false)
      : ((dir == RIGHT) ? true : false));
  double dist_thresh = (double)(vert ? dimblob->height() : dimblob->width());
  dist_thresh /= 2;
  double distop1, distop2; // dist = distop1 - distop2
  if(vert) {
    distop1 = ascending ? (double)neighbor->bottom() : (double)curblob->bottom();
    distop2 = ascending ? (double)curblob->top() : (double)neighbor->top();
  }
  else {
    distop1 = ascending ? (double)neighbor->left() : (double)curblob->left();
    distop2 = ascending ? (double)curblob->right() : (double)neighbor->right();
  }
  double dist = distop1 - distop2;
  if(dist <= dist_thresh)
    return true;
  return false;
}

double F_Ext1::getNGFeature(Sentence* sentence, int gram) {
  assert(gram > 0 && gram < 4); // only uni, bi, and tri-grams supported
  double ng_feat = 0;
  int gramindex = gram - 1; // zero-based index
  RankedNGramVecs ngramsvec = *sentence->ngrams;
  RankedNGramVec ngrams = ngramsvec[gramindex];
  for(int i = 0; i < ngrams.length(); i++)
    ng_feat += findNGProfileMatch(ngrams[i], math_ngrams[gramindex], gram);
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  cout << "Sentence:\n" << sentence->sentence_txt << endl;
  cout << "The unscaled " << gram << "-gram feature for the above sentence:\n";
  cout << ng_feat << endl;
#endif
  ng_feat = scaleNGramFeature(ng_feat, math_ngrams[gramindex]);
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  cout << "The scaled " << gram << "-gram feature: " << ng_feat << endl;
#endif
  ng_feat = expNormalize(ng_feat);
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  cout << "The normalized " << gram << "-gram feature: " << ng_feat << endl;
  M_Utils m;
  m.waitForInput();
#endif
  return ng_feat;
}

double F_Ext1::findNGProfileMatch(NGramFrequency* ngramfreq,
    RankedNGramVec profile, int gram) {
  NGram* ngram = ngramfreq->ngram;
  double ngram_count = ngramfreq->frequency;
  assert(ngram->length() == gram);
  for(int i = 0; i < profile.length(); i++) {
    NGram* p_ngram = profile[i]->ngram;
    double p_ngram_count = profile[i]->frequency;
    assert(p_ngram->length() == gram);
    if(*p_ngram == *ngram) {
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
      cout << "Matching profile ngram: " << *p_ngram << ", occurs " << p_ngram_count
           << " times on profile and " << ngram_count << " times in sentence\n";
#endif
      return ngram_count * p_ngram_count;
    }
  }
  return (double)0;
}

double F_Ext1::scaleNGramFeature(double ng_feat, RankedNGramVec profile) {
  double upperbound = (double)5;
  double scaled_feat = ng_feat;
  double top_profile_freq = profile[0]->frequency;
  if(top_profile_freq > upperbound)
    scaled_feat /= (double)10;
  if(scaled_feat > upperbound)
    scaled_feat = upperbound;
  return scaled_feat;
}

double F_Ext1::expNormalize(double f) {
  return 1 - exp(-f);
}

void F_Ext1::dbgDisplayBlob(BLOBINFO* blob) {
  M_Utils m;
  m.dispHlBlobInfoRegion(blob, curimg);
  m.dispBlobInfoRegion(blob, curimg);
  m.waitForInput();
}

void F_Ext1::dbgSubSuper(BLOBINFO* blob, BLOBINFO* neighbor, SubSuperScript subsuper) {
  cout << "found a " << ((subsuper == SUPER) ? "super" : "sub")
       << "-script for the displayed blob\n";
  cout << "heres the recognition result for that blob's word: "
       << ((blob->wordstr == NULL) ? "NULL" : blob->wordstr) << endl;
  if(blob->word == NULL)
    cout << "no blobs were recognized in the blob's word!\n";
  dbgDisplayBlob(blob);
  cout << "displayed is the previous blob's "
       << ((subsuper == SUPER) ? "super" : "sub") << "-script\n";
  dbgDisplayBlob(neighbor);
}

void F_Ext1::dbgDisplayNGrams(const RankedNGramVecs& ngrams) {
  for(int i = 0; i < ngrams.length(); i++) {
    cout << i + 1 << "-grams:\n";
    for(int j = 0; j < ngrams[i].length(); j++) {
      cout << *(ngrams[i][j]->ngram) << " : "
           << ngrams[i][j]->frequency << endl;
    }
  }
}

void F_Ext1::dbgAfterExtraction() {
  BlobInfoGridSearch bigs(grid);
  bigs.StartFullSearch();
  BLOBINFO* blob;
  M_Utils m;
#ifdef DBG_SHOW_NESTED
  PIX* dbgnested_im = pixCopy(NULL, curimg);
  dbgnested_im = pixConvertTo32(dbgnested_im);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->has_nested)
      m.drawHlBlobInfoRegion(blob, dbgnested_im, RED);
  }
  pixDisplay(dbgnested_im, 100, 100);
  pixWrite((dbgdir + (string)"nested.png").c_str(), dbgnested_im, IFF_PNG);
  pixDestroy(&dbgnested_im);
  m.waitForInput();
#endif
}

int F_Ext1::numFeatures() {
  return NUM_FEATURES;
}
