/*
 * NumAlignedBlobsFeatureExtractor.cpp
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#include <AlignedFeatExt.h>

#include <AlignedDesc.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <AlignedData.h>
#include <Direction.h>
#include <M_Utils.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>
#include <Utils.h>

#include <baseapi.h>

#include <assert.h>
#include <string>
#include <vector>

//#define DBG_COVER_FEATURE
//#define DBG_COVER_FEATURE_ALOT
//#define DBG_FEATURE
//#define DBG_DRAW_RIGHTWARD

NumAlignedBlobsFeatureExtractor::NumAlignedBlobsFeatureExtractor(NumAlignedBlobsFeatureExtractorDescription* description)
: rightwardFeatureEnabled(false),
  downwardFeatureEnabled(false),
  upwardFeatureEnabled(false),
  blobDataKey(-1), indbg(false),
  dbgdontcare(false),
  highCertaintyThresh(Utils::getCertaintyThresh() / 2),
  blobDataGrid(NULL) {
  this->description = description;
}

void NumAlignedBlobsFeatureExtractor::doPreprocessing(BlobDataGrid* const blobDataGrid) {
  // Get the key that will be used for retrieving data associated with
  // this class for each blob.
  blobDataKey = findOpenBlobDataIndex(blobDataGrid);
  this->blobDataGrid = blobDataGrid;

#ifdef DBG_DRAW_RIGHTWARD
  rightwardIm = pixCopy(NULL, blobDataGrid->getBinaryImage());
  rightwardIm = pixConvertTo32(rightwardIm);
#endif
  // Determine the adjacent covered neighbors in the rightward, downward,
  // and/or upward directions for each blob depending on which features are
  // enabled
  BlobDataGridSearch gridSearch(blobDataGrid);
  gridSearch.SetUniqueMode(true);
  gridSearch.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = gridSearch.NextFullSearch()) != NULL) {
    NumAlignedBlobsData* const data = new NumAlignedBlobsData(description);
    assert(blobDataKey == blob->appendNewVariableData(data));
    if(rightwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::RIGHT);
      data->setRhabcCount(count)
          ->setRhabcFeature(M_Utils::expNormalize(count));
    }
    if(upwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::UP);
      data->setUvabcCount(count)
          ->setUvabcFeature(M_Utils::expNormalize(count));
    }
    if(downwardFeatureEnabled) {
      const int count = countCoveredBlobs(blob, blobDataGrid, BlobSpatial::DOWN);
      data->setDvabcCount(count)
          ->setDvabcFeature(M_Utils::expNormalize(count));
    }
  }

#ifdef DBG_DRAW_RIGHTWARD
  pixDisplay(rightwardIm, 100, 100);
  std::cout << "Showing the blobs that have at least one rightward adjacent neighbor (feature) in red.\n";
  Utils::waitForInput();
  pixDestroy(&rightwardIm);
#endif
}

std::vector<DoubleFeature*> NumAlignedBlobsFeatureExtractor::extractFeatures(BlobData* const blob) {

  NumAlignedBlobsData* const data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));

#ifdef DBG_FEATURE
  double rhabc = (double)(data->getRhabcCount());
  double uvabc = (double)(data->getUvabcCount());
  double dvabc = (double)(data->getDvabcCount());
  if(rhabc > 0 && rightwardFeatureEnabled)
    std::cout << "Displayed blob has rhabc = " << rhabc << std::endl;
  if(uvabc > 0 && upwardFeatureEnabled)
    std::cout << "Displayed blob has uvabc = " << uvabc << std::endl;
  if(dvabc > 0 && downwardFeatureEnabled)
    std::cout << "Displayed blob has dvabc = " << dvabc << std::endl;
  if(rhabc > 0 || uvabc > 0 || dvabc > 0)
    M_Utils::dbgDisplayBlob(blob);
#endif

  return data->getExtractedFeatures();
}


int NumAlignedBlobsFeatureExtractor::countCoveredBlobs(BlobData* const blob,
    BlobDataGrid* const blobDataGrid, BlobSpatial::Direction dir, bool seg_mode,
    const int dbgSegId) {
//  if(dbgSegId == 0) {
//    indbg = true;
//  }
  this->blobDataGrid = blobDataGrid;
  BlobDataGridSearch bigs(blobDataGrid);
  bigs.SetUniqueMode(true);
  GenericVector<BlobData*> covered_blobs;
  GenericVector<BlobData*> noncovered_blobs;

  TBOX* segbox = NULL;

  TBOX* blob_box = NULL; // this could either be the blob itself or the blob's segmentation depending on
                  // whether seg_mode is turned off or on respectively.
  TBOX bbox = blob->getBoundingBox();
  if(seg_mode) {
    // !!!!!!Segmode should only be true when running the segmentor!!!!!
    assert(blob->getMergeData() != NULL);
    segbox = blob->getMergeData()->getSegBox();
    assert(segbox != NULL);

    blob_box = segbox;

  } else {
    blob_box = &bbox;
  }

  assert(blob_box != NULL);

  int count = 0;
#ifdef DBG_COVER_FEATURE
//  inT16 dbgleft = -1;
//  inT16 dbgtop = -1;
//  if(dbgleft < 0 || dbgtop < 0) // if i'm not debugging anything in particular debug everything
//    single_mode = false;
//
//  inT16 neighborleft = -1;
//  inT16 neighbortop = -1;
//
//  TBOX blobbox = blob->bounding_box();
//  if(blobbox.left() == dbgleft && blobbox.top() == dbgtop) {
//    std::cout << "found it!\n";
//    M_Utils::dispBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//    M_Utils::dispHlBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//    M_Utils::waitForInput();
//    indbg = true;
//  }
#endif
  // Very strict filter for segmentation mode to avoid false positives:
//?  if(seg_mode) {
//?    if((blob->belongsToRecognizedNormalRow() && blob->getAverageWordConfInRow() >= Utils::getCertaintyThresh())
//?    || blob->belongsToBadRegion()) {
//?      return 0;
//?    }
//    if(blob->belongsToRecognizedWord()) {
//      return 0;
//    }
//?  }
  // Below code and comment currently in question.... Should I do this?? I'm thinking not. All blobs should have this feature I think. Can't rely on Tesseract in face of math, so no telling if it's right or wrong.
  // ----------------COMMENT AND CODE IN QUESTION START---------------------
  // Some filters. First off I don't filter anything if I'm using this at the
  // heuristic merge stage (the seg_mode flag would be set to true). So with
  // that out of the way, I filter if the blob belongs to a word matching
  // an entry in Tesseract's dictionary and the word was recognized with a
  // high confidence. I also filter if the blob resides on a row that was
  // found to have a high likelihood of being part of a paragraph (i.e.,
  // 'normal' row of text).
  if(!seg_mode) {
    if((blob->belongsToRecognizedWord() &&
        blob->getWordRecognitionConfidence() > Utils::getCertaintyThresh())
        || blob->belongsToRecognizedNormalRow()
        || blob->getWordRecognitionConfidence() > highCertaintyThresh) {
      return 0;
    }
  }
  // ----------------COMMENT AND CODE IN QUESTION END---------------------
  int range;
  if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
    range = blob_box->height();
  }
  else if(dir == BlobSpatial::UP || dir == BlobSpatial::DOWN) {
    range = blob_box->width();
  }
  else {
    std::cout << "ERROR: countCoveredBlobs only "
         << "supports upward, downward, leftward, and rightward searches\n";
    assert(false);
  }

  // do beam searches to look for covered blobs
  int closeEnoughThreshold = dir == BlobSpatial::RIGHT || dir == BlobSpatial::UP ? INT_MAX : INT_MIN;
  for(int i = 0; i < range; ++i) {
    //if(indbg) {
      //std::cout << "top of for loop!\n";
    //}
    BlobData* n = NULL; // the neighbor
    // find nearest neighbor if looking horizontally
    if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
      bigs.StartSideSearch((dir == BlobSpatial::RIGHT) ? blob_box->right() : blob_box->left(),
          blob_box->bottom()+i, blob_box->bottom()+i+1);
      n = bigs.NextSideSearch((dir == BlobSpatial::RIGHT) ? false : true); // this starts with the rightmost/leftmost blob in the segmentation or the current blob if not in segmentation mode
    }
    // find nearest neighbor if looking vertically
    else {
      bigs.StartVerticalSearch(blob_box->left()+i, blob_box->left()+i+1,
          (dir == BlobSpatial::UP) ? blob_box->top() : blob_box->bottom());
      n = bigs.NextVerticalSearch((dir == BlobSpatial::UP) ? false : true);
    }
    if(n == NULL) {
      continue;
    }
    bool nothing = false;
    bool tooFarAway = false;
    BlobData* previous_neighbor_ptr = NULL;
    while(n->getBoundingBox() == *blob_box
        || ((dir == BlobSpatial::RIGHT) ? (((n->getBoundingBox().left() <= blob_box->right()) && n->left() < closeEnoughThreshold))
            : ((dir == BlobSpatial::LEFT) ? ((n->getBoundingBox().right() >= blob_box->left()) && n->right() > closeEnoughThreshold)
              : ((dir == BlobSpatial::UP) ? (n->getBoundingBox().bottom() <= blob_box->top()) && n->bottom() < closeEnoughThreshold
                : (n->getBoundingBox().top() >= blob_box->bottom()) && n->top() > closeEnoughThreshold)))
    ) {
      if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
        // Look horizontally
        n = bigs.NextSideSearch(dir == BlobSpatial::RIGHT ? false : true);
//        if(n != NULL && dir == BlobSpatial::RIGHT) {
//          if(seg_mode && blob_box->right() >= 2375 && !noncovered_blobs.bool_binary_search(n)) {
//            std::cout << "Looking at the shown blob as a covered candidate in the " <<
//                (dir == BlobSpatial::RIGHT ? "righward" : dir== BlobSpatial::LEFT ? "leftward" : "other") << " direction.\n";
//            M_Utils::dispHlBlobDataSegmentation(blob, blobDataGrid->getBinaryImage());
//            M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
//            M_Utils::waitForInput();
//            indbg = true;
//          } else {
//            indbg = false;
//          }
//        }
      }
      else {
        // Look vertically
        n = bigs.NextVerticalSearch(dir == BlobSpatial::UP ? false : true);

      }
      if(n == NULL) {
        nothing = true;
        break;
      }
    }
//    if(indbg) {
//      //std::cout << "Done while loop!!!!!\n";
//    }
    if(nothing) {
      continue;
    }
#ifdef DBG_COVER_FEATURE
    bool was_added = false;
//    if(indbg && !noncovered_blobs.bool_binary_search(n)) {
//      std::cout << "displaying element found " << ((dir == BlobSpatial::RIGHT) ? "to the right of "
//          : (dir == BlobSpatial::UP) ? "above " : "below ") << "the blob of interest\n";
//      M_Utils::dispHlBlobDataRegion(n, blobDataGrid->getBinaryImage());
//      M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
//      std::cout << "at i: " << i << std::endl;
//      std::cout << "point: " <<  ((dir == BlobSpatial::RIGHT) ? (blob->bottom() + i) :
//          (blob->left() + i)) << std::endl;
//      M_Utils::waitForInput();
//    }
#endif
    // if the neighbor is covered and isn't already on the list
    // then add it to the list (in ascending order)
    dbgdontcare = false;
    if(!covered_blobs.bool_binary_search(n) && n != blob && !noncovered_blobs.bool_binary_search(n)) {
      // Determine whether or not the neighbor is covered by the current bounding box
      // If in segmentation mode, then determine whether or not the neighbor is covered by the current blob's segmentation box
      bool tooFarAway = false;
      if(isNeighborCovered(n,
          blob,
          dir,
          seg_mode,
          &tooFarAway,
          dbgSegId)) {
#ifdef DBG_COVER_FEATURE
//        if(indbg && dir == BlobSpatial::DOWN) {
//          was_added = true;
//          std::cout << "the displayed element is being added to the covered list\n";
//          M_Utils::dispHlBlobDataRegion(n, blobDataGrid->getBinaryImage());
//          M_Utils::dispBlobDataRegion(n, blobDataGrid->getBinaryImage());
//          M_Utils::waitForInput();
//        }
#endif
        covered_blobs.push_back(n);
        covered_blobs.sort();
        ++count;
      } else if(tooFarAway) {
        closeEnoughThreshold = ((dir == BlobSpatial::RIGHT) ? n->left() :
            ((dir == BlobSpatial::LEFT) ? n->right() :
                ((dir == BlobSpatial::UP) ? n->bottom() :
                    n->top())));
      }
    }
    if(!noncovered_blobs.bool_binary_search(n)) {
      // if it's already been found to n turned on, only debugs a segment
      //                                   // of interest rather than thot be covered by the blob
      // then put it on this list so it won't get tested again
      noncovered_blobs.push_back(n);
      noncovered_blobs.sort();
    }
#ifdef DBG_COVER_FEATURE
//    if(indbg && !noncovered_blobs.bool_binary_search(n)) {
//      if(was_added) {
//
//      }
//      else if(!noncovered_blobs.bool_binary_search(n)){
//        std::cout << "the displayed element was not added to the covered list\n";
//        if(!dbgdontcare)
//          M_Utils::waitForInput();
//      }
//    }
#endif
  }
#ifdef DBG_COVER_FEATURE
//  if(count > 1 && indbg) {
//    std::cout << "the highlighted blob is the one being evaluated and has " << count
//         << " covered blobs " << "in the " << ((dir == BlobSpatial::RIGHT) ? " rightward "
//             : ((dir == BlobSpatial::UP) ? " upward " : " downward ")) << "direction\n";
//    M_Utils::dispHlBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//    M_Utils::dispBlobDataRegion(blob, blobDataGrid->getBinaryImage());
//    M_Utils::waitForInput();
#ifdef DBG_COVER_FEATURE_ALOT
    for(int j = 0; j < covered_blobs.length(); ++j) {
      cout << "the highlighted blob is covered by the blob previously shown\n";
      M_Utils::dispHlBlobDataRegion(covered_blobs[j], blobDataGrid->getBinaryImage());
      M_Utils::dispBlobDataRegion(covered_blobs[j], blobDataGrid->getBinaryImage());
      M_Utils::waitForInput();
    }
#endif
//  }
#endif
  NumAlignedBlobsData* const data = (NumAlignedBlobsData*)(blob->getVariableDataAt(blobDataKey));
  if(dir == BlobSpatial::UP)
    data->uvabc_blobs = covered_blobs;
  else if(dir == BlobSpatial::DOWN)
    data->dvabc_blobs = covered_blobs;
  else if(dir == BlobSpatial::RIGHT) {
    data->rhabc_blobs = covered_blobs;
#ifdef DBG_DRAW_RIGHTWARD
    if(covered_blobs.size() > 0) {
      M_Utils::drawHlBlobDataRegion(blob, rightwardIm, LayoutEval::RED);
    }
#endif
  } else if (dir == BlobSpatial::LEFT)
    data->lhabc_blobs = covered_blobs;
  else
    assert(false);
  return count;
}

/**
 * Determines whether or not a neighbor bounding box is "covered" by the current
 * bounding box based on distance vs size relationships between the two. A neighbor
 * bounding box is "covered" by the current bounding box in a given direction when
 * it resides entirely within the boundary of the current bounding box, is close
 * enough to the current object in the desired direction, is not too small in
 * relation to the current object, and has a size greater than or equal to half
 * of the its distance from the current object (where size is the greater of width
 * or height). If seg_mode is true then the current blob's bounding box is assumed
 * to correspond with that blob's segmentation results and some of the paramters are
 * slightly tweaked.
 */
