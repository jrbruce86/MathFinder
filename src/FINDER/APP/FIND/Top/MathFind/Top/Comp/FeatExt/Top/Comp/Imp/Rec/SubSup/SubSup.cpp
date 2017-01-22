/*
 * SubOrSuperScripts.cpp
 *
 *  Created on: Nov 13, 2016
 *      Author: jake
 */

#include <SubSup.h>

#include <SubSupDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <SubSupData.h>
#include <WordData.h>
#include <CharData.h>
#include <Utils.h>
#include <M_Utils.h>

#include <stddef.h>

#define DBG_SHOW_SUB_SUPER
#define DBG_FEAT3
//#define DBG_DISPLAY
//#define DBG_SUB_SUPER


SubOrSuperscriptsFeatureExtractor
::SubOrSuperscriptsFeatureExtractor(
    SubOrSuperscriptsFeatureExtractorDescription* const description,
    FinderInfo* const finderInfo)
: hasSubFeatureEnabled(false),
  isSubFeatureEnabled(false),
  hasSupFeatureEnabled(false),
  isSupFeatureEnabled(false),
  wordCertaintyThresh(Utils::getCertaintyThresh()),
  wordCertaintyThreshHigh(wordCertaintyThresh / 2) {
  this->description = description;
  this->subSupDir =
      Utils::checkTrailingSlash(
          finderInfo->getFinderTrainingPaths()->getFeatureExtDirPath())
        + std::string("SubSuper/");
}

void SubOrSuperscriptsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  blobSubscriptDataKey = findOpenBlobDataIndex(blobDataGrid);

  // Create the sub/superscript data for each blob in the grid
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.StartFullSearch();
  BlobData* blobData = NULL;
  while((blobData = gridSearch.NextFullSearch()) != NULL) {
    SubOrSuperscriptsData* const data = new SubOrSuperscriptsData();
    assert(blobSubscriptDataKey == blobData->appendNewVariableData(data));
  }

  // Determine the enabled features for each blob in the grid
  gridSearch.StartFullSearch();
  blobData = NULL;
  while((blobData = gridSearch.NextFullSearch()) != NULL) {
    // figure out whether or not the blob has sub/superscripts
    // and update data accordingly
    if(isSubFeatureEnabled || hasSubFeatureEnabled) {
      setBlobSubSuperScript(blobData, blobDataGrid, SUB);
    }
    if(isSupFeatureEnabled || hasSupFeatureEnabled) {
      setBlobSubSuperScript(blobData, blobDataGrid, SUPER);
    }
  }
 #ifdef DBG_SHOW_SUB_SUPER
   Pix* dbgss_im = pixCopy(NULL, blobDataGrid->getBinaryImage());
   dbgss_im = pixConvertTo32(dbgss_im);
   gridSearch.StartFullSearch();
   while((blobData = gridSearch.NextFullSearch()) != NULL) {
     SubOrSuperscriptsData* const ssData = (SubOrSuperscriptsData*)blobData->getVariableDataAt(blobSubscriptDataKey);
     if(ssData->hasSuperscript || ssData->hasSubscript)
       M_Utils::drawHlBlobDataRegion(blobData, dbgss_im, LayoutEval::RED);
     else if(ssData->isSuperscript)
       M_Utils::drawHlBlobDataRegion(blobData, dbgss_im, LayoutEval::GREEN);
     else if(ssData->isSubscript)
       M_Utils::drawHlBlobDataRegion(blobData, dbgss_im, LayoutEval::BLUE);
   }
   if(!(Utils::existsDirectory(subSupDir))) {
     Utils::exec(std::string("mkdir -p ") + subSupDir);
   }
   pixWrite((Utils::checkTrailingSlash(subSupDir) + blobDataGrid->getImageName()).c_str(), dbgss_im, IFF_PNG);
 #ifdef DBG_DISPLAY
   pixDisplay(dbgss_im, 100, 100);
   M_Utils::waitForInput();
 #endif
   pixDestroy(&dbgss_im);

 #endif
}

