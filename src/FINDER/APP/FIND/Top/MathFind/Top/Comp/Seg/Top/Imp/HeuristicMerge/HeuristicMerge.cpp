/**************************************************************************
 * File name:   HeuristicMerge.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 27, 2013 1:29:14 AM
 *              Updated Oct 30, 2016
 ***************************************************************************/
#include <HeuristicMerge.h>

#include <FeatExt.h>
#include <AlignedFeatExt.h>
#include <AlignedFac.h>
#include <StackedFeatExt.h>
#include <MFinderResults.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <M_Utils.h>
#include <Utils.h>
#include <BlobMergeData.h>
#include <Direction.h>
#include <AlignedData.h>
#include <WordData.h>
#include <CharData.h>
#include <AlignedDesc.h>
#include <StackedDesc.h>
#include <RowData.h>

#include <baseapi.h>

#include <allheaders.h>

#include <vector>
#include <iostream>
#include <stddef.h>
#include <assert.h>

//#define SHOW_DETECTION_RESULTS // shows the results of detection prior to any cleanup or segmentation
//#define SHOW_PASS1 // shows the results of the first pass (cleanup prior to segmentation)
//#define DBG_SHOW_MERGE_START  // shows the starting blob for a merge
//#define DBG_SHOW_MERGE // shows all the merges that happen between DBG_SHOW_MERGE_START and
//#define DBG_SHOW_MERGE_ONE_SEGMENT // if turned on, only debugs a segment
//                                   // of interest rather than threquires DBG_SHOW_MERGE, DBG_SHOW_MERGE_FINAL, and/or
//                                   // DBG_SHOW_MERGE_START to be turned on, only debugs a segment
//                                   // of interest rather than the entire page
//#define DBG_MERGE_VERBOSE // show everything that is being added to the merge list
//#define DBG_MERGE_INTERSECTING // show all the blobs that were merged because they intersect with a segment
//#define DBG_H_ADJACENT
//#define DBG_SHOW_MERGE_FINAL // shows the result of merging
//#define SHOW_SEGIDS
//#define SHOW_RECURSIONS
//#define SHOW_PASSES

bool g_dbg_flag = false;

HeuristicMerge::HeuristicMerge(MathExpressionFeatureExtractor* const featureExtractor)
: dbgim(NULL), highCertaintyThresh(Utils::getCertaintyThresh() / 2) {
  this->featureExtractor = featureExtractor;
  this->numAlignedBlobsFeatureExtractor = dynamic_cast<NumAlignedBlobsFeatureExtractor*>(getFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription::getName_()));
  this->numVerticallyStackedFeatureExtractor = dynamic_cast<NumVerticallyStackedBlobsFeatureExtractor*>(getFeatureExtractor(NumVerticallyStackedBlobsFeatureExtractorDescription::getName_()));
  this->otherFeatureExtractor = dynamic_cast<OtherRecognitionFeatureExtractor*>(getFeatureExtractor(OtherRecognitionFeatureExtractorDescription::getName_()));
}

