/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   F_Ext2.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Dec 16, 2013 10:52:43 AM
 * ------------------------------------------------------------------------
 * Description: Implements the FeatureExtractor interface. Derived from
 *              F_Ext1, this is a modification which tries a slightly different
 *              feature combination, namely it removes the height and
 *              width to height ratio features to see if false positives are
 *              reduced by this change.
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

#include <F_Ext2.h>

#define NUM_FEATURES 19

void F_Ext2::initFeatExtSinglePage() {
  dbgim = pixCopy(NULL, curimg);
  dbgim = pixConvertTo32(dbgim);
  // search initialization
  BlobInfoGridSearch bigs(grid);
  BLOBINFO* blob = NULL;

  grid->setFeatExtFormat(training_set_path, "F_Ext2", (int)NUM_FEATURES);

  // Determine the average height and width/height ratio of normal text on the page
  bigs.StartFullSearch();
 // double avgheight = 0;
 // double avgwhr = 0;
  double count = 0;
  while((blob = bigs.NextFullSearch()) != NULL) {
    if(blob->validword) {
      //avgheight += (double)blob->height();
      //avgwhr += ((double)blob->width() / (double)blob->height());
      ++count;
    }
  }
  if(count == 0)
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
        if(curblob->row_index != i) {
          cout << "expected row index: " << i << ", actual: " << curblob->row_index << endl;
          cout << "blob coords:\n";
          BOX* b = M_Utils::getBlobInfoBox(curblob, dbgim);
          M_Utils::dispBoxCoords(b);
          M_Utils::dispHlBlobInfoRegion(curblob, dbgim);
          boxDestroy(&b);
        }
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
  ng.setTessAPI(api);
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

std::vector<double> F_Ext2::extractFeatures(tesseract::BLOBINFO* blob) {
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
/*  double h = (double)0;
  if(!bad_page)
    h = (double)blob->height() / avg_blob_height;
  else
    h = (double)blob->height();
  h = expNormalize(h);
  fv.push_back(h);*/

  /******** Features I.5 ********/
 /* double whr = (double)blob->width() / (double)blob->height();
  if(!bad_page)
    whr = whr / avg_whr;
  whr = expNormalize(whr);
  fv.push_back(whr);*/

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

int F_Ext2::numFeatures() {
  return (int)NUM_FEATURES;
}