bool NumAlignedBlobsFeatureExtractor::isNeighborCovered(BlobData* const neighborBlob,
    BlobData* const curBlob, const BlobSpatial::Direction& dir,
    const bool seg_mode, bool* tooFarAway, const int dbgSegId) {
  bool dbgNeighbor = false;
//  if(seg_mode && indbg) {
//    TBOX roi(2004, 4790, 2027, 4846);
//    BlobSpatial::Direction doi = BlobSpatial::RIGHT;
//    const int soi = 0;
//    if(neighborBlob->bounding_box() == roi
//      && dir == doi && dbgSegId == soi) {
//      std::cout << "Found the neighbor of interest in the direction of interest for the segment of interest... displaying.\n";
//      M_Utils::dispHlBlobDataRegion(neighborBlob, blobDataGrid->getBinaryImage());
//      M_Utils::dispBlobDataRegion(neighborBlob, blobDataGrid->getBinaryImage());
//      M_Utils::waitForInput();
//      std::cout << "Showing what its a merge candidate for....\n";
//      M_Utils::dispHlBlobDataSegmentation(curBlob, blobDataGrid->getBinaryImage());
//      M_Utils::waitForInput();
//      dbgNeighbor = true;
//    } else {
//      dbgNeighbor = false;
//    }
//  }

  // For segmentation mode do some filtering to avoid excessive merging (and false positives)...
  if(seg_mode) {
    if((neighborBlob->belongsToRecognizedNormalRow() && neighborBlob->getAverageWordConfInRow() >= Utils::getCertaintyThresh())
    || neighborBlob->belongsToBadRegion()) {
      if(indbg) {
        std::cout << "Neighbor not covered either because it belongs to word with too high confidence, or belongs to a bad region (or both).\n";
        if(dbgNeighbor) {
          M_Utils::waitForInput();
        }
      }
      return false;
    }
//    if(neighborBlob->getParentChar() == NULL &&
//        (dir == BlobSpatial::UP || dir == BlobSpatial::DOWN)) {
//      return false;
//    }
//    if(neighborBlob->getParentChar() != NULL
//        && curBlob->getParentChar() == NULL) {
//      return false;
//    }
//    if(indbg) {
//      std::cout << "The displayed blob and its candidate neighbor passed the filters....\n";
//      M_Utils::dispHlBlobDataRegion(curBlob, curBlob->getParentGrid()->getBinaryImage());
//      M_Utils::dispHlBlobDataRegion(neighborBlob, curBlob->getParentGrid()->getBinaryImage());
//      M_Utils::waitForInput();
//    }
  }

  // ----------------COMMENT AND CODE IN QUESTION END---------------------
  const TBOX& blob_box = !seg_mode ? curBlob->getBoundingBox() : *(curBlob->getMergeData()->getSegBox());
  const TBOX& neighbor = neighborBlob->getBoundingBox();
  inT16 neighbor_dist;
  inT16 blob_upper;
  inT16 blob_lower;
  inT16 neighbor_center;
  double dist_thresh_param = (double)2;
  if(seg_mode) {
    if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
      dist_thresh_param = (double)1; // looser restriction for horizontal
    } else {
      if(curBlob->belongsToRecognizedNormalRow() || neighborBlob->belongsToRecognizedNormalRow()) {
        return false; // tighter restriction for vertical
      }
      dist_thresh_param = (double)4; // tighter restriction for vertical
    }
  }
  double dist_threshold;
  double area_thresh;
  double dist_to_size_thresh = (double)2;
  if(dir == BlobSpatial::RIGHT || dir == BlobSpatial::LEFT) {
    blob_lower = blob_box.bottom();
    blob_upper = blob_box.top();
    dist_threshold = std::min(((double)blob_box.height() / dist_thresh_param), 75.0);
    neighbor_dist = (dir == BlobSpatial::RIGHT) ? neighbor.left() - blob_box.right()
        : blob_box.left() - neighbor.right();
    if(!seg_mode) {
      area_thresh = (double)(blob_box.height()) / (double)32;
    } else {
      area_thresh = (double)(blob_box.height()) / (double)64; // looser restriction for horizontal in segmentation mode
      dist_to_size_thresh = (double)8;
    }
    neighbor_center = M_Utils::centery(neighbor);
  }
  else if(dir == BlobSpatial::UP || dir == BlobSpatial::DOWN) {
    blob_lower = blob_box.left();
    blob_upper = blob_box.right();
    dist_threshold = std::min(((double)(blob_box.width()) / dist_thresh_param), 25.0); // TODO: tweak for seg_mode!!!
    if(dir == BlobSpatial::UP)
      neighbor_dist = neighbor.bottom() - blob_box.top();
    else
      neighbor_dist = blob_box.bottom() - neighbor.top();
    area_thresh = (double)(blob_box.width()) / (double)32;
    neighbor_center = M_Utils::centerx(neighbor);
  }
  else {
    std::cout << "ERROR: isNeighborCovered only supports RIGHT, UP, and DOWN directions\n";
    assert(false);
  }
  // is the neighbor adjacent?
  if(neighbor_dist <= dist_threshold) {
  // is the neighbor covered?
  if(neighbor_center >= blob_lower && neighbor_center <= blob_upper) {
      const double area = (double)(neighbor.area());
      if(area > area_thresh) {
        double blobsize = (neighbor.height() >= neighbor.width())
            ? (double)(neighbor.height()) : (double)(neighbor.width());
        double dist_to_size_ratio = (double)neighbor_dist /
            blobsize;
        if(dist_to_size_ratio < dist_to_size_thresh) {
          return true;
        }
#ifdef DBG_COVER_FEATURE
        else {
          if(indbg)
            std::cout << "The neighbor is not covered because it's distance from the "
                 << "blob of interest is too great in relation to it's size.\n";
          if(dbgNeighbor) M_Utils::waitForInput();
        }
#endif
      }
#ifdef DBG_COVER_FEATURE
      else {
        if(indbg) {
          std::cout << "The neighbor is not covered because it is too small\n";
          if(dbgNeighbor) M_Utils::waitForInput();
        }
      }
#endif
  }
#ifdef DBG_COVER_FEATURE
  else {
    if(indbg) {
      std::cout << "The neighbor is not covered because its center is outside the boundary\n";
      if(dbgNeighbor) M_Utils::waitForInput();
    }
  }
#endif
  } else {
    *tooFarAway = true;
#ifdef DBG_COVER_FEATURE
    if(indbg) {
      std::cout << "The neighbor is not covered because its distance is above the threshold.\n";
      if(dbgNeighbor) {
        std::cout << "Neighbor distance: " << neighbor_dist << std::endl;
        std::cout << "Distance threshold: " << dist_threshold << std::endl;
        M_Utils::waitForInput();
      }
      dbgdontcare = true;
    }
#endif
  }
  return false;
}