void HeuristicMerge::runSegmentation(BlobDataGrid* const blobDataGrid) {
  // now do the segmentation step
  dbgim = blobDataGrid->getImage(); // allows for optional debugging

  numAlignedBlobsFeatureExtractor->doSegmentationInit(blobDataGrid);
  numVerticallyStackedFeatureExtractor->doSegmentationInit(blobDataGrid);

  this->blobDataGrid = blobDataGrid;

#ifdef SHOW_DETECTION_RESULTS
  {
    Pix* detIm = blobDataGrid->getVisualDetectionResultsDisplay();
    pixDisplay(detIm, 100, 100);
    std::cout << "Displaying the results of detection prior to any segmentation.\n";
    Utils::waitForInput();
    pixDestroy(&detIm);
  }
#endif

  // pass 1: does some post-processing to get rid of blobs sparsely detected within recognized words
  // marked as valid by Tesseract with high confidence. Gets rid of any blob detected as math
  // that's within a stop word recognized with high confidence by Tesseract. Also gets rid of
  // blobs that are likely to be part of a header (i.e., a number at the top left).
  {
#ifdef SHOW_PASSES
    std::cout << "Starting segmentation pass 1\n";
#endif
    BlobDataGridSearch bdgs(blobDataGrid);
    bdgs.SetUniqueMode(true);
    bdgs.StartFullSearch();
    BlobData* curblob = NULL;
    const double sparseness_threshold = .6; // minimum acceptable ratio of math blobs to total blobs in a recognized as valid non-math word
    while((curblob = bdgs.NextFullSearch()) != NULL) {
      if(curblob->getParentWord() == NULL ||
          !curblob->getMathExpressionDetectionResult()) {
        // all of the below filters assume blob was recognized as math and
        // is also part of some word recognized by Tesseract
        continue;
      }

      // Note:: confidence/certainty are used interchangeably to mean the same thing
      //        tesseract's word confidence is equal to the lowest blob confidence in the word
      // Filter blobs in words that are likely to be false positives based on
      // Tesseract's confidence metric. If they match my math dictionary and have
      // medium confidence I let them pass. But ones that don't meet that criteria
      // and match a Tesseract dictionary entry with low confidence, have medium confidence
      // or that have a high average confidence (average confidence of blobs as opposed to minimum)
      // For blobs passing the above criteria:
      // Any blobs in stopwords recognized with low confidence are discarded as nonmath.
      // Other blobs are set to non math if the word they are in has a lower math
      // to nonmath ratio than the sparseness threshold.
      const float mediumConf = Utils::getCertaintyThresh();
      const float lowConf = mediumConf * 2;
      const float wordConf = curblob->getWordRecognitionConfidence();
      if(!(curblob->belongsToRecognizedMathWord() && wordConf >= mediumConf)
          &&  (
              (curblob->belongsToRecognizedWord() && wordConf >= lowConf)
              || (curblob->getWordRecognitionConfidence() >= mediumConf)
              || (curblob->getWordAvgRecognitionConfidence() >= highCertaintyThresh)
          )
      ){
        int num_math_in_word = 0;
        if(curblob->belongsToRecognizedStopword()) {
          curblob->setMathExpressionDetectionResult(false);
          continue;
        }
        std::vector<BlobData*> wordChildBlobs = getBlobsWithSameParent(curblob);
        const int numwrdblobs = wordChildBlobs.size();
        for(int i = 0; i < numwrdblobs; ++i) {
          BlobData* const wrdblob = wordChildBlobs[i];
          if(wrdblob->getMathExpressionDetectionResult()) {
            ++num_math_in_word;
          }
        }
        double math_non_math_wrd_ratio = (double)num_math_in_word / (double)numwrdblobs;
        if(math_non_math_wrd_ratio < sparseness_threshold) {
          for(int i = 0; i < numwrdblobs; ++i) {
            wordChildBlobs[i]->setMathExpressionDetectionResult(false);
          }
        }
      }
    }
    // Any blobs on the top row are assumed to be header....
    GenericVector<TesseractWordData*> topRowWords =
        blobDataGrid->getAllTessRows()[0]->getTesseractWords();
    for(int i = 0; i < topRowWords.size(); ++i) {
      std::vector<TesseractCharData*> chars = topRowWords[i]->getChildChars();
      for(int j = 0; j < chars.size(); ++j) {
        std::vector<BlobData*> blobs = chars[j]->getBlobs();
        for(int k = 0; k < blobs.size(); ++k) {
          blobs[k]->setMathExpressionDetectionResult(false);
        }
      }
    }

#ifdef SHOW_PASS1
    {
      Pix* pass1Im = blobDataGrid->getVisualDetectionResultsDisplay();
      pixDisplay(pass1Im, 100, 100);
      std::cout << "Displaying the results of segmentation pass 1.\n";
      Utils::waitForInput();
      pixDestroy(&pass1Im);
    }
#endif
  }

  // pass 1.5: Go through and get rid of any previously marked covered regions
  //           While some code is shared between feature extracton and segmentation
  //           They use different paramaters to do what they need....
  {
    BlobDataGridSearch bdgs(blobDataGrid);
    bdgs.StartFullSearch();
    bdgs.SetUniqueMode(true);
    BlobData* curblob = NULL;
    while((curblob = bdgs.NextFullSearch()) != NULL) {
      NumAlignedBlobsData* numAlignedBlobsData =
          numAlignedBlobsFeatureExtractor->getBlobFeatureData(curblob);
      numAlignedBlobsData->dvabc_blobs.clear();
      numAlignedBlobsData->uvabc_blobs.clear();
      numAlignedBlobsData->lhabc_blobs.clear();
      numAlignedBlobsData->rhabc_blobs.clear();
    }
  }

  // pass 2: run the segmentation algorithm
  {
#ifdef SHOW_PASSES
    std::cout << "Running segmentation pass 2.\n";
#endif
    BlobDataGridSearch bdgs(blobDataGrid);
    bdgs.StartFullSearch();
    bdgs.SetUniqueMode(true);
    BlobData* curblob = NULL;
    int seg_id = 0; // unique id for each segment, incremented after each segment completed
    while((curblob = bdgs.NextFullSearch()) != NULL) {
      if(!curblob->getMathExpressionDetectionResult() || curblob->getMergeData() != NULL) {
        continue;
      }
#ifdef DBG_SHOW_MERGE_START
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      if(seg_id == 53) {
        g_dbg_flag = true;
      } else {
        g_dbg_flag = false;
      }
      if(g_dbg_flag) {
#endif
        std::cout << "About to start the merging process from the displayed blob:\n";
        M_Utils::dispHlBlobDataRegion(curblob, dbgim);
        M_Utils::dispBlobDataRegion(curblob, dbgim);
        M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      }
#endif
#endif
#ifdef SHOW_SEGIDS
      std::cout << "Start decideandmerge... segid=" << seg_id << std::endl;
#endif
      mergeRecursions = 0;
      decideAndMerge(curblob, seg_id); // recursively merges a blob to its neighbors to create a segmentation
#ifdef SHOW_SEGIDS
      std::cout << "done decideandmerge... segid=" << seg_id << ", took " << mergeRecursions << " recursions.\n";
#endif
#ifdef DBG_SHOW_MERGE_FINAL
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      if(g_dbg_flag) {
#endif
        std::cout << "completed merge!\n";
        std::cout << "Displaying the finalized segment!\n";
        std::cout << "This is the segment with id: " << curblob->getMergeData().seg_id << std::endl;
        TBOX* seg_tbox = curblob->getMergeData().segment_box;
        M_Utils::dispHlTBoxRegion(*seg_tbox, dbgim);
        M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      }
#endif
#endif
      ++seg_id;
    }
  }

  // pass 3: Merge all segments fully/nearly contained within other segments
  {
#ifdef SHOW_PASSES
    std::cout << "Starting segmentation pass 3\n";
#endif
    BlobDataGridSearch fullGridSearch(blobDataGrid);
    fullGridSearch.StartFullSearch();
    fullGridSearch.SetUniqueMode(true);
    BlobData* curBlob = NULL;
    while((curBlob = fullGridSearch.NextFullSearch()) != NULL) {
      if(curBlob->getMergeData() != NULL) {
        TBOX* const biggerSegmentBox = curBlob->getMergeData()->getSegBox();
        BlobDataGridSearch biggerSegmentSearch(blobDataGrid);
        biggerSegmentSearch.SetUniqueMode(true);
        biggerSegmentSearch.StartRectSearch(*biggerSegmentBox);
        BlobData* overlappingBlob = NULL;
        while((overlappingBlob = biggerSegmentSearch.NextRectSearch()) != NULL) {
          if(overlappingBlob->getMergeData() != NULL) {
            if(overlappingBlob->getMergeData()->getSegId() == curBlob->getMergeData()->getSegId()) {
              // already belongs to the expected segment, move on
              continue;
            }
            TBOX* const smallerSegmentBox = overlappingBlob->getMergeData()->getSegBox();
            if(M_Utils::almostContains(*biggerSegmentBox, *smallerSegmentBox)) {
              BlobMergeData* const biggerSegment = curBlob->getMergeData();
              BlobMergeData* const smallerSegment = overlappingBlob->getMergeData();

              // Remove reference to smaller segment from the grid's results list
              blobDataGrid->removeSegmentation(smallerSegment->getSegmentation());

              // Delete the shared reference to the smaller segment and nullify
              delete *(overlappingBlob->getMergeDataSharedPtr());
              *(overlappingBlob->getMergeDataSharedPtr()) = NULL;

              // Assign all of the blobs in the smaller box to their new segmentation
              {
                BlobDataGridSearch smallerSegmentSearch(blobDataGrid);
                smallerSegmentSearch.SetUniqueMode(true);
                smallerSegmentSearch.StartRectSearch(*smallerSegmentBox);
                BlobData* smallerSegBlob = NULL;
                while((smallerSegBlob = smallerSegmentSearch.NextRectSearch()) != NULL) {
                  if(smallerSegBlob->getMergeData() == NULL) {
                    smallerSegBlob->setToExistingMergeData(curBlob->getMergeDataSharedPtr());
                  }
                }
              }
            }
          }
        }
      }
    }
  }

#ifdef SHOW_PASSES
  std::cout << "finished segmentation.....\n";
#endif
}