std::vector<DoubleFeature*> SubOrSuperscriptsFeatureExtractor::extractFeatures(BlobData* const blobData) {
  double has_sup = (double)0, has_sub = (double)0,
      is_sup = (double)0, is_sub = (double)0;

  const double bin_val = (double)1;

  SubOrSuperscriptsData* const blobSubOrSuperscriptData = (SubOrSuperscriptsData*)blobData->getVariableDataAt(blobSubscriptDataKey);

  if(blobSubOrSuperscriptData->hasSubscript)
    has_sub = bin_val;
  if(blobSubOrSuperscriptData->isSubscript)
    is_sub = bin_val;
  if(blobSubOrSuperscriptData->hasSuperscript)
    has_sup = bin_val;
  if(blobSubOrSuperscriptData->isSuperscript)
    is_sup = bin_val;

#ifdef DBG_FEAT3
  if(has_sup || has_sub || is_sup || is_sub) {
    std::cout << "The displayed blob has/is a sub/superscript!\n";
    std::cout << "has_sup: " << has_sup << ", has_sub: " << has_sub
         << ", is_sup: " << is_sup << ", is_sub: "  << is_sub << std::endl;
    M_Utils::dbgDisplayBlob(blobData);
  }
#endif

  if(hasSubFeatureEnabled) {
    blobSubOrSuperscriptData->appendExtractedFeature(
        new DoubleFeature(
            description,
            has_sub,
            description->getHasSubscriptDescription()));
  }

  if(isSubFeatureEnabled) {
    blobSubOrSuperscriptData->appendExtractedFeature(
        new DoubleFeature(
            description,
            is_sub,
            description->getIsSubscriptDescription()));
  }

  if(hasSupFeatureEnabled) {
    blobSubOrSuperscriptData->appendExtractedFeature(
        new DoubleFeature(
            description,
            has_sup,
            description->getHasSuperscriptDescription()));
  }

  if(isSupFeatureEnabled) {
    blobSubOrSuperscriptData->appendExtractedFeature(
        new DoubleFeature(
            description,
            is_sup,
            description->getIsSuperscriptDescription()));
  }

  return blobSubOrSuperscriptData->getExtractedFeatures();
}

// Determines whether or not the blob in question has a super/subscript
// and if it does have a super/subscript then its has_sup/has_sub features
// are set and its sub/superscript blob's is_sup/is_sub feature is set also
void SubOrSuperscriptsFeatureExtractor::setBlobSubSuperScript(BlobData* const blob,
    BlobDataGrid* const blobDataGrid, const SubSuperScript subsuper) {
  // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------
  // if the blob belongs to a word that is both considered 'valid' by Tesseract's dictionary and was
  // recognized with high enough confidence then do some filtering
  if(blob->getWordRecognitionConfidence() > wordCertaintyThresh) {

    // filter blobs that don't reside on the rightmost character of a
    // word that matches up to a 'valid' one based on Tesseract's dictionary
    // (characters having sub/superscripts would have to be the rightmost
    // in their word)
    if(blob->belongsToRecognizedWord() && !blob->isRightmostInWord()) {
      return;
    }

    // filter blobs that are parts of words ending in punctuation from
    // being candidates of having subscripts. doesn't matter if the word
    // matches a valid entry in the dictionary or not, but the confidence
    // has to be extra high instead.
    const char lastChar = blob->getParentWordstr() == NULL ?
        'x' : blob->getParentWordstr()[strlen(blob->getParentWordstr()) - 1];
    if(subsuper == SUB &&
        (blob->getWordRecognitionConfidence() > wordCertaintyThreshHigh) &&
        (lastChar == '.'
            || lastChar == ','
                || lastChar == '?')) {
      return;
    }
  }

  // ----------------COMMENT AND/OR CODE IN QUESTION END-----------------------

  // Get pointer to relevant data in this blob
  SubOrSuperscriptsData* const data = (SubOrSuperscriptsData*)blob->getVariableDataAt(blobSubscriptDataKey);

  inT16 h_adj_thresh = blob->getBoundingBox().width() / 2;
  inT32 area_thresh = blob->getBoundingBox().area() / 8;
  inT16 blob_center_y = M_Utils::centery(blob->getBoundingBox());
  inT16 blob_right = blob->getBoundingBox().right() + 1;
  // do repeated beam searches starting from the vertical center
  // of the current blob, going downward for subscripts and upward for superscripts
  BlobDataGridSearch beamsearch(blobDataGrid);
  //beamsearch.SetUniqueMode(true);
  for(int i = 0; i < blob->getBoundingBox().height() / 2; ++i) {
    int j = (subsuper == SUPER) ? i : -i;
    beamsearch.StartSideSearch(blob_right, blob_center_y + j, blob_center_y + j + 1);
    BlobData* neighbor = beamsearch.NextSideSearch(false);
    if(neighbor == NULL)
      continue;
    bool nothing = false;
    while(neighbor->getBoundingBox() == blob->getBoundingBox()
        || neighbor->getBoundingBox().right() <= blob->getBoundingBox().right()
        || neighbor->getBoundingBox().left() <= (blob->getBoundingBox().right() - h_adj_thresh)
        || neighbor->getBoundingBox().bottom() > blob->getBoundingBox().top()
        || neighbor->getBoundingBox().top() < blob->getBoundingBox().bottom()) {
      neighbor = beamsearch.NextSideSearch(false);
      if(neighbor == NULL) {
        nothing = true;
        break;
      }
    }
    if(nothing)
      continue;
    if(neighbor->getBoundingBox().area() < area_thresh)
      continue; // too small
    inT16 h_dist = neighbor->getBoundingBox().left() - blob->getBoundingBox().right();
    if(h_dist > h_adj_thresh)
      continue; // too far away
    // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------
    if(neighbor->belongsToRecognizedWord()
        && neighbor->getWordRecognitionConfidence() > wordCertaintyThresh) {
      if(!neighbor->isLeftmostInWord())
        continue; // if the super/subscript is in a valid word, it has to be at the first letter
    }
    // ----------------COMMENT AND/OR CODE IN QUESTION START---------------------

    // get the neighbor's relevant data so I can update it based on results
    SubOrSuperscriptsData* const neighborData = (SubOrSuperscriptsData*)neighbor->getVariableDataAt(blobSubscriptDataKey);
    if(subsuper == SUPER) {
      if(neighbor->getBoundingBox().bottom()
          > (blob_center_y -
              ((blob_center_y -
                  blob->getBoundingBox().bottom())
                  /8))) {
#ifdef DBG_SUB_SUPER
        dbgSubSuper(blob, neighbor, subsuper);
#endif
        data->hasSuperscript = true;
        neighborData->isSuperscript = true;
        break;
      }
    }
    else if(subsuper == SUB) {
      if(neighbor->getBoundingBox().top() < blob_center_y) {
#ifdef DBG_SUB_SUPER
        dbgSubSuper(blob, neighbor, subsuper);
#endif
        data->hasSubscript = true;
        neighborData->isSubscript = true;
        break;
      }
    }
    else {
      std::cout << "ERROR: Invalid setBlobSubSuperScript option\n";
      assert(false);
    }
  }
}

