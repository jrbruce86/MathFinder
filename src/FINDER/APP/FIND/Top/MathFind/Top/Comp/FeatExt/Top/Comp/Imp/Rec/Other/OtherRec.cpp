/*
 * OtherRecognitionFeatureExtractor.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <OtherRec.h>

#include <OtherRecDesc.h>
#include <Utils.h>
#include <FinderTrainingPaths.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <WordData.h>
#include <RowData.h>
#include <M_Utils.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>

#include <allheaders.h>

#include <baseapi.h>

#include <string>
#include <iostream>
#include <assert.h>
#include <vector>

#define DBG_AVG
#define SHOW_ABNORMAL_ROWS
#define DBG_DISPLAY
#define DBG_DRAW_BASELINE
#define DBG_DRAW_BASELINES
#define DBG_SHOW_MATHWORDS
#define DBG_SHOW_ITALIC
#define DBG_CERTAINTY
#define SHOW_STOP_WORDS
#define SHOW_VALID_WORDS

OtherRecognitionFeatureExtractor
::OtherRecognitionFeatureExtractor(
    OtherRecognitionFeatureExtractorDescription* const description)
: avg_blob_height(0), avg_whr(0), bad_page(false), avg_confidence(0),
  vdarbFlagEnabled(false), heightFlagEnabled(false),
  widthHeightFlagEnabled(false), isOcrMathFlagEnabled(false),
  isItalicFlagEnabled(false), confidenceFlagEnabled(false),
  isOcrValidFlagEnabled(false), isOnValidOcrRowFlagEnabled(false),
  isOnBadPageFlagEnabled(false), isInOcrStopwordFlagEnabled(false),
  blobDataGrid(NULL) {
  this->description = description;
  this->stopwordHelper = description->getCategory()->getStopwordHelper();
  this->dbgdir = "debug/";

}


void OtherRecognitionFeatureExtractor::doTrainerInitialization() {
  // Read in the "math words" from the file
  std::string mathWordsFileName =
      Utils::checkTrailingSlash(FinderTrainingPaths::getTrainingRoot()) + (std::string)"mathwords";
  std::ifstream mathwordstream;
  mathwordstream.open(mathWordsFileName.c_str());
  if(!mathwordstream.is_open()) {
    std::cout << "ERROR: Could not open mathwords file at " << mathWordsFileName << std::endl;
    assert(false);
  }
  std::string mathWord;
  std::cout << "Mathwords read:\n";
  while(getline(mathwordstream, mathWord)) {
    std::string word = mathWord;
    if(word.empty())
      continue;
    mathwords.push_back(word);
    std::cout << word << std::endl;
  }
}

void OtherRecognitionFeatureExtractor::doFinderInitialization() {
  doTrainerInitialization(); // same
}


void OtherRecognitionFeatureExtractor
::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  //static int imdbgnum = 1;
  //std::string num = Utils::intToString(imdbgnum);

  this->blobDataGrid = blobDataGrid;

  BlobDataGridSearch bdgs(blobDataGrid);
  BlobData* blob = NULL;

  // Determine the average height and width/height ratio of normal text on the page
  bdgs.StartFullSearch();
  double avgheight = 0;
  double avgwhr = 0;
  double count = 0;
  TesseractWordData* const blobWord = blob->getParentChar()->getParentWord();
  TesseractRowData* const blobRow = blob->getParentChar()->getParentWord()->getParentRow();
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blobWord->getIsValidTessWord() && blobRow->getIsConsideredNormal()) {
      avgheight += (double)blob->getBoundingBox().height();
      avgwhr += ((double)blob->getBoundingBox().width() / (double)blob->getBoundingBox().height());
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
    std::cout << "The page has no valid words!!\n";
  else {
    std::cout << "Average normal text height: " << avg_blob_height << endl;
    std::cout << "Average width to height ratio for normal text: " << avg_whr << endl;
  }
#endif

#ifdef SHOW_ABNORMAL_ROWS
  bdgs.StartFullSearch();
  blob = NULL;
  Pix* pixdbg_r = pixCopy(NULL, blobDataGrid->getBinaryImage());
  pixdbg_r = pixConvertTo32(pixdbg_r);
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->getParentRow() == NULL)
      continue;
    if(blob->getParentRow()->isConsideredNormal)
      M_Utils::drawHlBlobDataRegion(blob, pixdbg_r, LayoutEval::RED);
    else
      M_Utils::drawHlBlobDataRegion(blob, pixdbg_r, LayoutEval::BLUE);
  }
  pixWrite((std::string("debug/Rows_Abnormal/") + blobDataGrid->getImageName()).c_str(),
      pixdbg_r, IFF_PNG);
#ifdef DBG_DISPLAY
  std::cout << "displaying the abnormal rows for image " << blobDataGrid->getImageName() << " as blue regions\n";
  pixDisplay(pixdbg_r, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&pixdbg_r);
#endif

  // --- Baseline distance feature ---
  // for each row, compute the average vertical distance from the baseline for all
  // tesseract characters belonging to words assumed to be "normal" based on Tesseract OCR
  std::vector<TesseractRowData*> rows = blobDataGrid->getAllTessRows();
  for(int i = 0; i < rows.size(); i++) {
    double avg_baseline_dist_ = 0;
    double count = 0;
    TesseractRowData* row = rows[i];
    GenericVector<TesseractWordData*> words = row->getTesseractWords();
    for(int j = 0; j < words.length(); ++j) {
      std::vector<TesseractCharData*> chars = words[j]->getTesseractChars();
      for(int k = 0; k < chars.size(); ++k) {
        TesseractCharData* curChar = chars[k];
        assert(curChar->getParentWord()->getParentRow() != NULL);
        assert(curChar->getParentWord()->getParentRow()->getBoundingBox() == row->getBoundingBox());
        if(curChar->getParentWord()->getIsValidTessWord()) {
          double dist = findBaselineDist(curChar);
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
std::cout << "row " << i << " average baseline dist: " << avg_baseline_dist_ << endl;
#endif
rows[i]->avg_baselinedist = avg_baseline_dist_;
  }
#ifdef DBG_DRAW_BASELINES
  PIX* dbgim = pixCopy(NULL, blobDataGrid->getBinaryImage());
  dbgim = pixConvertTo32(dbgim);
  for(int i = 0; i < rows.size(); i++) {
    TesseractRowData* row = rows[i];
    // find left-most and rightmost blobs on that row
    BlobDataGridSearch bigs(blobDataGrid);
    bigs.StartFullSearch();
    BlobData* b;
    int left = INT_MAX;
    int right = INT_MIN;
    while((b = bigs.NextFullSearch()) != NULL) {
      if(b->getParentWord() == NULL)
        continue;
      if(b->getParentRow()->getBoundingBox() == row->row()->bounding_box()) {
        if(b->left() < left)
          left = b->left();
        if(b->right() > right)
          right = b->right();
      }
    }
    if(left == INT_MAX || right == INT_MIN) {
      std::cout << "WARNING::ROW EMPTY!!\n";
      continue;
    }
    for(int j = left; j < right; j++) {
      inT32 y = blobDataGrid->getBinaryImage()->h - (inT32)row->row()->base_line((float)j);
      Lept_Utils::drawAtXY(dbgim, j, y, LayoutEval::GREEN);
    }
  }
  pixWrite((std::string("debug/") + std::string("baselines/") + blobDataGrid->getImageName()).c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  std::cout << "Displaying the baselines in green.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim);

#endif



  // determine whether or not each blob belongs to a "math word"
  bdgs.StartFullSearch();
  while((blob = bdgs.NextFullSearch()) != NULL) {
    const char* wordStr = blob->getParentChar()->getParentWord()->wordstr();
    if(wordStr == NULL)
      continue;
    std::string blobword = (std::string)wordStr;
    for(int i = 0; i < mathwords.length(); i++) {
      if(blobword == mathwords[i]) {
        blob->getParentChar()->getParentWord()->setResultMatchesMathWord(true);
        break;
      }
    }
  }
#ifdef DBG_SHOW_MATHWORDS
  bdgs.StartFullSearch();
  PIX* mathwordim = pixCopy(NULL, blobDataGrid->getBinaryImage());
  mathwordim = pixConvertTo32(mathwordim);
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->belongsToRecognizedMathWord()) {
      M_Utils::drawHlBlobDataRegion(blob, mathwordim, LayoutEval::RED);
    }
  }
  pixWrite((std::string("debug/") + (std::string)"mathwords" + blobDataGrid->getImageName()).c_str(), mathwordim, IFF_PNG);
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
  bdgs.StartFullSearch();
  blob = NULL;
  while((blob = bdgs.NextFullSearch()) != NULL) {
    bool isItalic = false;
    if(blob->getParentWord() != NULL) {
      const FontInfo* const fontInfo = blob->getParentWord()->getWordRes()->fontinfo;
      if(fontInfo != NULL) {
        isItalic = fontInfo->is_italic();
      }
    }
    if(!isItalic)
      ++non_ital_blobs;
    ++total_blobs;
  }
  assert(blobDataGrid->getNonItalicizedRatio() == -1);
  blobDataGrid->setNonItalicizedRatio((double)non_ital_blobs / (double)total_blobs);
#ifdef DBG_SHOW_ITALIC
  bdgs.StartFullSearch();
  PIX* boldital_img = pixCopy(NULL, blobDataGrid->getBinaryImage());
  boldital_img = pixConvertTo32(boldital_img);
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->getParentWord() != NULL) {
      const tesseract::FontInfo* fontInfo = blob->getParentWord()->getWordRes()->fontinfo;
      if(fontInfo != NULL) {
        if(fontInfo->is_italic()) {
          M_Utils::drawHlBlobDataRegion(blob, boldital_img, LayoutEval::RED);
        }
      }
    }
  }
  pixWrite((dbgdir + std::string("bolditalics") + blobDataGrid->getImageName()).c_str(), boldital_img, IFF_PNG);
#ifdef DBG_DISPLAY
  std::cout << "Displaying the blobs which were found by Tesseract to be italicized as red. "
      << "All other blobs are in black.\n";
  std::cout << "The displayed image was saved to "
      << (dbgdir + "bolditalics" + blobDataGrid->getImageName()).c_str() << endl;
  pixDisplay(boldital_img, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&boldital_img);
#endif

  // determine the average ocr confidence for valid words on the page,
  // if there are no valid words the bad_page flag is set to true
  bdgs.StartFullSearch();
  double validblobcount = 0;
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->getParentWord() != NULL && blob->getParentWord()->getIsValidTessWord()) {
      avg_confidence += blob->getCharRecognitionConfidence();
      ++validblobcount;
    }
  }
  avg_confidence /= validblobcount;
#ifdef DBG_CERTAINTY
  std::cout << "Average valid word certainty: " << avg_confidence << endl;
#endif


  // Find out which blobs belong to stop words
  bdgs.StartFullSearch();
  blob = NULL;
#ifdef SHOW_STOP_WORDS
  std::string stpwrdim_name = dbgdir + (std::string)"stop_words_"
            + blobDataGrid->getImageName();
  assert(dbgim == NULL);
  dbgim = pixCopy(NULL, blobDataGrid->getBinaryImage());
  dbgim = pixConvertTo32(dbgim);
#endif
  while((blob = bdgs.NextFullSearch()) != NULL) {
    blob->getParentChar()->getParentWord()->setResultMatchesStopword(stopwordHelper->isStopWord(std::string(blob->getParentChar()->getParentWord()->wordstr())));
#ifdef SHOW_STOP_WORDS
    if(blob->belongsToRecognizedStopword())
      M_Utils::drawHlBlobDataRegion(blob, dbgim, LayoutEval::RED);
#endif
  }
#ifdef SHOW_STOP_WORDS
  pixWrite(stpwrdim_name.c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  std::cout << "Displaying the blobs belonging to stop words in red on the page.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim);
  dbgim = NULL;
#endif

#ifdef SHOW_VALID_WORDS
  std::string validwrdim_name = dbgdir + (std::string)"valid_words_" +
      blobDataGrid->getImageName();
  assert(dbgim == NULL);
  dbgim = pixCopy(NULL, blobDataGrid->getBinaryImage());
  dbgim = pixConvertTo32(dbgim);
  blob = NULL;
  bdgs.StartFullSearch();
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->belongsToRecognizedWord())
      M_Utils::drawHlBlobDataRegion(blob, dbgim, LayoutEval::RED);
  }
  pixWrite(validwrdim_name.c_str(), dbgim, IFF_PNG);
#ifdef DBG_DISPLAY
  std::cout << "Displaying the blobs belonging to valid words in red on the page.\n";
  pixDisplay(dbgim, 100, 100);
  M_Utils::waitForInput();
#endif
  pixDestroy(&dbgim);
  dbgim = NULL;
#endif

  //++imdbgnum;
}



std::vector<DoubleFeature*> OtherRecognitionFeatureExtractor::extractFeatures(BlobData* const blob) {

  std::vector<DoubleFeature*> fv;

  /******** Height flag ********/
  if(heightFlagEnabled) {
    double h = (double)0;
    if(!bad_page) {
      h = (double)blob->getBoundingBox().height() / avg_blob_height;
    } else {
      h = (double)blob->getBoundingBox().height();
    }
    h = M_Utils::expNormalize(h);
    fv.push_back(new DoubleFeature(description, h, description->getHeightFlag()));
  }

  /******** Width/height ratio flag ********/
  if(widthHeightFlagEnabled) {
    double whr = (double)blob->getBoundingBox().width() / (double)blob->getBoundingBox().height();
    if(!bad_page) {
      whr = whr / avg_whr;
    }
    whr = M_Utils::expNormalize(whr);
    fv.push_back(new DoubleFeature(description, whr, description->getWidthHeightFlag()));
  }

  /******** Vertical distance above row baseline flag ********/
  if(vdarbFlagEnabled) {
    double vdarb = (double)0;
    TesseractRowData* const rowData =
        blob->getParentRow();
    if(rowData != NULL) {
      if(rowData->getHasValidTessWord()) {
        double rowheight = -1;
        double avg_baseline_dist_ = (double)-1;
        // find the average baseline for the blob's row
        avg_baseline_dist_ = rowData->avg_baselinedist;
        assert(avg_baseline_dist_ >= 0);
        assert(blob->getParentChar() != NULL); // sanity
        double baseline_dist = findBaselineDist(blob->getParentChar());
        blob->getParentChar()->setDistanceAboveRowBaseline(baseline_dist);
        vdarb = baseline_dist - avg_baseline_dist_;
        rowheight = rowData->row()->bounding_box().height();
        if(vdarb < 0) {
          vdarb = (double)0;
        } else {
          assert(rowheight > 0);
          vdarb /= rowheight;
          vdarb = M_Utils::expNormalize(vdarb);
        }
      }
    }
    fv.push_back(new DoubleFeature(description, vdarb, description->getVdarbFlag()));
  }

  /******** Is OCR math word flag ********/
  if(isOcrMathFlagEnabled) {
    double imw = (double)0;
    if(blob->getParentWord() != NULL) {
      if(blob->getParentWord()->getResultMatchesMathWord()) {
        imw = (double)1;
      }
    }
    fv.push_back(new DoubleFeature(description, imw, description->getIsOcrMathFlag()));
  }

  /******** Is italic flag ********/
  if(isItalicFlagEnabled) {
    double is_italic = (double)0;
    assert(blobDataGrid->getNonItalicizedRatio() >= 0);
    if(blob->getParentWord() != NULL) {
      const FontInfo* fontInfo = blob->getParentWord()->getWordRes()->fontinfo;
      if(fontInfo != NULL) {
        if(fontInfo->is_italic()) {
          is_italic = (double)1 * blobDataGrid->getNonItalicizedRatio();
        }
      }
    }
    fv.push_back(new DoubleFeature(description, is_italic, description->getIsItalicFlag()));
  }

  /******** Confidence flag ********/
  if(confidenceFlagEnabled) {
    double ocr_conf = (double)blob->getCharRecognitionConfidence();
    if(ocr_conf > avg_confidence) {
      ocr_conf = avg_confidence; // don't punish for having more confidence
    }
    ocr_conf /= avg_confidence;
    ocr_conf = M_Utils::expNormalize(ocr_conf);
    fv.push_back(new DoubleFeature(description, ocr_conf, description->getConfidenceFlag()));
  }

  /******** Belongs to Valid OCR Row Flag ********/
  if(isOnValidOcrRowFlagEnabled) {
    double in_valid_row = (double)0;
    TesseractRowData* const rowData = blob->getParentRow();
    if(rowData != NULL) {
      if(rowData->getHasValidTessWord()) {
        in_valid_row = (double)1;
      }
    }
    fv.push_back(new DoubleFeature(description, in_valid_row, description->getIsOnValidOcrRowFlag()));
  }

  /******** Belongs to Valid OCR Word Flag ********/
  if(isOcrValidFlagEnabled) {
    double in_valid_word = (double)0;
    TesseractWordData* const wordData = blob->getParentWord();
    if(wordData != NULL) {
      if(wordData->getIsValidTessWord()) {
        in_valid_word = (double)1;
      }
    }
    fv.push_back(new DoubleFeature(description, in_valid_word, description->getIsOcrValidFlag()));
  }

  /******** Bad OCR Page Flag ********/
  if(isOnBadPageFlagEnabled) {
    double bad_page_ = (double)0;
    if(bad_page) {
      bad_page_ = (double)1;
    }
    fv.push_back(new DoubleFeature(description, bad_page_, description->getIsOnBadPageFlag()));
  }

  /******** Belongs to Stopword Flag ********/
  if(isInOcrStopwordFlagEnabled) {
    double stop_word = (double)0;
    TesseractWordData* const wordData = blob->getParentWord();
    if(wordData != NULL) {
      if(wordData->getResultMatchesStopword()) {
        stop_word = (double)1;
      }
    }
    fv.push_back(new DoubleFeature(description, stop_word, description->getIsInOcrStopwordFlag()));
  }

  return fv;
}