// make the merge decision for left, right, up, and down
// carry out the merge operation(s) for left, right, up, or down if applicable
void HeuristicMerge::decideAndMerge(BlobData* blob,
    const int& seg_id) {
  if(mergeRecursions++ > 30) {
    std::cout << "ERROR Exceeded 30 recursions. Exiting.\n";
    return; // obviously too many done.
  }
#ifdef SHOW_RECURSIONS
  std::cout << "merge recursion " << mergeRecursions << std::endl;
#endif

  // initialize the blob's segmentation if it hasn't been initialized yet
  // if it's not part of an existing segment make a new one for it
  if(blob->getMergeData() == NULL) {
    Segmentation* seg = new Segmentation;
    seg->box = new TBOX(
        blob->getBoundingBox().left(),
        blob->getBoundingBox().bottom(),
        blob->getBoundingBox().right(),
        blob->getBoundingBox().top());
    RESULT_TYPE segRes;
    if(blob->belongsToRecognizedNormalRow())
      segRes = EMBEDDED;
    else
      segRes = DISPLAYED;
    seg->res = segRes;
    blobDataGrid->appendSegmentation(seg); // the grid just owns a shallow copy
    blob->setToNewMergeData(seg, seg_id);
  }

  BlobMergeData* const blob_merge_info = blob->getMergeData();
  blob_merge_info->clearBuffers();
  if(blob_merge_info->getSegId() != seg_id) {
    return; // this blob was already added to a different segment
  }
//  M_Utils::dispHlBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//  M_Utils::dispBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//  //M_Utils::waitForInput();
//  int total = blob_merge_info.down.size()
//      + blob_merge_info.up.size()
//      + blob_merge_info.right.size()
//      + blob_merge_info.left.size()
//      + blob_merge_info.intersecting.size();
//  std::cout << "total blobs " << total << std::endl;
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Starting the recursive algorithm with the displayed region at segment id " << seg_id << " and also showing the current blob.:\n";
    M_Utils::dispHlBlobDataSegmentation(blob, dbgim);
    M_Utils::dispBlobDataRegion(blob, dbgim);
    std::cout << "seg area " << blob_merge_info->getSegBox()->area() << std::endl;
    M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
  // Quick sanity check
  assert(blob_merge_info->getSegId() > -1);
  if(blob_merge_info->getSegId() != seg_id) {
    assert(false);
  }

  // figure out which blobs aught to be merged to the current segmentation
  mergeDecision(blob, BlobSpatial::LEFT);
  mergeDecision(blob, BlobSpatial::RIGHT);
  mergeDecision(blob, BlobSpatial::UP);
  mergeDecision(blob, BlobSpatial::DOWN);
  checkIntersecting(blob); // see if there's an unprocessed blob that intersects current segment

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Found " << blob->getMergeData().intersecting.length() << " blobs that intersect with the "
        << "current segmentation.\n";
#ifdef DBG_MERGE_INTERSECTING
    const GenericVector<BlobData*>& intersecting_blobs = blob->getMergeData().intersecting;
    for(int i = 0; i < intersecting_blobs.length(); ++i) {
      std::cout << "Displaying intersecting blob " << i << std::endl;
      M_Utils::dispBlobDataRegion(intersecting_blobs[i], dbgim);
      M_Utils::waitForInput();
    }
#endif
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
  // carry out the merge operation in the applicable directions
  bool mergeCarriedOut = blob_merge_info->hasNonEmptyBuffer();
  const GenericVector<BlobData*>& mergedown = blob_merge_info->down;
  for(int i = 0; i < mergedown.length(); ++i) {
    if(mergedown[i]->getMergeData() == NULL) {
      mergeOperation(blob, mergedown[i], BlobSpatial::DOWN);
    }
  }
  const GenericVector<BlobData*>& mergeup = blob_merge_info->up;
  for(int i = 0; i < mergeup.length(); ++i) {
    if(mergeup[i]->getMergeData() == NULL) {
      mergeOperation(blob, mergeup[i], BlobSpatial::UP);
    }
  }
  const GenericVector<BlobData*>& mergeright = blob_merge_info->right;
  for(int i = 0; i < mergeright.length(); ++i) {
    if(mergeright[i]->getMergeData() == NULL) {
      mergeOperation(blob, mergeright[i], BlobSpatial::RIGHT);
    }
  }
  const GenericVector<BlobData*>& mergeleft = blob_merge_info->left;
  for(int i = 0; i < mergeleft.length(); ++i) {
    if(mergeleft[i]->getMergeData() == NULL) {
      mergeOperation(blob, mergeleft[i], BlobSpatial::LEFT);
    }
  }
  const GenericVector<BlobData*>& intersecting = blob_merge_info->intersecting;
  for(int i = 0; i < intersecting.length(); ++i) {
    if(intersecting[i]->getMergeData() == NULL) {
      mergeOperation(blob, intersecting[i], BlobSpatial::INTERSECT);
    }
  }

   //Look for horizontal merge on updated segment prior to jumping into recursion
  if(g_dbg_flag) {
    std::cout << "Looking for rightward merge.\n";
  }
  BlobData* const hMergeRight = lookForHorizontalMerge(
      blob->getMergeData()->getSegBox(), blobDataGrid, BlobSpatial::RIGHT, seg_id);
  if(g_dbg_flag) {
    std::cout << "Looking for leftward merge\n";
  }
  BlobData* const hMergeLeft = lookForHorizontalMerge(
      blob->getMergeData()->getSegBox(), blobDataGrid, BlobSpatial::LEFT, seg_id);
  if(hMergeRight != NULL) {
    if(hMergeRight->getMergeData() == NULL) {
#ifdef DBG_H_ADJACENT
      std::cout << "Merging the blob to the right.\n";
#endif
      mergeOperation(blob, hMergeRight, BlobSpatial::RIGHT);
      mergeCarriedOut = true; // so know we need to recurse
    }
#ifdef DBG_H_ADJACENT
    else {
      std::cout << "Can't merge horizontal adjacent right since blob owned by a segment\n";
    }
#endif
  }
  if(hMergeLeft != NULL) {
    if(hMergeLeft->getMergeData() == NULL) {
#ifdef DBG_H_ADJACENT
      std::cout << "Merging the blob to the left.\n";
#endif
      mergeOperation(blob, hMergeLeft, BlobSpatial::LEFT);
      mergeCarriedOut = true; // so know we need to recurse
    }
#ifdef DBG_H_ADJACENT
    else {
      std::cout << "Can't merge horizontal adjacent left since blob owned by another segment\n";
    }
#endif
  }

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
  if(!mergeCarriedOut) {
    std::cout << "No merges. Here's the current segmentation:\n";
    M_Utils::dispHlBlobDataSegmentation(blob, dbgim);
    M_Utils::waitForInput();
  }
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif

  // recursively repeat for the newly merged segmentation
  // don't recurse if no merge carried out
  if(mergeCarriedOut) {
    // just need to do merge on one blob doesn't matter which as
    // they are all part of the same segmentation
    decideAndMerge(blob, seg_id);
  }
}