void SubOrSuperscriptsFeatureExtractor::enableHasSubFeature() {
  enabledFlagDescriptions.push_back(description->getHasSubscriptDescription());
  hasSubFeatureEnabled = true;
}

void SubOrSuperscriptsFeatureExtractor::enableIsSubFeature() {
  enabledFlagDescriptions.push_back(description->getIsSubscriptDescription());
  isSubFeatureEnabled = true;
}

void SubOrSuperscriptsFeatureExtractor::enableHasSupFeature() {
  enabledFlagDescriptions.push_back(description->getHasSuperscriptDescription());
  hasSupFeatureEnabled = true;
}

void SubOrSuperscriptsFeatureExtractor::enableIsSupFeature() {
  enabledFlagDescriptions.push_back(description->getIsSuperscriptDescription());
  isSupFeatureEnabled = true;
}

BlobFeatureExtractorDescription* SubOrSuperscriptsFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}

std::vector<FeatureExtractorFlagDescription*> SubOrSuperscriptsFeatureExtractor::getEnabledFlagDescriptions() {
  return enabledFlagDescriptions;
}


void SubOrSuperscriptsFeatureExtractor::dbgSubSuper(BlobData* blob, BlobData* neighbor, SubSuperScript subsuper) {
  std::cout << "found a " << ((subsuper == SUPER) ? "super" : "sub")
       << "-script for the displayed blob\n";
  std::cout << "here's the recognition result for that blob's word: "
       << ((blob->getParentWordstr() == NULL) ? "NULL" : blob->getParentWordstr()) << std::endl;
  if(blob->getParentWord() == NULL)
    std::cout << "no blobs were recognized in the blob's word!\n";
  std::cout << "here's the confidence Tesseract had in its result for the blob's word: " << blob->getWordRecognitionConfidence()
      << ", and for the neighbor: " << neighbor->getWordRecognitionConfidence() << std::endl;
  M_Utils::dbgDisplayBlob(blob);
  std::cout << "displayed is the previous blob's "
       << ((subsuper == SUPER) ? "super" : "sub") << "-script\n";
  std::cout << "here's the recognition result for that neighbor's word: "
       << ((neighbor->getParentWordstr() == NULL) ? "NULL" : neighbor->getParentWordstr()) << std::endl;
  if(neighbor->getParentWord() == NULL)
    std::cout << "no blobs were recognized in the neighbor's word!\n";
  M_Utils::dbgDisplayBlob(neighbor);
}