void NumAlignedBlobsFeatureExtractor::enableRightwardFeature() {
  enabledFlagDescriptions.push_back(description->getRightwardFlagDescription());
  rightwardFeatureEnabled = true;
}
void NumAlignedBlobsFeatureExtractor::enableDownwardFeature() {
  enabledFlagDescriptions.push_back(description->getDownwardFlagDescription());
  downwardFeatureEnabled = true;
}

void NumAlignedBlobsFeatureExtractor::enableUpwardFeature() {
  enabledFlagDescriptions.push_back(description->getUpwardFlagDescription());
  upwardFeatureEnabled = true;
}

BlobFeatureExtractorDescription* NumAlignedBlobsFeatureExtractor::getFeatureExtractorDescription() {
  return description;
}

std::vector<FeatureExtractorFlagDescription*> NumAlignedBlobsFeatureExtractor::getEnabledFlagDescriptions() {
  return enabledFlagDescriptions;
}

void NumAlignedBlobsFeatureExtractor::doSegmentationInit(BlobDataGrid* blobDataGrid) {
  if(blobDataKey < 0) {
    blobDataKey = findOpenBlobDataIndex(blobDataGrid);
  }
}

NumAlignedBlobsData* NumAlignedBlobsFeatureExtractor::getBlobFeatureData(BlobData* const blobData) {
  if(blobDataKey < 0) {
    return NULL;
  }
  return (NumAlignedBlobsData*)(blobData->getVariableDataAt(blobDataKey));
}