void HeuristicMerge::mergeDecision(BlobData* blob, BlobSpatial::Direction dir) {
  assert(dir == BlobSpatial::LEFT || dir == BlobSpatial::RIGHT
      || dir == BlobSpatial::UP || dir == BlobSpatial::DOWN);
  BlobMergeData* merge_info = blob->getMergeData();
//std::cout << "in mergeDecision " << (dir == BlobSpatial::LEFT ? "left."
//    : dir == BlobSpatial::RIGHT ? "right." : dir == BlobSpatial::UP ?
//        "up." : "down.") << "\n";
  // Get the blob's data associated with the feature extractor
  NumAlignedBlobsData* numAlignedBlobsData = numAlignedBlobsFeatureExtractor->getBlobFeatureData(blob);
  numAlignedBlobsData->clearBuffers(); // only care about what is computed here (not on prev recursions)

  //GenericVector<BlobData*> stacked_merges;

  numAlignedBlobsFeatureExtractor->countCoveredBlobs(
      blob, blobDataGrid, dir, true, merge_info->getSegId());

  // horizontal merge decision
  GenericVector<BlobData*> covered_merges; // stores blob merged by the "cover feature"
  if(dir == BlobSpatial::LEFT || dir == BlobSpatial::RIGHT) {
    assert(covered_merges.empty());
    if(dir == BlobSpatial::LEFT) {
      covered_merges = filterAlreadyMerged(numAlignedBlobsData->lhabc_blobs);
    } else {
      covered_merges = filterAlreadyMerged(numAlignedBlobsData->rhabc_blobs);
    }
  }
  else { // vertical merge decision
    assert(covered_merges.empty());
    if(dir == BlobSpatial::DOWN)
      covered_merges = filterAlreadyMerged(numAlignedBlobsData->dvabc_blobs);
    else
      covered_merges = filterAlreadyMerged(numAlignedBlobsData->uvabc_blobs);
  }

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Found " << covered_merges.length() << " " <<
        ((dir == BlobSpatial::UP) ? "upward" : (dir == BlobSpatial::DOWN) ? "downward"
            : (dir == BlobSpatial::RIGHT) ? "rightward" : "leftward") << " covered merges\n";
#ifdef DBG_MERGE_VERBOSE
    for(int i = 0; i < covered_merges.length(); ++i) {
      std::cout << "Displaying covered merge " << i << std::endl;
      M_Utils::dispBlobDataRegion(covered_merges[i], dbgim);
      covered_merges[i]->bounding_box().print();
      M_Utils::waitForInput();
    }
#endif
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif

  // add all the blobs to be merged in the given direction onto the mergeinfo for this blob
  GenericVector<BlobData*>& merge_list = (dir == BlobSpatial::UP) ? merge_info->up :
      (dir == BlobSpatial::DOWN) ? merge_info->down : (dir == BlobSpatial::RIGHT) ? merge_info->right :
          merge_info->left;
  for(int i = 0; i < covered_merges.length(); ++i) {
    if(covered_merges[i]->getMergeData() != NULL) {
      continue; // the covered blob is part of a different segment or was already
                // merged to this one so should not be merged again
    }
    merge_list.push_back(covered_merges[i]);
  }
}


