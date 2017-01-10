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

#include <baseapi.h>

#include <allheaders.h>

#include <vector>
#include <iostream>
#include <stddef.h>
#include <assert.h>

#define DBG_SHOW_MERGE_START  // shows the starting blob for a merge
#define DBG_SHOW_MERGE // shows all the merges that happen between DBG_SHOW_MERGE_START and
                       // DBG_SHOW_MERGE_FINAL
#define DBG_SHOW_MERGE_ONE_SEGMENT // requires DBG_SHOW_MERGE, DBG_SHOW_MERGE_FINAL, and/or
                                   // DBG_SHOW_MERGE_START to be turned on, only debugs a segment
                                   // of interest rather than the entire page
#define DBG_MERGE_VERBOSE // show everything that is being added to the merge list
#define DBG_MERGE_INTERSECTING // show all the blobs that were merged because they intersect with a segment
#define DBG_H_ADJACENT
#define DBG_SHOW_MERGE_FINAL // shows the result of merging

#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
int g_dbg_x = 1786; // define the left x coord of roi
int g_dbg_y = 1506; // define the top y coord of roi
bool g_dbg_flag = true;
#endif

HeuristicMerge::HeuristicMerge(MathExpressionFeatureExtractor* const featureExtractor)
: dbgim(NULL) {
  this->featureExtractor = featureExtractor;
  this->numAlignedBlobsFeatureExtractor = dynamic_cast<NumAlignedBlobsFeatureExtractor*>(getFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription::getName_()));
  this->numVerticallyStackedFeatureExtractor = dynamic_cast<NumVerticallyStackedBlobsFeatureExtractor*>(getFeatureExtractor(NumVerticallyStackedBlobsFeatureExtractorDescription::getName_()));
}

