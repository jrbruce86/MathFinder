/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   BlobInfoGrid.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Oct 4, 2013 1:17:33 PM
 * ------------------------------------------------------------------------
 * Description: The blobinfo grid supersedes the ColPartitionGrid provided after
 *              all previous document layout analysis as well as all information
 *              that can be ascertained from normal language text recognition.
 *              This grid has all of their combined information and whatever
 *              extra information is needed for proper segmentation of mathematical
 *              expression regions.
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

#include "BlobInfoGrid.h"

#define DBG_INFO_GRID // comment out to disable debugging

namespace tesseract {

BlobInfoGrid::BlobInfoGrid(int gridsize, const ICOORD& bleft,
    const ICOORD& tright) :
        part_grid(NULL), best_col_divisions(NULL), bbgrid(NULL),
          tess(NULL), img(NULL), newapi(NULL) {
    Init(gridsize, bleft, tright);
}

// TODO: Make sure I delete everything I allocated!!!!!!!!!!
BlobInfoGrid::~BlobInfoGrid() {}

void BlobInfoGrid::partGridToBBGrid() {
  ColPartitionGridSearch colsearch(part_grid);
  //ScrollView* part_win = part_grid->MakeWindow(100, 300, "Tesseract Column Partitions");
  //part_grid->DisplayBoxes(part_win);

  // Copy all the blobs in the partition grid to a blob grid
  // The MEDS module detects regions and makes segmentations independently
  // of whatever column partitions have already been made (i.e. it makes no
  // assumptions on the correctness or incorrectness of the Tesseract
  // layout analysis framework in which it is embedded). This module does,
  // however, rely upon various data structures provided with Tesseract and
  // also utilizes Tesseract's character recognition module. The results of
  // this module are combined with those of Tesseract in hopes that whatever
  // correct segmentations already made by Tesseract will remain correct
  // while the proper mathematical expression zones only will be modified.
  int total_blobs_grid = 0; // for debugging, count the total number of blobs in the original BlobGrid
  bbgrid = new BlobGrid(part_grid->gridsize(), part_grid->bleft(), part_grid->tright());
  ColPartition* curpart = NULL;
  colsearch.StartFullSearch();
  while ((curpart = colsearch.NextFullSearch()) != NULL) {
    BLOBNBOX_CLIST* boxes = curpart->boxes();
    CLIST_ITERATOR bbox_it(boxes);
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
        bbox_it.forward()) {
      BLOBNBOX* curbox = (BLOBNBOX*)bbox_it.data();
      //mutils.dispBlobOCRRes(curbox, img, tess);
      // make a new one so that I preserve everything that's already on the colparts grid
      BLOBNBOX* newbox = new BLOBNBOX(*curbox);
      bbgrid->InsertBBox(true, true, newbox);
      total_blobs_grid++;
    }
  }
  #ifdef DBG_INFO_GRID
    cout << "total blobs in grid: " << total_blobs_grid << endl; // debug
  #endif
}