/**
 * Look horizontally for a blob adjacent to either the leftmost or rightmost
 * blob in the given segment box. Only works horizontally not vertically.
 * Returns the horizontally adjacent blob if found, otherwise returns null
 *
 */
BlobData* HeuristicMerge::lookForHorizontalMerge(
    TBOX* const segmentBox, BlobDataGrid* const blobDataGrid,
    BlobSpatial::Direction dir, const int& segId) {
  assert(isHorizontal(dir));
  bool leftToRight = false;
  if(dir == BlobSpatial::RIGHT) {
    leftToRight = true;
  }
  // Get the rightmost/leftmost blob in the segment
  BlobData* sideBlob = NULL;
  TBOX segmentBoxVal = *segmentBox;
  BlobDataGridSearch bdgs(blobDataGrid);
  if(leftToRight) {
    bdgs.StartSideSearch(segmentBox->right(), segmentBox->bottom(), segmentBox->top());
    sideBlob = bdgs.NextSideSearch(true); // find the blob all the way to the top right
  } else {
    bdgs.StartSideSearch(segmentBox->left(), segmentBox->bottom(), segmentBox->top());
    sideBlob = bdgs.NextSideSearch(false); // find the blob all the way to the top left
  }
  if(sideBlob == NULL) {
    if(g_dbg_flag) {
      std::cout << "Couldn't find sideblob!!!!\n";
    }
    return NULL;
  } else if(!sideBlob->bounding_box().overlap(segmentBoxVal)) {
    if(g_dbg_flag) {
      std::cout << "Found sideblob but doesn't intersect the segment!!!!\n";
    }
    return NULL;
  } else {
    if(g_dbg_flag) {
      std::cout << "Found sideblob that intersects the segment. showing on top of the highlighted segmentation\n";
      M_Utils::dispHlTBoxRegion(segmentBoxVal, blobDataGrid->getBinaryImage());
      M_Utils::dispBlobDataRegion(sideBlob, blobDataGrid->getBinaryImage());
      M_Utils::waitForInput();
    }
  }

  bdgs.StartSideSearch(leftToRight ? segmentBox->right() : segmentBox->left(),
      segmentBox->bottom(), segmentBox->top());
  bdgs.SetUniqueMode(true);
  BlobData* n = bdgs.NextSideSearch(!leftToRight);
  while((n->bounding_box() == sideBlob->bounding_box())
      || segmentBox->contains(n->bounding_box())
      || n->getMergeData() != NULL
      || (leftToRight ? (n->left() <= segmentBox->right() || n->left() <= sideBlob->right())
          : (n->right() >= segmentBox->left() || n->right() >= sideBlob->left()))
      || (n->top() < segmentBox->bottom())
      || (n->bottom() > segmentBox->top()))
  {
    n = bdgs.NextSideSearch(!leftToRight);
    if(n == NULL) {
#ifdef DBG_SHOW_MERGE
      if(g_dbg_flag) {
        std::cout << "Couldn't find neighbor to sideblob\n";
      }
#endif
      return NULL;
    }
    if(n->getMergeData() != NULL) {
      if(n->getMergeData()->getSegId() == segId) {
        if(g_dbg_flag) {
          std::cout << "Can't add neighbor to merge since already part of this segment.\n";
          std::cout << "segbox: "; M_Utils::dispTBoxAsCoords(segmentBoxVal);
          std::cout << "neighbor: "; M_Utils::dispTBoxAsCoords(n->bounding_box());
          std::cout << "sideblob: "; M_Utils::dispTBoxAsCoords(sideBlob->bounding_box());
          std::cout << "Showing neighbor on top of highlighted segment.\n";
          M_Utils::dispHlTBoxRegion(segmentBoxVal, blobDataGrid->getBinaryImage());
          M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
          M_Utils::waitForInput();
        }
      } else {
#ifdef DBG_SHOW_MERGE
        if(g_dbg_flag) {
          std::cout << "Can't add neighbor to merge since owned by different one\n";
        }
#endif
        return NULL;
      }
    }
  }
  if(n != NULL) {
#ifdef DBG_SHOW_MERGE
    if(g_dbg_flag) {
      std::cout << "checking adjacent!!!!!!!!!!!!!!!\n";
    }
#endif
    if(NumVerticallyStackedBlobsFeatureExtractor::isAdjacent(n, sideBlob, dir, true))
    {
#ifdef DBG_H_ADJACENT
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      if(g_dbg_flag) {
#endif
      std::cout << "Found a horizontal merge in the " << (leftToRight ? "rightward" : "leftward") << " direction. Showing the adjacent blob.\n";
      M_Utils::dispHlTBoxRegion(segmentBoxVal, blobDataGrid->getBinaryImage());
      M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
      M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      }
#endif
#endif
#ifdef DBG_SHOW_MERGE
      if(g_dbg_flag) {
        std::cout << "Found an adjacent!!!!!!!!!!!!!!!1\n";
      }
#endif
      bool passesFilter = true;
      if(n->belongsToRecognizedWord()
          && n->getWordAvgRecognitionConfidence() > Utils::getCertaintyThresh() * 2
          && n->getCharRecognitionConfidence() > Utils::getCertaintyThresh()) {
        if(!n->belongsToRecognizedMathWord()) {
#ifdef DBG_SHOW_MERGE
          if(g_dbg_flag) {
            std::cout << "However, will not merge since the neighbor belongs to a recognized non-math word with good confidence.\n";
            std::cout << "Avg word rec conf: " << n->getWordAvgRecognitionConfidence() << std::endl;
            std::cout << "Char rec conf: " << n->getCharRecognitionConfidence() << std::endl;
            M_Utils::waitForInput();
          }
#endif
          passesFilter = false;
        }
      }
      if(passesFilter) {
        return n;
      }
    }

    // if there weren't any covered or horizontally adjacent blobs found yet
    // see if there's something directly to the right or left of the current
    // blob that may constitute an operator or operand depending upon the situation

    // is the current symbol an operator? if so whatever was found in the given
    // direction should be an operand!
    if(isOperator(sideBlob)) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      if(g_dbg_flag) {
#endif
#ifdef DBG_H_ADJACENT
        std::cout << "The horizontally adjacent blob is an operand to the sideblob which is " << sideBlob->getParentCharStr() << std::endl;
        M_Utils::dispHlTBoxRegion(segmentBoxVal, blobDataGrid->getBinaryImage());
        M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
        M_Utils::waitForInput();
#endif
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
      }
#endif
#endif
      return n;
    }
    else { // if whatever was found is an operator then this is likely an operand to it!
      if(isOperator(n)) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
        if(g_dbg_flag) {
#endif
#ifdef DBG_H_ADJACENT
          std::cout << "The horizontally adjacent blob is an operator: " << n->getParentCharStr() << std::endl;
          M_Utils::dispHlTBoxRegion(segmentBoxVal, blobDataGrid->getBinaryImage());
          M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
          M_Utils::waitForInput();
#endif
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
        }
#endif
#endif
        return n;
      }
#ifdef DBG_SHOW_MERGE
      else {
        std::cout << "The neighbor is not an operator: " << n->getParentCharStr() << std::endl;
      }
#endif
    }
  }
  return NULL;
}