MathExpressionFinderResults* HeuristicMerge::runSegmentation(BlobDataGrid* const blobDataGrid) {
  // now do the segmentation step
  dbgim = blobDataGrid->getImage(); // allows for optional debugging

  numAlignedBlobsFeatureExtractor->doSegmentationInit(blobDataGrid);
  numVerticallyStackedFeatureExtractor->doSegmentationInit(blobDataGrid);

  this->blobDataGrid = blobDataGrid;

  BlobDataGridSearch bdgs(blobDataGrid);

  // pass 1: does some post-processing to get rid of blobs sparsely detected within recognized words
  // marked as valid by Tesseract with high confidence. also gets rid of any blob detected as math
  // that's within a stop word recognized with high confidence by Tesseract.
  bdgs.StartFullSearch();
  BlobData* curblob = NULL;
  double sparseness_threshold = .6; // minimum acceptable ratio of math blobs to total blobs in a recognized as valid non-math word
  while((curblob = bdgs.NextFullSearch()) != NULL) {
    if(curblob->getParentWord() == NULL) {
      continue; // all of the below filters assume blob was recognized as part of some word
    }
    if(curblob->getMathExpressionDetectionResult()) {
      if(curblob->belongsToRecognizedWord() && (!curblob->belongsToRecognizedMathWord())) {
        int num_math_in_word = 0;
        if(curblob->belongsToRecognizedStopword()) {
          curblob->setMathExpressionDetectionResult(false);
          continue;
        }
        std::vector<BlobData*> wordChildBlobs = getBlobsWithSameParent(curblob);
        int numwrdblobs = wordChildBlobs.size();
        for(int i = 0; i < numwrdblobs; ++i) {
          BlobData* const wrdblob = wordChildBlobs[i];
          if(wrdblob->getMathExpressionDetectionResult()) {
            ++num_math_in_word;
          }
        }
        double math_non_math_wrd_ratio = (double)num_math_in_word / (double)numwrdblobs;
        if(math_non_math_wrd_ratio < sparseness_threshold) {
          curblob->setMathExpressionDetectionResult(false);
        }
      }
    }
  }

  // pass 2: run the segmentation algorithm
  bdgs.StartFullSearch();
  curblob = NULL;
  int seg_id = 0;
  while((curblob = bdgs.NextFullSearch()) != NULL) {
    if(!curblob->getMathExpressionDetectionResult() || curblob->getMergeData().is_processed)
      continue;
#ifdef DBG_SHOW_MERGE_START
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
    if(curblob->getBoundingBox().left() == g_dbg_x && curblob->getBoundingBox().top() == (dbgim->h - g_dbg_y))
      g_dbg_flag = true;
    else
      g_dbg_flag = false;
    if(g_dbg_flag) {
#endif
      std::cout << "About the start merging process from the displayed blob:\n";
      M_Utils::dispHlBlobDataRegion(curblob, dbgim);
      M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
    }
#endif
#endif
    decideAndMerge(curblob, seg_id); // recursively merges a blob to its neighbors to create a segmentation
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

  return MathExpressionFinderResultsBuilder()
      .setResults(blobDataGrid->getSegments())
      ->setVisualResultsDisplay(getVisualResultsDisplay())
      ->setResultsName(blobDataGrid->getImageName())
      ->setResultsDirName(featureExtractor->getFinderInfo()->getFinderName())
      ->build();
}

// make the merge decision for left, right, up, and down
// carry out the merge operation(s) for left, right, up, or down if applicable
void HeuristicMerge::decideAndMerge(BlobData* blob,
    const int& seg_id) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Starting the decideAndMerge with the displayed blob:\n";
    M_Utils::dispBlobDataRegion(blob, dbgim);
    M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
  BlobMergeData& blob_merge_info = blob->getMergeData();
  if(blob_merge_info.seg_id > -1 && blob_merge_info.seg_id != seg_id)
    return; // this blob was already added to a different segment
  bool was_already_processed = blob_merge_info.is_processed;
  blob_merge_info.is_processed = true; // go ahead and flag as processed to prevent endless recursion
  blob_merge_info.seg_id = seg_id;
  // initialize the blob's segmentation if it hasn't been initialized yet
  TBOX*& blob_segment = blob_merge_info.segment_box;
  // if it's not part of an existing segment make a new one for it
  if(!blob_segment) {
    blob_segment = new TBOX(
        blob->getBoundingBox().left(),
        blob->getBoundingBox().bottom(),
        blob->getBoundingBox().right(),
        blob->getBoundingBox().top());
    RESULT_TYPE res;
    if(blob->belongsToRecognizedNormalRow())
      res = EMBEDDED;
    else
      res = DISPLAYED;
    Segmentation* seg = new Segmentation;
    seg->box = blob_segment;
    seg->res = res;
    blobDataGrid->getSegments().push_back(seg); // the allocated memory is owned by the grid
  }
  if(was_already_processed) {
    assert(blob_merge_info.seg_id > -1);
    if(blob_merge_info.seg_id == seg_id) {
      assert(blob_merge_info.processed_seg_area <= blob_segment->area());
      if(blob_merge_info.processed_seg_area == blob_segment->area())
        return;
    }
    else
      return;
  }
  blob_merge_info.processed_seg_area = blob_segment->area();

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

  // carry out the merge operation on the applicable directions
  GenericVector<BlobData*> all_merges;
  const GenericVector<BlobData*>& mergedown = blob_merge_info.down;
  for(int i = 0; i < mergedown.length(); ++i) {
    if(mergedown[i]->getMergeData().seg_id == -1)
      mergeOperation(blob, mergedown[i], BlobSpatial::DOWN);
    all_merges.push_back(mergedown[i]);
  }
  const GenericVector<BlobData*>& mergeup = blob_merge_info.up;
  for(int i = 0; i < mergeup.length(); ++i) {
    if(mergeup[i]->getMergeData().seg_id == -1)
      mergeOperation(blob, mergeup[i], BlobSpatial::UP);
    all_merges.push_back(mergeup[i]);
  }
  const GenericVector<BlobData*>& mergeright = blob_merge_info.right;
  for(int i = 0; i < mergeright.length(); ++i) {
    if(mergeright[i]->getMergeData().seg_id == -1)
      mergeOperation(blob, mergeright[i], BlobSpatial::RIGHT);
    all_merges.push_back(mergeright[i]);
  }
  const GenericVector<BlobData*>& mergeleft = blob_merge_info.left;
  for(int i = 0; i < mergeleft.length(); ++i) {
    if(mergeleft[i]->getMergeData().seg_id == -1)
      mergeOperation(blob, mergeleft[i], BlobSpatial::LEFT);
    all_merges.push_back(mergeleft[i]);
  }
  const GenericVector<BlobData*>& intersecting = blob_merge_info.intersecting;
  for(int i = 0; i < intersecting.length(); ++i) {
    if(intersecting[i]->getMergeData().seg_id == -1)
      mergeOperation(blob, intersecting[i], BlobSpatial::INTERSECT);
    all_merges.push_back(intersecting[i]);
  }
  // recursively repeat for each merged region
  for(int i = 0; i < all_merges.length(); ++i)
    decideAndMerge(all_merges[i], seg_id);
}