void BlobInfoGrid::recognizeColParts() {
  // Now run Tesseract on each of the column partitions to obtain all of
  // the information on blobs that can be recognized by the language OCR
  // engine. I'll put all of the recognized blobs into my BlobInfoGrid.
  // On a second pass I will come back and find all of the blobs in the
  // BlobGrid and insert them where they need to be in the blob info grid
  // so that I don't miss anything.
  // TODO: Modify this method so that, rather than shutting down and creating a
  //       new api for each ColPartition, have it reuse the same one. This
  //       might use more memory, but would likely speed things up significantly
  ColPartitionGridSearch colsearch(part_grid);
  ColPartition* curpart = NULL;
  colsearch.StartFullSearch();
  int total_recognized_blobs = 0;
  while ((curpart = colsearch.NextFullSearch()) != NULL) {
    BOX* partbox;
    if(curpart->boxes_count() > 0)
      partbox = mutils.getColPartImCoords(curpart, img);
    else {
      TBOX b = curpart->bounding_box();
      partbox = mutils.tessTBoxToImBox(&b, img);
    }
    newapi = new TessBaseAPI();
    // Set up the new api so that it operates on the same image we are analyzing
    // and we assume it is in the same language we have been using as well.
    newapi->Init(tess->datadir.string(), tess->lang.string());
    // need to tell it to save the blob choices so I have access
    newapi->SetVariable("save_blob_choices", "true");
    // run the language-specific OCR on the column partition and
    // store a deep copy of the result for later analysis
    newapi->SetImage(img);
    newapi->SetRectangle(partbox->x, partbox->y, partbox->w, partbox->h);
    char* colpart_result = newapi->GetUTF8Text();
    char* result_deepcopy;
    result_deepcopy = mutils.strDeepCpy(colpart_result);
    recognized_text.push_back(result_deepcopy);
    // Now to get all the info I need on the recognized blobs inside
    // the partition
    const PAGE_RES* pr = newapi->extGetPageResults();
    // The Page_Res is sort of like a Russian doll. There are many layers:
    // BLOCK_RES -> ROW_RES -> WERD_RES -> WERD_CHOICE -> BLOB_CHOICE
    //                         WERD_RES -> WERD        -> C_BLOB
    // Iterate through the block(s) of text inside the partition
    const BLOCK_RES_LIST* block_results = &(pr->block_res_list);
    BLOCK_RES_IT bres_it((BLOCK_RES_LIST*)block_results);
    bres_it.move_to_first();
    for (int i = 0; i < block_results->length(); i++) {
      BLOCK_RES* block_res = bres_it.data();
      // Iterate through the row(s) of text inside the text block
      ROW_RES_LIST* rowreslist = &(block_res->row_res_list);
      ROW_RES_IT rowresit(rowreslist);
      rowresit.move_to_first();
      for(int j = 0; j < rowreslist->length(); j++) {
        ROW_RES* rowres = rowresit.data();
        WERD_RES_LIST* wordreslist = &(rowres->word_res_list);
        // Iterate through the words inside the row (within the block within the partition)
        WERD_RES_IT wordresit(wordreslist);
        wordresit.move_to_first();
        for(int k = 0; k < wordreslist->length(); k++) {
          WERD_RES* wordres = wordresit.data();
          WERD* word = wordres->word;
          WERD_CHOICE* wordchoice = wordres->best_choice;
          // Iterate through the blobs that are within the current word (within the row within the
          // block within the partition) <= russian doll..
          C_BLOB_LIST* bloblist = word->cblob_list();
          C_BLOB_IT blob_it(bloblist);
          blob_it.move_to_first();
          for(int l = 0; l < bloblist->length(); l++) {
            // Find the bounding box for the blob, need to convert it to
            // Grid coordinates which involve y=0 being the bottom of the image
            C_BLOB* recblob = blob_it.data();
            BOX* recbox = mutils.getCBlobImCoords(recblob,
                newapi->GetThresholdedImage()); // remember the box is in the partition space
            recbox->x += partbox->x; // convert to image space (from partition space)
            recbox->y += partbox->y;

            TBOX recbb = mutils.imToCBlobGridCoords(recbox, img); // convert to GridCoords
            // Create the blob's BlobInfo object so it has the proper grid
            // coorneed to split that
            // BlobInfo element dinates and everything else. Making sure to do deep copies
            // of everything that belongs to the language-specific api I'm using
            // so that I can delete the api when I'm done with it.
            BLOBINFO* blobinfo = new BLOBINFO(recbb);
            blobinfo->original_part = curpart; // can shallow copy, since this is owned outside our scope
            blobinfo->recognized_blob = recblob->deep_copy(recblob); // deep copy
            if(wordchoice == NULL) {
              blobinfo->word = NULL;
              blobinfo->wordstr = NULL;
              blobinfo->validword = false;
            }
            else {
              blobinfo->word = new WERD_CHOICE(*wordchoice); // deep copy
              const char* wrd = blobinfo->word->unichar_string().string();
              blobinfo->wordstr = mutils.strDeepCpy(wrd);
              if(strcmp(blobinfo->word->unichar_string().string(), "=") == 0
                  || strcmp(blobinfo->word->unichar_string().string(), "i") == 0)
                blobinfo->validword = true;
              else
                blobinfo->validword = newapi->IsValidWord(wordchoice->unichar_string().string());
            }
            // Now to insert the blobinfo object into the BlobInfoGrid
            InsertBBox(true, true, blobinfo);
            total_recognized_blobs++; // for debugging count how many blobs we have here
            blob_it.forward();
          }
          wordresit.forward();
        }
        rowresit.forward();
      }
      bres_it.forward();
    }
    newapi->End();
    delete newapi;
    boxDestroy(&partbox);
  }
  #ifdef DBG_INFO_GRID
    cout << "total blobs recognized: " << total_recognized_blobs << endl; // debug
  #endif
}