void HeuristicMerge::checkIntersecting(BlobData* blob) {
  //std::cout << "Checking for intersecting blobs to current segmentation.\n";
  BlobMergeData* const mergeinfo = blob->getMergeData();
  assert(mergeinfo != NULL); // sanity
  GenericVector<BlobData*> intersecting;
  const int& segid = mergeinfo->getSegId();
  BlobDataGridSearch bdgs(blobDataGrid);
  bdgs.SetUniqueMode(true);
  TBOX* const segbox = mergeinfo->getSegBox();
  bdgs.StartRectSearch(*segbox);
  BlobData* n = NULL;
  while((n = bdgs.NextRectSearch()) != NULL) {
    if(n->getMergeData() == NULL
        && !intersecting.binary_search(n)) {
      intersecting.push_back(n);
      intersecting.sort();
    }
  }
  mergeinfo->intersecting = intersecting;
}

void HeuristicMerge::mergeOperation(BlobData* merge_from, BlobData* to_merge,
    BlobSpatial::Direction merge_dir) {
  assert(merge_dir == BlobSpatial::RIGHT || merge_dir == BlobSpatial::LEFT || merge_dir == BlobSpatial::UP
      || merge_dir == BlobSpatial::DOWN  || merge_dir == BlobSpatial::INTERSECT);

  BlobMergeData** merge_from_info = merge_from->getMergeDataSharedPtr();
  assert(to_merge->getMergeData() == NULL); // shouldn't have been merged yet

  // assign merged blob to the segment it's being merged with
  to_merge->setToExistingMergeData(merge_from_info);
  // expand the segment to accomodate the blob being merged if necessary
  TBOX* segbox = (*merge_from_info)->getSegBox();
  TBOX merged_box = to_merge->getBoundingBox();
  if(!segbox->contains(merged_box)) {
    if(merged_box.right() > segbox->right())
      segbox->move_right_edge(merged_box.right() - segbox->right());
    if(merged_box.top() > segbox->top())
      segbox->move_top_edge(merged_box.top() - segbox->top());
    if(merged_box.bottom() < segbox->bottom())
      segbox->move_bottom_edge(merged_box.bottom() - segbox->bottom());
    if(merged_box.left() < segbox->left())
      segbox->move_left_edge(merged_box.left() - segbox->left());
  }

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Finished merge operation. Showing the updated segmentation and what was merged.\n";
    std::cout << "Showing what is being merged in the "
         << ((merge_dir == BlobSpatial::UP) ? "upward" : (merge_dir == BlobSpatial::DOWN) ? "downward"
             : (merge_dir == BlobSpatial::RIGHT) ? "rightward" : (merge_dir == BlobSpatial::LEFT) ? "leftward"
                 : "intersect") << " direction.\n";
    M_Utils::dispBlobDataRegion(to_merge, dbgim);
    M_Utils::waitForInput();
    std::cout << "Showing the updated segmentation.\n";
    M_Utils::dispHlTBoxRegion(*(merge_from_info.segment_box), dbgim);
    M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
}