void HeuristicMerge::mergeDecision(BlobData* blob, BlobSpatial::Direction dir) {
  assert(dir == BlobSpatial::LEFT || dir == BlobSpatial::RIGHT
      || dir == BlobSpatial::UP || dir == BlobSpatial::DOWN);
  BlobMergeData& merge_info = blob->getMergeData();

  // Get the blob's data associated with the feature extractor
  NumAlignedBlobsData* numAlignedBlobsData = numAlignedBlobsFeatureExtractor->getBlobFeatureData(blob);

  GenericVector<BlobData*> covered_merges; // stores blob merged by the "cover feature"
  GenericVector<BlobData*> stacked_merges;
  BlobData* h_adjacent = NULL;


  // horizontal merge decision
  if(dir == BlobSpatial::LEFT || dir == BlobSpatial::RIGHT) {
    int count = numAlignedBlobsFeatureExtractor->countCoveredBlobs(blob, blobDataGrid, dir, true);
    assert(covered_merges.empty());
    if(dir == BlobSpatial::LEFT)
      covered_merges = numAlignedBlobsData->lhabc_blobs;
    else
      covered_merges = numAlignedBlobsData->rhabc_blobs;
    // is there something to the left or right that's not on the covered list
    // but that is still adjacent?
    TBOX* segbox = merge_info.segment_box;
    BlobDataGridSearch bdgs(blobDataGrid);
    //const inT16 segbox_center_y = segbox->bottom() + (segbox->height() / 2);
    bdgs.StartSideSearch((dir == BlobSpatial::RIGHT) ? segbox->right() : segbox->left(),
        segbox->bottom(), segbox->top());
    BlobData* n = bdgs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true);
    while((n->getBoundingBox() == blob->getBoundingBox()) ||
        ((dir == BlobSpatial::RIGHT) ? (n->getBoundingBox().left() <= segbox->right())
            : (n->getBoundingBox().right() >= segbox->left()))
              || (n->getBoundingBox().top() < segbox->bottom() || n->getBoundingBox().bottom() > segbox->top())) {
      n = bdgs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true);
      if(n == NULL)
        break;
    }
    if(n != NULL) {
      if(NumVerticallyStackedBlobsFeatureExtractor::isAdjacent(n, blob, dir, merge_info.segment_box))
        h_adjacent = n;
    }
    // if there weren't any covered or horizontally adjacent blobs found yet
    // see if there's something directly to the right or left of the current
    // blob that may constitute an operator or operand depending upon the situation
    if(h_adjacent == NULL && covered_merges.empty()) {
      bdgs.StartSideSearch((dir == BlobSpatial::RIGHT) ? blob->getBoundingBox().right() : blob->getBoundingBox().left(),
          blob->getBoundingBox().bottom(), blob->getBoundingBox().top());
      n = bdgs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true);
      while((n->getBoundingBox() == blob->getBoundingBox()) ||
          ((dir == BlobSpatial::RIGHT) ? (n->getBoundingBox().left() <= blob->getBoundingBox().right())
              : (n->getBoundingBox().right() >= blob->getBoundingBox().left()))
                || (n->getBoundingBox().top() < blob->getBoundingBox().bottom() || n->getBoundingBox().bottom() > blob->getBoundingBox().top())) {
        n = bdgs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true);
        if(n == NULL)
          break;
        // has to be DIRECTLY to the right or left of the current blob
        // in order to be of significance, otherwise just move on
        if((!(n->getBoundingBox() == blob->getBoundingBox()))
            && wasAlreadyMerged(n, blob)) {
          n = NULL;
          break;
        }
      }
      // is the current symbol an operator? if so whatever was found in the given
      // direction should be an operand!
      if(isOperator(blob)) {
        if(n != NULL) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
          if(g_dbg_flag) {
#endif
            std::cout << "The horizontally adjacent blob is an operand.\n";
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
          }
#endif
#endif
          h_adjacent = n;
        }
      }
      else { // if whatever was found is an operator then this is likely an operand to it!
        if(n != NULL) {
          if(isOperator(n)) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
          if(g_dbg_flag) {
#endif
            std::cout << "The horizontally adjacent blob is an operator.\n";
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
          }
#endif
#endif
            h_adjacent = n;
          }
        }
      }
    }
  }
  else { // vertical merge decision
    // using cover feature (on the segment rather than on the blob though!)
    int count = numAlignedBlobsFeatureExtractor->countCoveredBlobs(blob, blobDataGrid, dir, true);
    assert(covered_merges.empty());
    if(dir == BlobSpatial::DOWN)
      covered_merges = numAlignedBlobsData->dvabc_blobs;
    else
      covered_merges = numAlignedBlobsData->uvabc_blobs;

    // using stack feature
    stacked_merges =
        numVerticallyStackedFeatureExtractor
        ->getBlobFeatureData(blob)->getStackedBlobs();
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
      M_Utils::waitForInput();
    }
