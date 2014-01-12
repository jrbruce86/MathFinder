/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   Segmentor1.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 27, 2013 1:29:14 AM
 * ------------------------------------------------------------------------
 * Description: First segmentor implementation
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
#include "Segmentor1.h"


//#define DBG_SHOW_MERGE_START  // shows the starting blob for a merge
//#define DBG_SHOW_MERGE // shows all the merges that happen between DBG_SHOW_MERGE_START and
                       // DBG_SHOW_MERGE_FINAL
//#define DBG_SHOW_MERGE_ONE_SEGMENT // requires DBG_SHOW_MERGE, DBG_SHOW_MERGE_FINAL, and/or
                                   // DBG_SHOW_MERGE_START to be turned on, only debugs a segment
                                   // of interest rather than the entire page
//#define DBG_MERGE_VERBOSE // show everything that is being added to the merge list
//#define DBG_MERGE_INTERSECTING // show all the blobs that were merged because they intersect with a segment
//#define DBG_H_ADJACENT
//#define DBG_SHOW_MERGE_FINAL // shows the result of merging

#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
int g_dbg_x = 1786; // define the left x coord of roi
int g_dbg_y = 1506; // define the top y coord of roi
bool g_dbg_flag = false;
#endif

Segmentor1::Segmentor1() {}

BlobInfoGrid* Segmentor1::runSegmentation(BlobInfoGrid* grid_) {
  grid = grid_;
  BlobInfoGridSearch bigs(grid);

  // pass 1: does some post-processing to get rid of blobs sparsely detected within valid
  // non math words. also gets rid of any blob detected as math that's within a stop word.
  bigs.StartFullSearch();
  BLOBINFO* curblob = NULL;
  double sparseness_threshold = .6; // minimum acceptable ratio of math blobs to total blobs in a valid, non-math word
  while((curblob = bigs.NextFullSearch()) != NULL) {
    if(curblob->predicted_math) {
      if(curblob->validword && (!curblob->ismathword)) {
        int num_math_in_word = 0;
        if(curblob->isstopword) {
          curblob->predicted_math = false;
          continue;
        }
        int numwrdblobs = curblob->wordinfo->blobs.length();
        for(int i = 0; i < numwrdblobs; ++i) {
          BLOBINFO* wrdblob = curblob->wordinfo->blobs[i];
          if(wrdblob->predicted_math)
            ++num_math_in_word;
        }
        double math_non_math_wrd_ratio = (double)num_math_in_word / (double)numwrdblobs;
        if(math_non_math_wrd_ratio < sparseness_threshold) {
          curblob->predicted_math = false;
        }
      }
    }
  }

  // pass 2: run the segmentation algorithm
  bigs.StartFullSearch();
  curblob = NULL;
  int seg_id = 0;
  while((curblob = bigs.NextFullSearch()) != NULL) {
    if(!curblob->predicted_math || curblob->merge.is_processed)
      continue;
#ifdef DBG_SHOW_MERGE_START
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
    if(curblob->left() == g_dbg_x && curblob->top() == (dbgim->h - g_dbg_y))
      g_dbg_flag = true;
    else
      g_dbg_flag = false;
    if(g_dbg_flag) {
#endif
      cout << "About the start merging process from the displayed blob:\n";
      M_Utils::dispHlBlobInfoRegion(curblob, dbgim);
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
      cout << "completed merge!\n";
      cout << "Displaying the finalized segment!\n";
      cout << "This is the segment with id: " << curblob->merge.seg_id << endl;
      TBOX* seg_tbox = curblob->merge.segment_box;
      M_Utils::dispHlTBoxRegion(*seg_tbox, dbgim);
      M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
    }
#endif
#endif
    ++seg_id;
  }

  return grid;
}