bool HeuristicMerge::isOperator(BlobData* blob) {
  if(blob->getCharRecognitionConfidence() < Utils::getCertaintyThresh() * 3) {
    // if it's a really low confidence then rule it out here
    return false;
  }
  TesseractCharData* parentChar = blob->getParentChar();
  if(parentChar == NULL) {
    return false;
  }
  const char* blobtxt = parentChar->getUnicode().c_str();
  if(!blobtxt) {
    return false;
  }
  if(Utils::stringCompare(blobtxt, ">") ||
     Utils::stringCompare(blobtxt, "<") ||
     Utils::stringCompare(blobtxt, "=") ||
     Utils::stringCompare(blobtxt, "+") ||
     Utils::stringCompare(blobtxt, "-")) {
    return true;
  }
  return false;
}

// returns true if the neighbor is already part of the blob's segmentation
bool HeuristicMerge::wasAlreadyMerged(BlobData* neighbor, BlobData* blob) {
  const BlobMergeData* blob_m = blob->getMergeData();
  assert(blob_m != NULL); // sanity
  const BlobMergeData* neighbor_m = neighbor->getMergeData();
  if(neighbor_m == NULL) {
    return false;
  }
  const int& blob_seg_id = blob_m->getSegId();
  const int& neighbor_seg_id = neighbor_m->getSegId();
  if(neighbor_seg_id == blob_seg_id)
    return true;
  return false;
}