#endif
    std::cout << "Found " << stacked_merges.length() << " " <<
        ((dir == BlobSpatial::UP) ? "upward" : (dir == BlobSpatial::DOWN) ? "downward"
            : (dir == BlobSpatial::RIGHT) ? "rightward" : "leftward") << " stacked merges\n";
#ifdef DBG_MERGE_VERBOSE
    for(int i = 0; i < stacked_merges.length(); ++i) {
      std::cout << "Displaying stacked merge " << i << std::endl;
      M_Utils::dispBlobDataRegion(stacked_merges[i], dbgim);
      M_Utils::waitForInput();
    }
#endif
    if(h_adjacent != NULL) {
      std::cout << "Found a " << ((dir == BlobSpatial::RIGHT) ? "rightward" : "leftward") << " horizontal merge\n";
#ifdef DBG_MERGE_VERBOSE
      std::cout << "Displaying the horizontal merge:\n";
      M_Utils::dispBlobDataRegion(h_adjacent, dbgim);
      M_Utils::waitForInput();
#endif
    }
    else
      std::cout << "Found 0 " << ((dir == BlobSpatial::RIGHT) ? "rightward" : "leftward") << " horizontal merges\n";
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif

  // add all the blobs to be merged in the given direction onto the mergeinfo for this blob
  GenericVector<BlobData*>& merge_list = (dir == BlobSpatial::UP) ? merge_info.up :
      (dir == BlobSpatial::DOWN) ? merge_info.down : (dir == BlobSpatial::RIGHT) ? merge_info.right :
          merge_info.left;
  for(int i = 0; i < covered_merges.length(); ++i) {
    const BlobMergeData& covered_mergeinfo = covered_merges[i]->getMergeData();
    if(covered_mergeinfo.seg_id > -1)
      continue; // the covered blob is part of a different segment or was already
                // merged to this one so should not be merged again
    merge_list.push_back(covered_merges[i]);
  }
  for(int i = 0; i < stacked_merges.length(); ++i) {
    const BlobMergeData& stacked_mergeinfo = stacked_merges[i]->getMergeData();
    if(stacked_mergeinfo.seg_id > -1)
      continue; // the covered blob is part of a different segment or was already
                // merged to this one so should not be merged again
    merge_list.push_back(stacked_merges[i]);
  }
  if(h_adjacent != NULL) {
    const BlobMergeData& h_adj_mergeinfo = h_adjacent->getMergeData();
    if(h_adj_mergeinfo.seg_id == -1)
      merge_list.push_back(h_adjacent);
  }
}

void HeuristicMerge::checkIntersecting(BlobData* blob) {
  BlobMergeData& mergeinfo = blob->getMergeData();
  GenericVector<BlobData*> intersecting;
  TBOX* segbox = mergeinfo.segment_box;
  const int& segid = mergeinfo.seg_id;
  BlobDataGridSearch bdgs(blobDataGrid);
  bdgs.StartRectSearch(*segbox);
  BlobData* n = NULL;
  while((n = bdgs.NextRectSearch()) != NULL) {
    const BlobMergeData& neighbor_mergeinfo = n->getMergeData();
    const int& n_segid = neighbor_mergeinfo.seg_id;
    if(((n_segid == -1) && (n_segid != segid))
        && !intersecting.binary_search(n)) {
      intersecting.push_back(n);
      intersecting.sort();
    }
  }
  mergeinfo.intersecting = intersecting;
}