void BlobInfoGrid::insertRemainingBlobs() {
  #ifdef DBG_INFO_GRID
    ScrollView* blobinfosv = MakeWindow(100, 100, "Original BlobInfoGrid (just after recognition)");
    DisplayBoxes(blobinfosv);
    mutils.waitForInput();
  #endif

  // I have to first convert the bbgrid to normal coordinates.
  // It's been deskewed which causes problems for me
  BlobGridSearch* bbgridsearch = new BlobGridSearch(bbgrid);
  bbgridsearch->StartFullSearch();
  BLOBNBOX* blob = NULL;
  BlobGrid* newbbgrid = new BlobGrid(bbgrid->gridsize(), bbgrid->bleft(), bbgrid->tright());
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
    blob->set_bounding_box(blob->cblob()->bounding_box()); // convert to non-deskewed
    newbbgrid->InsertBBox(true, true, blob);
  }
  delete bbgridsearch;
  delete bbgrid;
  bbgrid = newbbgrid; // bbgrid is now in normal coordinates
  bbgridsearch = new BlobGridSearch(bbgrid); // start new search

  // First pass: Here I simply add all of the BLOBNBOXes in the
  //             BBGrid to the BlobInfoGrid (either creating a
  //             new BlobInfo element with the BLOBNBOX and inserting
  //             it, or adding the BLOBNBOX to an existing element depending
  //             upon whether or not the BLOBNBOX overlaps any BlobInfo element
  bbgridsearch->StartFullSearch();
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
    // See if there is a BlobInfo object in the BlobInfoGrid that is
    // overlapping the current blob. In order to check for overlap,
    // I use the RectangleEmpty() function with the bounding box of
    // the current blob. This returns false if there is something on
    // the BlobInfoGrid which overlaps the blob.
    TBOX box = blob->bounding_box();
    BLOBINFO* blinfo;

    if(!RectangleEmpty(box)) {
      // Find what it overlaps
      BlobInfoGridSearch blobinfosearch(this);
      blobinfosearch.StartRectSearch(box);
      blinfo = blobinfosearch.NextRectSearch();
    }
    else {
      blinfo = new BLOBINFO(box);
      blinfo->original_part = blob->owner();
    }
    if(blinfo->unrecognized_blobs == NULL)
      blinfo->unrecognized_blobs = new BLOBNBOX_LIST();
    BLOBNBOX* blobcpy = new BLOBNBOX(*blob);
    blinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, blobcpy);
  }

  // Second pass: Here I look for BLOBNBOXes in the bbgrid that overlap elements
  //              in the BlobInfoGrid. If the overlapping BlobInfo element
  //              is not part of a valid word and contains more than just the
  //              current BLOBNBOX then I create a new BlobInfo element for each
  //              BLOBNBOX inside the current one, delete the current one, then
  //              insert the new ones. This way, only the word segmentation of
  //              language-specific text is preserved.
  bbgridsearch->StartFullSearch();
  BlobInfoGridSearch* blobinfosearch = NULL;
  vector<BLOBINFO*> todelete;
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
    TBOX box = blob->bounding_box();
    if(!RectangleEmpty(box)) {
      if(blobinfosearch == NULL) // the search is either new or has been invalidated
        blobinfosearch = new BlobInfoGridSearch(this);
      blobinfosearch->StartRectSearch(box);
      BLOBINFO* blinfo = blobinfosearch->NextRectSearch();
      BLOBNBOX_LIST* bblist = blinfo->unrecognized_blobs;
      if(bblist == NULL) { // if there's a NULL bblist this likely means that the
                           // current blobinfo object is deprecated and should be
                           // deleted
        if(blinfo->dbgjustadded) {
          cout << "ERROR: A blobinfo object just added into the grid has a null blob list!\n";
          exit(EXIT_FAILURE);
        }
        // check for duplicate
        BlobInfoGridSearch bigs(this);
        bigs.StartRectSearch(box);
        BLOBINFO* curblob = NULL;
        int nullcount = 0;
        int count = 0;
        while((curblob = bigs.NextRectSearch()) != NULL) {
          if(!RectangleEmpty(box)) {
            if(curblob->unrecognized_blobs == NULL) {
              nullcount++;
              todelete.push_back(curblob);
            }
            count++;
          }
        }
        if(count > 1) {
          if(count > 2) {
            #ifdef DBG_INFO_GRID
              cout << count << " duplicates detected, " << nullcount << " of which have null lists\n";
            #endif
            continue;
          }
          #ifdef DBG_INFO_GRID
            cout << "amount that had null blob lists (and were added to a list to be deleted later): " << nullcount << endl;
          #endif
          continue;
        }
        else {
          cout << "ERROR: Second pass of insertRemainingBlobs(): A null blobnbox list found, but there were no duplicates!\n";
          exit(EXIT_FAILURE);
        }
      }
      if(!blinfo->validword && (bblist->length() > 1)) {
        // To prevent any confusion, I first remove the blobinfo element from the grid
        // before adding it's BLOBNBOXes
        BLOBINFO* cpyblinfo = new BLOBINFO(*blinfo);
        RemoveBBox(blinfo); // this both removes the blinfo object and deletes it!!
        bblist = cpyblinfo->unrecognized_blobs;
        BLOBNBOX_IT bbit(bblist);
        bbit.move_to_first();
        for(int i = 0; i < bblist->length(); i++) {
          BLOBNBOX* bbox = bbit.data();
          BLOBNBOX* newbbox = new BLOBNBOX(*bbox);
          BLOBINFO* newblinfo = new BLOBINFO(newbbox->bounding_box());
          newblinfo->original_part = bbox->owner();
          newblinfo->unrecognized_blobs = new BLOBNBOX_LIST();
          newblinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, newbbox);
          newblinfo->dbgjustadded = true;
          InsertBBox(true, true, newblinfo); // insert into the grid
          bbit.forward();
        }
        delete blinfo;
        delete blobinfosearch; // search invalidated need to start a new one
        blobinfosearch = NULL;
      }
    }
  }
  delete bbgridsearch;
  bbgridsearch = NULL;

  // delete all the entries with null lists
  for(vector<BLOBINFO*>::iterator b_it = todelete.begin();
      b_it != todelete.end(); b_it++) {
    BLOBINFO* bx = (BLOBINFO*)(*b_it);
    RemoveBBox(bx);
  }

  #ifdef DBG_INFO_GRID
    // now display it!
    ScrollView* blobinfosv2 = MakeWindow(100, 100, "BlobInfoGrid after insertRemainingBlobs()");
    DisplayBoxes(blobinfosv2);
    mutils.waitForInput();
  #endif
}

} // end namespace tesseract