void HeuristicMerge::setDbgImg(Pix* im) {
    dbgim = im;
  }

HeuristicMerge::~HeuristicMerge() {
}

std::vector<BlobData*> HeuristicMerge::getBlobsWithSameParent(BlobData* const blobData) {
  std::vector<BlobData*> blobsWithSameParent;
  if(blobData->getParentWord() == NULL) {
    return blobsWithSameParent;
  }
  std::vector<TesseractCharData*> wordChildChars = blobData->getParentWord()->getChildChars();
  for(int i = 0; i < wordChildChars.size(); ++i) {
    std::vector<BlobData*> charChildBlobs = wordChildChars[i]->getBlobs();
    for(int j = 0; j < charChildBlobs.size(); ++j) {
      blobsWithSameParent.push_back(charChildBlobs[j]);
    }
  }
  return blobsWithSameParent;
}

BlobFeatureExtractor* HeuristicMerge::getFeatureExtractor(const std::string& name) {
  BlobFeatureExtractor* blobsFeatureExtractor = NULL;
  std::vector<BlobFeatureExtractor*> blobFeatureExtractors =
      featureExtractor->getBlobFeatureExtractors();
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
    if(blobFeatureExtractors[i]->getFeatureExtractorDescription()->getName() == name) {
      blobsFeatureExtractor = blobFeatureExtractors[i];
      break;
    }
  }
  if(blobsFeatureExtractor == NULL) {
    std::cout << "ERROR: The Heuristic Merge algorithm needs to have the " << name << " feature extractor enabled in order to work.\n";
    assert(false);
    /*
    if(name == NumAlignedBlobsFeatureExtractorDescription::getName_()) {
      blobsFeatureExtractor =
          dynamic_cast<BlobFeatureExtractor*>(NumAlignedBlobsFeatureExtractorFactory().create(
              featureExtractor->getFinderInfo()));
    } else if(name == NumVerticallyStackedBlobsFeatureExtractorDescription::getName_()) {
      blobsFeatureExtractor =
          dynamic_cast<BlobFeatureExtractor*>(NumVerticallyStackedBlobsFeatureExtractorFactory().create(
              featureExtractor->getFinderInfo()));
    }
    */
  }
  return blobsFeatureExtractor;
}

GenericVector<BlobData*> HeuristicMerge::filterAlreadyMerged(
    const GenericVector<BlobData*> inputVector) {
  GenericVector<BlobData*> filteredVector;
  for(int i = 0; i < inputVector.size(); ++i) {
    if(inputVector[i]->getMergeData() == NULL) {
      filteredVector.push_back(inputVector[i]);
    }
  }
  return filteredVector;
}

bool HeuristicMerge::isHorizontal(BlobSpatial::Direction dir) {
  return dir == BlobSpatial::LEFT || dir == BlobSpatial::RIGHT;
}