void HeuristicMerge::mergeOperation(BlobData* merge_from, BlobData* to_merge,
    BlobSpatial::Direction merge_dir) {
  assert(merge_dir == BlobSpatial::RIGHT || merge_dir == BlobSpatial::LEFT || merge_dir == BlobSpatial::UP
      || merge_dir == BlobSpatial::DOWN  || merge_dir == BlobSpatial::INTERSECT);
  BlobMergeData& merge_from_info = merge_from->getMergeData();
  BlobMergeData& to_merge_info = to_merge->getMergeData();
  assert(to_merge_info.seg_id == -1); // shouldn't have been merged yet
  BlobSpatial::Direction opposite_dir = (merge_dir == BlobSpatial::UP) ? BlobSpatial::DOWN : (merge_dir == BlobSpatial::DOWN) ? BlobSpatial::UP
      : (merge_dir == BlobSpatial::RIGHT) ? BlobSpatial::LEFT : (merge_dir == BlobSpatial::LEFT) ? BlobSpatial::RIGHT : BlobSpatial::INTERSECT;
  // logically connect the blob being merged so they are connected both ways
  if(opposite_dir == BlobSpatial::UP)
    to_merge_info.up.push_back(merge_from);
  else if(opposite_dir == BlobSpatial::DOWN)
    to_merge_info.down.push_back(merge_from);
  else if(opposite_dir == BlobSpatial::LEFT)
    to_merge_info.left.push_back(merge_from);
  else if(opposite_dir == BlobSpatial::RIGHT)
    to_merge_info.right.push_back(merge_from);
  else
    to_merge_info.intersecting.push_back(merge_from);
  // assign merged blob to the segment it's being merged with
  TBOX*& segbox = merge_from_info.segment_box;
  to_merge_info.segment_box = segbox;
  // expand the segment to accomodate the blob being merged if necessary
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
  to_merge_info.seg_id = merge_from_info.seg_id;
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    std::cout << "Finished merge operation. Showing the updated segmented and what was merged.\n";
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
  TesseractWordData* const word = blob->getParentWord();
  if(word == NULL) {
    return false;
  }
  const char* blobtxt = word->wordstr();
  if(!blobtxt)
    return false;
  if(Utils::stringCompare(blobtxt, ">") ||
     Utils::stringCompare(blobtxt, "<") ||
     Utils::stringCompare(blobtxt, "=") ||
     Utils::stringCompare(blobtxt, "+") ||
     Utils::stringCompare(blobtxt, "-"))
    return true;
  return false;
}

// returns true if the neighbor is already part of the blob's segmentation
bool HeuristicMerge::wasAlreadyMerged(BlobData* neighbor, BlobData* blob) {
  const BlobMergeData& blob_m = blob->getMergeData();
  const BlobMergeData& neighbor_m = neighbor->getMergeData();
  const int& blob_seg_id = blob_m.seg_id;
  const int& neighbor_seg_id = neighbor_m.seg_id;
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

Pix* HeuristicMerge::getVisualResultsDisplay() {

  Pix* dbgimg = pixCopy(NULL, blobDataGrid->getBinaryImage());
  dbgimg = pixConvertTo32(dbgimg);

  GenericVector<Segmentation*> segments = blobDataGrid->getSegments();
  for(int i = 0; i < segments.length(); ++i ) {
    const Segmentation* seg = segments[i];
    BOX* bbox = M_Utils::tessTBoxToImBox(seg->box, blobDataGrid->getBinaryImage());
    const RESULT_TYPE& restype = seg->res;
    M_Utils::drawHlBoxRegion(bbox, dbgimg, ((restype == DISPLAYED) ?
        LayoutEval::RED : (restype == EMBEDDED) ? LayoutEval::BLUE
            : LayoutEval::GREEN));
    boxDestroy(&bbox);
  }

  return dbgimg;
}