// make the merge decision for left, right, up, and down
// carry out the merge operation(s) for left, right, up, or down if applicable
void Segmentor1::decideAndMerge(BLOBINFO* blob, const int& seg_id) {
#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    cout << "Starting the decideAndMerge with the displayed blob:\n";
    M_Utils::dispBlobInfoRegion(blob, dbgim);
    M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
  MergeInfo& blob_merge_info = blob->merge;
  if(blob_merge_info.seg_id > -1 && blob_merge_info.seg_id != seg_id)
    return; // this blob was already added to a different segment
  bool was_already_processed = blob_merge_info.is_processed;
  blob_merge_info.is_processed = true; // go ahead and flag as processed to prevent endless recursion
  blob_merge_info.seg_id = seg_id;
  // initialize the blob's segmentation if it hasn't been initialized yet
  TBOX*& blob_segment = blob_merge_info.segment_box;
  // if it's not part of an existing segment make a new one for it
  if(!blob_segment) {
    blob_segment = new TBOX(blob->left(), blob->bottom(), blob->right(), blob->top());
    RESULT_TYPE res;
    if(blob->onRowNormal())
      res = EMBEDDED;
    else
      res = DISPLAYED;
    Segmentation* seg = new Segmentation;
    seg->box = blob_segment;
    seg->res = res;
    grid->Segments.push_back(seg); // the allocated memory is owned by the grid, blob just has pointer
                                   // (albiet a danlging one, but the pointer is not to be
                                   // changed until destruction of the grid)
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
  mergeDecision(blob, LEFT);
  mergeDecision(blob, RIGHT);
  mergeDecision(blob, UP);
  mergeDecision(blob, DOWN);
  checkIntersecting(blob); // see if there's an unprocessed blob that intersects current segment

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    cout << "Found " << blob->merge.intersecting.length() << " blobs that intersect with the "
        << "current segmentation.\n";
#ifdef DBG_MERGE_INTERSECTING
    const GenericVector<BLOBINFO*>& intersecting_blobs = blob->merge.intersecting;
    for(int i = 0; i < intersecting_blobs.length(); ++i) {
      cout << "Displaying intersecting blob " << i << endl;
      M_Utils::dispBlobInfoRegion(intersecting_blobs[i], dbgim);
      M_Utils::waitForInput();
    }
#endif
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif

  // carry out the merge operation on the applicable directions
  GenericVector<BLOBINFO*> all_merges;
  const GenericVector<BLOBINFO*>& mergedown = blob_merge_info.down;
  for(int i = 0; i < mergedown.length(); ++i) {
    if(mergedown[i]->merge.seg_id == -1)
      mergeOperation(blob, mergedown[i], DOWN);
    all_merges.push_back(mergedown[i]);
  }
  const GenericVector<BLOBINFO*>& mergeup = blob_merge_info.up;
  for(int i = 0; i < mergeup.length(); ++i) {
    if(mergeup[i]->merge.seg_id == -1)
      mergeOperation(blob, mergeup[i], UP);
    all_merges.push_back(mergeup[i]);
  }
  const GenericVector<BLOBINFO*>& mergeright = blob_merge_info.right;
  for(int i = 0; i < mergeright.length(); ++i) {
    if(mergeright[i]->merge.seg_id == -1)
      mergeOperation(blob, mergeright[i], RIGHT);
    all_merges.push_back(mergeright[i]);
  }
  const GenericVector<BLOBINFO*>& mergeleft = blob_merge_info.left;
  for(int i = 0; i < mergeleft.length(); ++i) {
    if(mergeleft[i]->merge.seg_id == -1)
      mergeOperation(blob, mergeleft[i], LEFT);
    all_merges.push_back(mergeleft[i]);
  }
  const GenericVector<BLOBINFO*>& intersecting = blob_merge_info.intersecting;
  for(int i = 0; i < intersecting.length(); ++i) {
    if(intersecting[i]->merge.seg_id == -1)
      mergeOperation(blob, intersecting[i], INTERSECT);
    all_merges.push_back(intersecting[i]);
  }
  // recursively repeat for each merged region
  for(int i = 0; i < all_merges.length(); ++i)
    decideAndMerge(all_merges[i], seg_id);
}

void Segmentor1::mergeDecision(BLOBINFO* blob, Direction dir) {
  assert(dir == LEFT || dir == RIGHT || dir == UP || dir == DOWN);
  MergeInfo& merge_info = blob->merge;
  GenericVector<BLOBINFO*> covered_merges; // stores blob merged by the "cover feature"
  GenericVector<BLOBINFO*> stacked_merges;
  BLOBINFO* h_adjacent = NULL;

  // horizontal merge decision
  if(dir == LEFT || dir == RIGHT) {
    int count = countCoveredBlobs(blob, dir, true);
    assert(covered_merges.empty());
    if(dir == LEFT)
      covered_merges = blob->lhabc_blobs;
    else
      covered_merges = blob->rhabc_blobs;
    // is there something to the left or right that's not on the covered list
    // but that is still adjacent?
    TBOX* segbox = merge_info.segment_box;
    BlobInfoGridSearch bigs(grid);
    //const inT16 segbox_center_y = segbox->bottom() + (segbox->height() / 2);
    bigs.StartSideSearch((dir == RIGHT) ? segbox->right() : segbox->left(),
        segbox->bottom(), segbox->top());
    BLOBINFO* n = bigs.NextSideSearch((dir == RIGHT) ? false : true);
    while((n->bounding_box() == blob->bounding_box()) ||
        ((dir == RIGHT) ? (n->left() <= segbox->right())
            : (n->right() >= segbox->left()))
              || (n->top() < segbox->bottom() || n->bottom() > segbox->top())) {
      n = bigs.NextSideSearch((dir == RIGHT) ? false : true);
      if(n == NULL)
        break;
    }
    if(n != NULL) {
      if(isAdjacent(n, blob, dir, merge_info.segment_box))
        h_adjacent = n;
    }
    // if there weren't any covered or horizontally adjacent blobs found yet
    // see if there's something directly to the right or left of the current
    // blob that may constitute an operator or operand depending upon the situation
    if(h_adjacent == NULL && covered_merges.empty()) {
      bigs.StartSideSearch((dir == RIGHT) ? blob->right() : blob->left(),
          blob->bottom(), blob->top());
      n = bigs.NextSideSearch((dir == RIGHT) ? false : true);
      while((n->bounding_box() == blob->bounding_box()) ||
          ((dir == RIGHT) ? (n->left() <= blob->right())
              : (n->right() >= blob->left()))
                || (n->top() < blob->bottom() || n->bottom() > blob->top())) {
        n = bigs.NextSideSearch((dir == RIGHT) ? false : true);
        if(n == NULL)
          break;
        // has to be DIRECTLY to the right or left of the current blob
        // in order to be of significance, otherwise just move on
        if((!(n->bounding_box() == blob->bounding_box()))
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
            cout << "The horizontally adjacent blob is an operand.\n";
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
            cout << "The horizontally adjacent blob is an operator.\n";
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
    int count = countCoveredBlobs(blob, dir, true);
    assert(covered_merges.empty());
    if(dir == DOWN)
      covered_merges = blob->dvabc_blobs;
    else
      covered_merges = blob->uvabc_blobs;

    // using stack feature
    stacked_merges = blob->stacked_blobs;
  }

#ifdef DBG_SHOW_MERGE
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  if(g_dbg_flag) {
#endif
    cout << "Found " << covered_merges.length() << " " <<
        ((dir == UP) ? "upward" : (dir == DOWN) ? "downward"
            : (dir == RIGHT) ? "rightward" : "leftward") << " covered merges\n";
#ifdef DBG_MERGE_VERBOSE
    for(int i = 0; i < covered_merges.length(); ++i) {
      cout << "Displaying covered merge " << i << endl;
      M_Utils::dispBlobInfoRegion(covered_merges[i], dbgim);
      M_Utils::waitForInput();
    }
#endif
    cout << "Found " << stacked_merges.length() << " " <<
        ((dir == UP) ? "upward" : (dir == DOWN) ? "downward"
            : (dir == RIGHT) ? "rightward" : "leftward") << " stacked merges\n";
#ifdef DBG_MERGE_VERBOSE
    for(int i = 0; i < stacked_merges.length(); ++i) {
      cout << "Displaying stacked merge " << i << endl;
      M_Utils::dispBlobInfoRegion(stacked_merges[i], dbgim);
      M_Utils::waitForInput();
    }
#endif
    if(h_adjacent != NULL) {
      cout << "Found a " << ((dir == RIGHT) ? "rightward" : "leftward") << " horizontal merge\n";
#ifdef DBG_MERGE_VERBOSE
      cout << "Displaying the horizontal merge:\n";
      M_Utils::dispBlobInfoRegion(h_adjacent, dbgim);
      M_Utils::waitForInput();
#endif
    }
    else
      cout << "Found 0 " << ((dir == RIGHT) ? "rightward" : "leftward") << " horizontal merges\n";
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif

  // add all the blobs to be merged in the given direction onto the mergeinfo for this blob
  GenericVector<BLOBINFO*>& merge_list = (dir == UP) ? merge_info.up :
      (dir == DOWN) ? merge_info.down : (dir == RIGHT) ? merge_info.right :
          merge_info.left;
  for(int i = 0; i < covered_merges.length(); ++i) {
    const MergeInfo& covered_mergeinfo = covered_merges[i]->merge;
    if(covered_mergeinfo.seg_id > -1)
      continue; // the covered blob is part of a different segment or was already
                // merged to this one so should not be merged again
    merge_list.push_back(covered_merges[i]);
  }
  for(int i = 0; i < stacked_merges.length(); ++i) {
    const MergeInfo& stacked_mergeinfo = stacked_merges[i]->merge;
    if(stacked_mergeinfo.seg_id > -1)
      continue; // the covered blob is part of a different segment or was already
                // merged to this one so should not be merged again
    merge_list.push_back(stacked_merges[i]);
  }
  if(h_adjacent != NULL) {
    const MergeInfo& h_adj_mergeinfo = h_adjacent->merge;
    if(h_adj_mergeinfo.seg_id == -1)
      merge_list.push_back(h_adjacent);
  }
}

void Segmentor1::checkIntersecting(BLOBINFO* blob) {
  MergeInfo& mergeinfo = blob->merge;
  GenericVector<BLOBINFO*> intersecting;
  TBOX* segbox = mergeinfo.segment_box;
  const int& segid = mergeinfo.seg_id;
  BlobInfoGridSearch bigs(grid);
  bigs.StartRectSearch(*segbox);
  BLOBINFO* n = NULL;
  while((n = bigs.NextRectSearch()) != NULL) {
    const MergeInfo& neighbor_mergeinfo = n->merge;
    const int& n_segid = neighbor_mergeinfo.seg_id;
    if(((n_segid == -1) && (n_segid != segid))
        && !intersecting.binary_search(n)) {
      intersecting.push_back(n);
      intersecting.sort();
    }
  }
  mergeinfo.intersecting = intersecting;
}

void Segmentor1::mergeOperation(BLOBINFO* merge_from, BLOBINFO* to_merge,
    Direction merge_dir) {
  assert(merge_dir == RIGHT || merge_dir == LEFT || merge_dir == UP
      || merge_dir == DOWN  || merge_dir == INTERSECT);
  MergeInfo& merge_from_info = merge_from->merge;
  MergeInfo& to_merge_info = to_merge->merge;
  assert(to_merge_info.seg_id == -1); // shouldn't have been merged yet
  Direction opposite_dir = (merge_dir == UP) ? DOWN : (merge_dir == DOWN) ? UP
      : (merge_dir == RIGHT) ? LEFT : (merge_dir == LEFT) ? RIGHT : INTERSECT;
  // logically connect the blob being merged so they are connected both ways
  if(opposite_dir == UP)
    to_merge_info.up.push_back(merge_from);
  else if(opposite_dir == DOWN)
    to_merge_info.down.push_back(merge_from);
  else if(opposite_dir == LEFT)
    to_merge_info.left.push_back(merge_from);
  else if(opposite_dir == RIGHT)
    to_merge_info.right.push_back(merge_from);
  else
    to_merge_info.intersecting.push_back(merge_from);
  // assign merged blob to the segment it's being merged with
  TBOX*& segbox = merge_from_info.segment_box;
  to_merge_info.segment_box = segbox;
  // expand the segment to accomodate the blob being merged if necessary
  TBOX merged_box = to_merge->bounding_box();
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
    cout << "Finished merge operation. Showing the updated segmented and what was merged.\n";
    cout << "Showing what is being merged in the "
         << ((merge_dir == UP) ? "upward" : (merge_dir == DOWN) ? "downward"
             : (merge_dir == RIGHT) ? "rightward" : (merge_dir == LEFT) ? "leftward"
                 : "intersect") << " direction.\n";
    M_Utils::dispBlobInfoRegion(to_merge, dbgim);
    M_Utils::waitForInput();
    cout << "Showing the updated segmentation.\n";
    M_Utils::dispHlTBoxRegion(*(merge_from_info.segment_box), dbgim);
    M_Utils::waitForInput();
#ifdef DBG_SHOW_MERGE_ONE_SEGMENT
  }
#endif
#endif
}

bool Segmentor1::isOperator(BLOBINFO* blob) {
  const char* blobtxt = blob->wordstr();
  if(!blobtxt)
    return false;
  if(Basic_Utils::stringCompare(blobtxt, ">") ||
     Basic_Utils::stringCompare(blobtxt, "<") ||
     Basic_Utils::stringCompare(blobtxt, "=") ||
     Basic_Utils::stringCompare(blobtxt, "+") ||
     Basic_Utils::stringCompare(blobtxt, "-"))
    return true;
  return false;
}

// returns true if the neighbor is already part of the blob's segmentation
bool Segmentor1::wasAlreadyMerged(BLOBINFO* neighbor, BLOBINFO* blob) {
  const MergeInfo& blob_m = blob->merge;
  const MergeInfo& neighbor_m = neighbor->merge;
  const int& blob_seg_id = blob_m.seg_id;
  const int& neighbor_seg_id = neighbor_m.seg_id;
  if(neighbor_seg_id == blob_seg_id)
    return true;
  return false;
}
