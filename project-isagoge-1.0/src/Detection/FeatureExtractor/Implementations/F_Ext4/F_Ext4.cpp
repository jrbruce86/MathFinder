/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext4.cpp
 * Written by:  Jake Bruce, Copyright (C) 2014
 * History:     Created Jan 1, 2014 2:57:58 PM
 * ------------------------------------------------------------------------
 * Description: Implements the FeatureExtractor interface. Derived from
 *              F_Ext1, this is a modification which tries a slightly different
 *              feature combination, i.e., without the italics feature
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

#include <F_Ext4.h>

#define NUM_FEATURES 22

/* uncomment any of the following to enable debugging */
//#define DBG_AVG
//#define SHOW_ABNORMAL_ROWS
//#define DBG_COVER_FEATURE
//#define DBG_COVER_FEATURE_ALOT // need DBG_COVER_FEATURE enabled for this to go into effect
//#define DBG_NESTED_FEATURE
//#define DBG_SHOW_NESTED
//#define DBG_SUB_SUPER
//#define DBG_SHOW_SUB_SUPER
//#define DBG_FEAT1
//#define DBG_FEAT2
//#define DBG_FEAT3
//#define DBG_DRAW_BASELINES
//#define DBG_SHOW_STACKED_FEATURE
//#define DBG_STACKED_FEATURE_ALOT
//#define DBG_SHOW_MATHWORDS
//#define DBG_SHOW_ITALIC
//#define DBG_CERTAINTY
//#define DBG_SHOW_NGRAMS
//#define DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
//#define SHOW_STOP_WORDS
//#define SHOW_VALID_WORDS

//#define DBG_DISPLAY // turn this on to display dbg images as they are saved

void F_Ext4::initFeatExtSinglePage() {
  static int imdbgnum = 1;
  string num = Basic_Utils::intToString(imdbgnum);
  //dbgim = pixCopy(NULL, curimg);
 // dbgim = pixConvertTo32(dbgim);
  // search initialization
  BlobInfoGridSearch bigs(grid);
  BLOBINFO* blob = NULL;

  grid->setFeatExtFormat(training_set_path, "F_Ext4", (int)NUM_FEATURES);

  // Determine the average height and width/height ratio of normal text on the page
  bigs.StartFullSearch();
  double avgheight = 0;
  double avgwhr = 0;
  double count = 0;
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->validword && blob->onRowNormal()) {
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

#ifdef SHOW_ABNORMAL_ROWS
  bigs.StartFullSearch();
  blob = NULL;
  PIX* pixdbg_r = pixCopy(NULL, curimg);
  pixdbg_r = pixConvertTo32(pixdbg_r);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->rowinfo() == NULL)
      continue;
    if(blob->rowinfo()->is_normal)
      M_Utils::drawHlBlobInfoRegion(blob, pixdbg_r, LayoutEval::RED);
    else
      M_Utils::drawHlBlobInfoRegion(blob, pixdbg_r, LayoutEval::BLUE);
  }
  pixWrite((dbgdir + (string)"Rows_Abnormal"
      + Basic_Utils::intToString(imdbgnum) + (string)".png").c_str(),
      pixdbg_r, IFF_PNG);
#ifdef DBG_DISPLAY
  cout << "displaying the abnormal rows for image " << imdbgnum << " as blue regions\n";
  pixDisplay(pixdbg_r, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&pixdbg_r);