double OtherRecognitionFeatureExtractor::findBaselineDist(TesseractCharData* tessChar) {
  // calculate the current recognized char's distance from the baseline
  TesseractRowData* const row = tessChar->getParentWord()->getParentRow();
  double char_baseline = (double)row->row()->base_line(M_Utils::centerx(*(tessChar->getBoundingBox())));
  double char_bottom = (double)tessChar->getBoundingBox()->bottom();
  double baseline_dist = char_bottom - char_baseline;
  if(baseline_dist < 0) {
    if((double)tessChar->getBoundingBox()->top() < char_baseline)
      baseline_dist = -baseline_dist;
    else
      baseline_dist = 0;
  }
  return baseline_dist;
}

void OtherRecognitionFeatureExtractor::enableVdarbFlag() {
  enabledFlagDescriptions.push_back(description->getVdarbFlag());
  vdarbFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableHeightFlag() {
  enabledFlagDescriptions.push_back(description->getHeightFlag());
  heightFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableWidthHeightFlag() {
  enabledFlagDescriptions.push_back(description->getWidthHeightFlag());
  widthHeightFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsOcrMathFlag() {
  enabledFlagDescriptions.push_back(description->getIsOcrMathFlag());
  isOcrMathFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsItalicFlag() {
  enabledFlagDescriptions.push_back(description->getIsItalicFlag());
  isItalicFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableConfidenceFlag() {
  enabledFlagDescriptions.push_back(description->getConfidenceFlag());
  confidenceFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsOcrValidFlag() {
  enabledFlagDescriptions.push_back(description->getIsOcrValidFlag());
  isOcrValidFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsOnValidOcrRowFlag() {
  enabledFlagDescriptions.push_back(description->getIsOnValidOcrRowFlag());
  isOnValidOcrRowFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsOnBadPageFlag() {
  enabledFlagDescriptions.push_back(description->getIsOnBadPageFlag());
  isOnBadPageFlagEnabled = true;
}

void OtherRecognitionFeatureExtractor::enableIsInOcrStopwordFlag() {
  enabledFlagDescriptions.push_back(description->getIsInOcrStopwordFlag());
  isInOcrStopwordFlagEnabled = true;
}

BlobFeatureExtractorDescription* OtherRecognitionFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}

std::vector<FeatureExtractorFlagDescription*> OtherRecognitionFeatureExtractor
::getEnabledFlagDescriptions() {
  return enabledFlagDescriptions;
}