#endif

  // Determine the adjacent covered neighbors in the rightward, downward,
  // and upward directions for each blob
  bigs.StartFullSearch();
  blob = NULL;
  while((blob = bigs.NextFullSearch()) != NULL) {
    blob->rhabc = countCoveredBlobs(blob, RIGHT);
    blob->uvabc = countCoveredBlobs(blob, UP);
    blob->dvabc = countCoveredBlobs(blob, DOWN);
  }

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
      M_Utils::drawHlBlobInfoRegion(blob, dbgss_im, LayoutEval::RED);
    else if(blob->is_sup)
      M_Utils::drawHlBlobInfoRegion(blob, dbgss_im, LayoutEval::GREEN);
    else if(blob->is_sub)
      M_Utils::drawHlBlobInfoRegion(blob, dbgss_im, LayoutEval::BLUE);
  }
  pixWrite((dbgdir + (string)"subsuper" + num + (string)".png").c_str(), dbgss_im, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(dbgss_im, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgss_im);

#endif
  // --- Baseline distance feature ---
  // for each row, compute the average vertical distance from the baseline for all
  // blobs belonging to normal words
  GenericVector<ROW_INFO*> rows = grid->getRows();
  for(int i = 0; i < rows.length(); i++) {
    double avg_baseline_dist_ = 0;
    double count = 0;
    ROW_INFO* row = rows[i];
    GenericVector<WORD_INFO*> words = row->wordinfovec;
    for(int j = 0; j < words.length(); ++j) {
      GenericVector<BLOBINFO*> blobs = words[j]->blobs;
      for(int k = 0; k < blobs.length(); ++k) {
        BLOBINFO* curblob = blobs[k];
        assert(curblob->row_index == i);
        assert(curblob->row() != NULL);
        assert(curblob->row()->bounding_box() == row->row()->bounding_box());
        if(curblob->validword) {
          double dist = findBaselineDist(curblob);
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
    rows[i]->avg_baselinedist = avg_baseline_dist_;
  }
#ifdef DBG_DRAW_BASELINES
  PIX* dbgim = pixCopy(NULL, curimg);
  dbgim = pixConvertTo32(dbgim);
  for(int i = 0; i < rows.length(); i++) {
    ROW_INFO* row = rows[i];
    // find left-most and rightmost blobs on that row
    BlobInfoGridSearch bigs(grid);
    bigs.StartFullSearch();
    BLOBINFO* b;
    int left = INT_MAX;
    int right = INT_MIN;
    while((b = bigs.NextFullSearch()) != NULL) {
      if(b->wordinfo == NULL)
        continue;
      if(b->row()->bounding_box() == row->row()->bounding_box()) {
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
    for(int j = left; j < right; j++) {
      inT32 y = curimg->h - (inT32)row->row()->base_line((float)j);
      Lept_Utils::drawAtXY(dbgim, j, y, LayoutEval::GREEN);
    }
  }
  pixWrite((dbgdir + "baselines" + num + ".png").c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  cout << "Displaying the baselines in green.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
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
      assert(false);
    }
    LayoutEval::Color color;
    if(blob->cosbabp == 1)
      color = LayoutEval::RED;
    if(blob->cosbabp == 2)
      color = LayoutEval::GREEN;
    if(blob->cosbabp > 2)
      color = LayoutEval::BLUE;
    M_Utils::drawHlBlobInfoRegion(blob, dbgim2, color);
  }
  pixWrite((dbgdir + "stacked_blobs" + num + ".png").c_str(), dbgim2, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(dbgim2, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim2);

#endif

  // determine whether or not each blob belongs to a "math word"
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->wordstr() == NULL)
      continue;
    string blobword = (string)blob->wordstr();
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
      M_Utils::drawHlBlobInfoRegion(blob, mathwordim, LayoutEval::RED);
    }
  }
  pixWrite((dbgdir + (string)"mathwords" + num + ".png").c_str(), mathwordim, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(mathwordim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&mathwordim);

#endif

  // Determine the ratio of non-italicized blobs to total blobs on the page
  // A low proportion of non-italics would indicate that the italics feature
  // is not reliable.
  int total_blobs = 0;
  int non_ital_blobs = 0;
  bigs.StartFullSearch();
  blob = NULL;
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(!blob->is_italic)
      ++non_ital_blobs;
    ++total_blobs;
  }
  assert(grid->non_ital_ratio == -1);
  grid->non_ital_ratio = (double)non_ital_blobs / (double)total_blobs;

#ifdef DBG_SHOW_ITALIC
  bigs.StartFullSearch();
  PIX* boldital_img = pixCopy(NULL, curimg);
  boldital_img = pixConvertTo32(boldital_img);
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->is_italic)
      M_Utils::drawHlBlobInfoRegion(blob, boldital_img, LayoutEval::RED);
  }
  pixWrite((dbgdir + "bolditalics" + num + ".png").c_str(), boldital_img, IFF_PNG);
#ifdef DBG_DISPLAY
  cout << "Displaying the blobs which were found by Tesseract to be italicized as red. "
       << "All other blobs are in black.\n";
  cout << "The displayed image was saved to "
       << (dbgdir + "bolditalics" + num + ".png").c_str() << endl;
  pixDisplay(boldital_img, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&boldital_img);
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
  NGramRanker ng(training_set_path, stopwords);
  ng.setTessAPI(api);
  for(int i = 0; i < page_sentences.length(); ++i) {
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


  // Find out which blobs belong to stop words
  bigs.StartFullSearch();
  blob = NULL;
#ifdef SHOW_STOP_WORDS
  string stpwrdim_name = dbgdir + (string)"stop_words_"
      + Basic_Utils::intToString(imdbgnum) + (string)".png";
  assert(dbgim == NULL);
  dbgim = pixCopy(NULL, curimg);
  dbgim = pixConvertTo32(dbgim);
#endif
  while((blob = bigs.NextFullSearch()) != NULL) {
    blob->isstopword = ng.isStrStopWord(blob->wordstr());
#ifdef SHOW_STOP_WORDS
    if(blob->isstopword)
      M_Utils::drawHlBlobInfoRegion(blob, dbgim, LayoutEval::RED);
#endif
  }
#ifdef SHOW_STOP_WORDS
  pixWrite(stpwrdim_name.c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  cout << "Displaying the blobs belonging to stop words in red on the page.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim);
  dbgim = NULL;
#endif

#ifdef SHOW_VALID_WORDS
  string validwrdim_name = dbgdir + (string)"valid_words_" +
      Basic_Utils::intToString(imdbgnum) + (string)".png";
  assert(dbgim == NULL);
  dbgim = pixCopy(NULL, curimg);
  dbgim = pixConvertTo32(dbgim);
  blob = NULL;
  bigs.StartFullSearch();
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->validword)
      M_Utils::drawHlBlobInfoRegion(blob, dbgim, LayoutEval::RED);
  }
  pixWrite(validwrdim_name.c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  cout << "Displaying the blobs belonging to valid words in red on the page.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim);
  dbgim = NULL;
#endif
  ++imdbgnum;
}

std::vector<double> F_Ext4::extractFeatures(tesseract::BLOBINFO* blob) {
  // The following 22 features are extracted:
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
  //  8. Blob belongs to a stop word (stop_word) -f22
  //  9. Blob belongs to a valid word (valid_word) -f23 (not used here)
  const int num_features = NUM_FEATURES;

  const double bin_val = (double)1;

  // feature vector
  vector<double> fv;

  /******** Features I.1 ********/
  double rhabc = blob->rhabc;
  double uvabc = blob->uvabc;
  double dvabc = blob->dvabc;

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
  double cn = countNestedBlobs(blob);
  blob->nestedcount = cn;

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
    has_sup = bin_val;
  if(blob->has_sub)
    has_sub = bin_val;
  if(blob->is_sup)
    is_sup = bin_val;
  if(blob->is_sub)
    is_sub = bin_val;
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
    ROW_INFO* rowinfo = blob->rowinfo();
    assert(rowinfo != NULL); // if row_has_valid is true, should be on a row...
    avg_baseline_dist_ = rowinfo->avg_baselinedist;
    assert(avg_baseline_dist_ >= 0);
    double baseline_dist = findBaselineDist(blob);
    blob->dist_above_baseline = baseline_dist;
    vdarb = baseline_dist - avg_baseline_dist_;
    rowheight = rowinfo->row()->bounding_box().height();
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
    imw = bin_val;
  fv.push_back(imw);

  /******** Features II.2 ********/
/*  double is_italic = (double)0;
  assert(grid->non_ital_ratio >= 0);
  if(blob->is_italic) {
    is_italic = (double)1 * grid->non_ital_ratio;
  }
  fv.push_back(is_italic);*/

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
    in_valid_row = bin_val;
  fv.push_back(in_valid_row);

  /******** Features II.6 ********/
  double in_valid_word = (double)0;
  if(blob->validword)
    in_valid_word = bin_val;
  fv.push_back(in_valid_word);

  /******** Features II.7 ********/
  double bad_page_ = (double)0;
  if(bad_page)
    bad_page_ = bin_val;
  fv.push_back(bad_page_);

  /******** Features II.8 ********/
  double stop_word = (double)0;
  if(blob->isstopword)
    stop_word = bin_val;
  fv.push_back(stop_word);

  /******** Features II.9 ********/
  double valid_word = (double)0;
  if(blob->validword)
    valid_word = bin_val;
  fv.push_back(valid_word);

  /******** Done extracting features! ********/
  // return all the features, make sure there's the right amount
  assert(num_features == fv.size());
  blob->features_extracted = true;
  return fv;
}

int F_Ext4::numFeatures() {
  return (int)NUM_FEATURES;
}

