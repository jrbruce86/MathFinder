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

// debug params (comment out or keep commented out to disable)
//#define DBG_INFO_GRID
//#define DBG_INFO_GRID_S
//#define DBG_INFO_GRID_SHOW_SENTENCE_REGIONS

#include "BlobInfoGrid.h"
#include <ctype.h>

#ifdef DBG_INFO_GRID_S
#include <fstream>
using namespace std;
#endif

namespace tesseract {

BlobInfoGrid::BlobInfoGrid(int gridsize, const ICOORD& bleft,
    const ICOORD& tright) :
            part_grid(NULL), best_col_divisions(NULL), bbgrid(NULL),
            tess(NULL), img(NULL), newapi(NULL), part_win(NULL),
            blobnboxgridview(NULL), rec_col_parts_sv(NULL),
            insertrblobs_sv(NULL), line_sv(NULL), sentence_sv(NULL) {
    Init(gridsize, bleft, tright);
}

// It's very important to delete everything that has been allocated
// to avoid memory corruption
BlobInfoGrid::~BlobInfoGrid() {
  // I've found that I have to manually delete each entry in order
  // for my blobinfo destructor to be called, so I do that here.
  BlobInfoGridSearch bigs(this);
  bigs.StartFullSearch();
  BLOBINFO* blob = NULL;
  while((blob = bigs.NextFullSearch()) != NULL) {
    delete blob; // manually deleting all the elements in the grid
    blob = NULL;
  }
  deleteStringVector(recognized_lines);
  deleteStringVector(recognized_text);
  recognized_lines_numwrds.clear();

  for(int i = 0; i < recognized_sentences.length(); i++) {
    Sentence* cursentence = recognized_sentences[i];
    if(cursentence != NULL) {
      char* txt = cursentence->sentence_txt;
      if(txt != NULL) {
        delete [] txt;
        txt = NULL;
      }
      Boxa* lboxes = cursentence->lineboxes;
      boxaDestroy(&lboxes);
      delete cursentence;
      cursentence = NULL;
    }
  }
  recognized_sentences.clear();

  // just to be safe I'm going to go through and manually
  // delete each entry in the bbgrid as well
  BlobGridSearch bgs(bbgrid);
  bgs.StartFullSearch();
  BLOBNBOX* b = NULL;
  while((b = bgs.NextFullSearch()) != NULL) {
    delete b;
    b = NULL;
  }
  delete bbgrid;
  bbgrid = NULL;

  // delete all scroll views
  freeScrollView(part_win);
  freeScrollView(blobnboxgridview);
  freeScrollView(rec_col_parts_sv);
  freeScrollView(insertrblobs_sv);
  freeScrollView(line_sv);
  freeScrollView(sentence_sv);
}

void BlobInfoGrid::partGridToBBGrid() {
  ColPartitionGridSearch colsearch(part_grid);
#ifdef DBG_INFO_GRID
  part_win = part_grid->MakeWindow(100, 300, "Tesseract Column Partitions");
  part_grid->DisplayBoxes(part_win);
  mutils.waitForInput();
#endif

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
      // make a new one so that I preserve everything that's already on the colparts grid
      BLOBNBOX* newbox = new BLOBNBOX(*curbox);
      bbgrid->InsertBBox(true, true, newbox);
      total_blobs_grid++;
    }
  }
#ifdef DBG_INFO_GRID
  cout << "total blobs in grid: " << total_blobs_grid << endl; // debug
  // this doesn't display anything because the copy contstructor
  // doesn't assign the BLOBNBOX a color. This ok since I'm not
  // using any of the blobregiontype or blobtextflowtype information
  // which was previously assigned to each element.
  blobnboxgridview = bbgrid->MakeWindow(100, 100,
      "Deep copied BLOBNBOX Grid");
  bbgrid->DisplayBoxes(blobnboxgridview);
  mutils.waitForInput();
#endif
}

// I need more information than could be gleaned from just looking at
// the individual blobs alone and their recognition results. Using
// Tesseract's API it is possible to achieve high accuracy on normal
// text regions. I can use sentences extracted from Tesseract's output
// in order to assign each sentence an N-Gram feature. Thus each blob
// which is considered part of such a sentence will be assigned a
// probability of being part of a sentence which contains embedded
// mathematical expressions.. Further, by simply running the OCR engine
// on each individual blob various characters are missed altogether.
// For instance the "=" sign is seen as two horizontal bars, the
// horizontal bars are often misrecognized as "j". Likewise periods
// and commas are often misrecognized. Characters like "l", "I", and "i"
// are mistaken for "1"'s. The dot on top of the i is not, at this stage,
// known to be part of the "i". By taking advantage of information gleaned
// from Tesseract's word recognition these issues can be avoided
// altogether for regions which have relatively normal layout structure.

// Now I use a Tesseract API assigned to the language being used in
// order to recognize all the text in each ColPartition (colpartitions
// gives the page's current segmentatmion from all processing carried out
// by Tesseract's layout analysis framework up to this point). As I
// recognize whatever is in these partitions I move all the information
// that can be gleaned from the recognition into my grid so features may
// be extracted from these results later.
void BlobInfoGrid::recognizeColParts() {
  // Now run Tesseract on each of the column partitions to obtain all of
  // the information on blobs that can be recognized by the language OCR
  // engine. I'll put all of the recognized blobs into my BlobInfoGrid.
  // On a second pass I will come back and find all of the blobs in the
  // BlobGrid and insert them where they need to be in the blob info grid
  // so that I don't miss anything.
  // TODO: See if there's any way to speed it up!! Maybe not shutting down on each iteration
  ColPartitionGridSearch colsearch(part_grid);
  ColPartition* curpart = NULL;
  colsearch.StartFullSearch();
#ifdef DBG_INFO_GRID
  int dbgdispcol = -1;
#endif
  int total_recognized_blobs = 0;
  // Set up the new api so that it operates on the same image we are analyzing
  // and we assume it is in the same language we have been using as well.
  int colpartnum = 1;
  while ((curpart = colsearch.NextFullSearch()) != NULL) {
    BOX* partbox;
    if(curpart->boxes_count() > 0)
      partbox = mutils.getColPartImCoords(curpart, img);
    else {
      TBOX b = curpart->bounding_box();
      partbox = mutils.tessTBoxToImBox(&b, img);
    }
    // run the language-specific OCR on the column partition
    newapi->Init(tess->datadir.string(), tess->lang.string());
    // need to tell it to save the blob choices so I have access
    newapi->SetVariable("save_blob_choices", "true");
    newapi->SetImage(img);
    newapi->SetRectangle(partbox->x, partbox->y, partbox->w, partbox->h);
    char* colpart_result = newapi->GetUTF8Text();

#ifdef DBG_INFO_GRID
    if(colpartnum == dbgdispcol)
      mutils.dispRegion(partbox, img);
#endif

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
            blobinfo->line_rel_colpart = j; // the blob's row index relative to this col partition
            blobinfo->linewordindex = k; // the blob's word index within a row
            //cout << recblob->area() << endl;
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
                  || strcmp(blobinfo->word->unichar_string().string(), "i") == 0) {
                blobinfo->validword = true;
              }
              else
                blobinfo->validword = newapi->IsValidWord(blobinfo->wordstr);
            }
            // Now to insert the blobinfo object into the BlobInfoGrid
            InsertBBox(true, true, blobinfo);
            total_recognized_blobs++; // for debugging count how many blobs we have here
            blob_it.forward();
            boxDestroy(&recbox);
          }
          wordresit.forward();
        }
        rowresit.forward();
      }
      bres_it.forward();
    }
    newapi->End();
    boxDestroy(&partbox);
    char* result_deepcopy;
    result_deepcopy = mutils.strDeepCpy(colpart_result);
    recognized_text.push_back((char*)result_deepcopy);
    colpartnum++;
  }
#ifdef DBG_INFO_GRID
  cout << "total blobs recognized: " << total_recognized_blobs << endl; // debug
  rec_col_parts_sv = MakeWindow(100, 100,
      "Original BlobInfoGrid (after recognizeColParts)");
  DisplayBoxes(rec_col_parts_sv);
  mutils.waitForInput();
#endif
}

// Next step is to iterate through the BlobGrid, inserting all the BLOBNBOXEs
// into their appropriate BLOBINFO object and/or creating a new BLOBINFO object
// for blobs which may not have been recognized at all.
void BlobInfoGrid::insertRemainingBlobs() {
#ifdef DBG_INFO_GRID
  int dbgBLOBNBOXtop = -1;
  int dbgBLOBNBOXleft = -1;
  int dbgBLOBINFOtop = -1;
  int dbgBLOBINFOleft = -1;
#endif
  // I have to first convert the bbgrid to normal coordinates.
  // It's been deskewed which causes problems for me
  BlobGridSearch* bbgridsearch = new BlobGridSearch(bbgrid);
  bbgridsearch->StartFullSearch();
  BLOBNBOX* blob = NULL;
  BlobGrid* newbbgrid = new BlobGrid(bbgrid->gridsize(), bbgrid->bleft(), bbgrid->tright());
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
    blob->set_bounding_box(blob->cblob()->bounding_box()); // convert to non-deskewed
    newbbgrid->InsertBBox(true, true, blob); // shallow copy
  }
  delete bbgridsearch;
  delete bbgrid; // just need to delete the old grid.. new grid points to the same blobs though
  bbgrid = newbbgrid; // bbgrid is now in normal coordinates

  // First pass: Here I simply add all of the BLOBNBOXes in the
  //             BBGrid to the BlobInfoGrid (either creating a
  //             new BlobInfo element with the BLOBNBOX and inserting
  //             it, or adding the BLOBNBOX to an existing element depending
  //             upon whether or not the BLOBNBOX overlaps any previously
  //             recognized element
  bbgridsearch = new BlobGridSearch(bbgrid); // start new search
  bbgridsearch->StartFullSearch();
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
#ifdef DBG_INFO_GRID
    BOX* b = mutils.getBlobBoxImCoords(blob, img);
    if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft)
      cout << "FOUND BLOBNBOX!!!!!! first pass\n";
    boxDestroy(&b);
#endif
    // See if there is a BlobInfo object in the BlobInfoGrid that is
    // overlapping the current blob. In order to check for overlap,
    // I use the RectangleEmpty() function with the bounding box of
    // the current blob. This returns false if there is something on
    // the BlobInfoGrid which overlaps the blob.
    TBOX box = blob->bounding_box();
    BLOBINFO* blinfo = NULL;

    if(!RectangleEmpty(box)) {
      // Find what it overlaps
      BlobInfoGridSearch blobinfosearch(this);
      blobinfosearch.StartRectSearch(box);
      blinfo = blobinfosearch.NextRectSearch();
#ifdef DBG_INFO_GRID
      BOX* b = mutils.getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "BLOBNBOX at the following coordinates:\n";
        mutils.dispBoxCoords(b);
        cout << "overlaps something in the blobinfogrid at the following coordinates:\n";
        TBOX t = blinfo->bounding_box();
        BOX* blb = mutils.tessTBoxToImBox(&t, img);
        mutils.dispBoxCoords(blb);
        boxDestroy(&blb);
      }
      boxDestroy(&b);
#endif
    }
    else {
#ifdef DBG_INFO_GRID
      BOX* b = mutils.getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "BLOBNBOX at the following coordinates:\n";
        mutils.dispBoxCoords(b);
        cout << "doesn't overlap anything\n";
      }
      boxDestroy(&b);
#endif
      blinfo = new BLOBINFO(box);
      blinfo->original_part = blob->owner();
      InsertBBox(true, true, blinfo); // insert the new blobinfo object into the grid
    }
    if(blinfo->unrecognized_blobs == NULL)
      blinfo->unrecognized_blobs = new BLOBNBOX_LIST();
    BLOBNBOX* blobcpy = new BLOBNBOX(*blob);
    blinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, blobcpy);
#ifdef DBG_INFO_GRID
    b = mutils.getBlobBoxImCoords(blobcpy, img);
    if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
      cout << "BLOBNBOX at the following coordinates:\n";
      mutils.dispBoxCoords(b);
      cout << "should be added to tbe BLOBNBOX list for the blobinfo element at the following coordinates:\n";
      TBOX t = blinfo->bounding_box();
      BOX* blb = mutils.tessTBoxToImBox(&t, img);
      mutils.dispBoxCoords(blb);
      boxDestroy(&blb);
    }
    boxDestroy(&b);
#endif
  }

  // Second pass: Here I look for BLOBNBOXes in the bbgrid that overlap elements
  //              in the BlobInfoGrid. If the overlapping BlobInfo element
  //              is not part of a valid word and contains more than just the
  //              current BLOBNBOX then I create a new BlobInfo element for each
  //              BLOBNBOX inside the current one, delete the current one, then
  //              insert the new ones. This way, only the word segmentation of
  //              language-specific text is preserved. The rest is split back up
  //              so it can be processed more thoroughly.
  bbgridsearch->StartFullSearch();
  BlobInfoGridSearch* blobinfosearch = NULL;
  GenericVector<BLOBINFO*> todelete;
  while((blob = bbgridsearch->NextFullSearch()) != NULL) {
#ifdef DBG_INFO_GRID
    BOX* b = mutils.getBlobBoxImCoords(blob, img);
    if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft)
      cout << "FOUND BLOBNBOX!!!!!! second pass\n";
    boxDestroy(&b);
#endif
    TBOX box = blob->bounding_box();
    if(!RectangleEmpty(box)) {
      if(blobinfosearch == NULL) // the search is either new or has been invalidated
        blobinfosearch = new BlobInfoGridSearch(this);
      blobinfosearch->StartRectSearch(box);
      BLOBINFO* blinfo = blobinfosearch->NextRectSearch();
#ifdef DBG_INFO_GRID
      BOX* b = mutils.getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "on second pass, blobnbox overlaps blobinfo element at: \n";
        TBOX t = blinfo->bounding_box();
        BOX* blb = mutils.tessTBoxToImBox(&t, img);
        mutils.dispBoxCoords(blb);
        boxDestroy(&blb);
      }
      boxDestroy(&b);
#endif
      BLOBNBOX_LIST* bblist = blinfo->unrecognized_blobs;
      if(bblist == NULL) { // if there's a NULL bblist this likely means that the
                           // current blobinfo object is deprecated and should be
                           // deleted
        if(blinfo->dbgjustadded) {
          cout << "ERROR: A blobinfo object just added into the grid has a null blob list!\n";
          exit(EXIT_FAILURE);
        }
#ifdef DBG_INFO_GRID
        BOX* b = mutils.getBlobBoxImCoords(blob, img);
        if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft)
          cout << "BLOBNBOX detected as overlapping a blobinfo elemetn that has a NULL list!\n";
        boxDestroy(&b);
#endif
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
#ifdef DBG_INFO_GRID
              cout << "going to delete blob at image coordinates:\n";
              TBOX tbox = curblob->bounding_box();
              BOX* b = mutils.tessTBoxToImBox(&tbox, img);
              mutils.dispBoxCoords(b);
              boxDestroy(&b);
#endif
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
          cout << "ERROR: Second pass of insertRemainingBlobs(): A null blobnbox list found, "
               << "but there were no duplicates!\n";
          exit(EXIT_FAILURE);
        }
      }
      if(!blinfo->validword && (bblist->length() >= 1)) {
        // To prevent any confusion, I first remove the blobinfo element from the grid
        // before adding it's BLOBNBOXes
        BLOBINFO* cpyblinfo = new BLOBINFO(*blinfo); // deep copy
        int line_rel_colpart = blinfo->line_rel_colpart; // the row to assign any new blobs created here
        int linewordindex = blinfo->linewordindex;
        RemoveBBox(blinfo); // this both removes the blinfo object and deletes it!!
        bblist = cpyblinfo->unrecognized_blobs;
        BLOBNBOX_IT bbit(bblist);
        bbit.move_to_first();
        for(int i = 0; i < bblist->length(); i++) {
          BLOBNBOX* bbox = bbit.data();
#ifdef DBG_INFO_GRID
        BOX* b = mutils.getBlobBoxImCoords(bbox, img);
        if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
          cout << "new blob info element being created for BLOBNBOX at\n";
          mutils.dispBoxCoords(b);
        }
        boxDestroy(&b);
#endif
          BLOBNBOX* newbbox = new BLOBNBOX(*bbox);
          BLOBINFO* newblinfo = new BLOBINFO(newbbox->bounding_box());
          newblinfo->original_part = bbox->owner();
          newblinfo->line_rel_colpart = line_rel_colpart;
          newblinfo->linewordindex = linewordindex;
          newblinfo->unrecognized_blobs = new BLOBNBOX_LIST();
          newblinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, newbbox);
          newblinfo->dbgjustadded = true;
          InsertBBox(true, true, newblinfo); // insert into the grid
          bbit.forward();
        }
        // delete the element after it's already been removed
        if(blinfo != NULL) {
          delete blinfo;
          blinfo = NULL;
        }
        if(cpyblinfo != NULL) {
          delete cpyblinfo;
          cpyblinfo = NULL;
        }
      }
    }
  }
  if(blobinfosearch != NULL) {
    delete blobinfosearch; // search invalidated need to start a new one
    blobinfosearch = NULL;
  }
  delete bbgridsearch;
  bbgridsearch = NULL;

  // delete all the duplicate entries with null lists
  for(int i = 0; i < todelete.length(); i++) {
    BLOBINFO* bx = todelete[i];
    RemoveBBox(bx);
  }

#ifdef DBG_INFO_GRID
  // now display it!
  insertrblobs_sv = MakeWindow(100, 100, "BlobInfoGrid after insertRemainingBlobs()");
  DisplayBoxes(insertrblobs_sv);
  mutils.waitForInput();
#endif
}

// here a sentence is simply any group of one or more words starting with
// a capital letter, valid first word, and ending with a period or question mark.
void BlobInfoGrid::findSentences() {
  // initialize the searches I'll be doing
  ColPartitionGridSearch cpgs(part_grid);
  BlobInfoGridSearch bigs(this);
  int colpartnum = 1; // each recognized_text entry corresponds to a colpartition
  int lineindex = 0;

#ifdef DBG_INFO_GRID
  int dbgcolpart = -1; // debug
  bool dbgallparts = false;
#endif

  // 1. split the recognized text into lines and figure out what line each blob belongs to!
  for(int i = 0; i < recognized_text.length(); i++) {
    // split the partition's recognized text into lines
    // often a colpart just corresponds to one line but not always
    char* text = recognized_text[i];
    text = Basic_Utils::removeExtraNLs(text);
    recognized_text[i] = text;

#ifdef DBG_INFO_GRID
    if(colpartnum == dbgcolpart || dbgallparts) {
      cout << "debugging a partition with the following text:\n";
      cout << text << endl;
    }
#endif

    GenericVector<char*> cp_textlines = mutils.lineSplit(text);
    // all the lines should have a trailing \n.. make sure
    for(int j = 0; j < cp_textlines.length(); j++) {
      char* cur = cp_textlines[j];
      cp_textlines[j] = Basic_Utils::ensureTrailingNL(cur);
    }
    // find the colpartition we are on
    ColPartition* cp = NULL;
    cpgs.StartFullSearch();
    for(int i = 0; i < colpartnum; i++)
      cp = cpgs.NextFullSearch();
    if(cp == NULL) {
      cout << "a NULL colpartition in getSentences()!!!!!\n";
      exit(EXIT_FAILURE);
    }
#ifdef DBG_INFO_GRID
    //dbg make sure we are getting the right colpart
    if(colpartnum == dbgcolpart || dbgallparts) {
      BOX* partbox = mutils.getColPartImCoords(cp, img);
      cout << "trying to display region:\n";
      mutils.dispBoxCoords(partbox);
      cout << "image dimensions are w=" << img->w << ", h=" << img->h << endl;
      cout << "region has " << cp_textlines.length() << " lines.\n";
      mutils.dispRegion(partbox, img);
      mutils.waitForInput();
      boxDestroy(&partbox);
    }
#endif

    // make deep copies of and append each recognized line from this column partition
    // to a vector owned by the grid
    for(int j = 0; j < cp_textlines.length(); j++) {
      const char* curline = cp_textlines[j];
      char* cldeepcopy = mutils.strDeepCpy(curline);
      recognized_lines.push_back(cldeepcopy);
      recognized_lines_numwrds.push_back(-1);
    }

    // assign each blob to its line index based on the colpartition and the
    // row it is on in relation to that colpartition (as found earlier from the
    // page_results after recognition)
    TBOX cprect = mutils.getColPartTBox(cp, img);
    bigs.StartRectSearch(cprect);
    int colpartlines = cp_textlines.length(); // the total number of rows in the current column partition
    BLOBINFO* b = NULL;
    // if a line is found to have all NULL recognition results, then don't assign any blob to it
    // pass 1: find out which ones have NULL recognition results if any do
    GenericVector<Line*> lines;
    while((b = bigs.NextRectSearch()) != NULL) {
      int line_rel_colpart = b->line_rel_colpart;
      if(line_rel_colpart == -1) { // the blob wasn't recognized and thus not assigned a line
        b->linestrindex = -1;
        continue;
      }
      // see if the line is on the vector if not add it
      bool found = false;
      Line* l = NULL;
      for(int j = 0; j < lines.length(); j++) {
        l = lines[j];
        if(l->originalindex == line_rel_colpart) {
          found = true;
          break;
        }
      }
      if(!found) {
        l = new Line;
        lines.push_back(l);
        l->originalindex = line_rel_colpart;
        l->newindex = -1;
        l->has_something = false;
      }
      if(b->wordstr != NULL)
        l->has_something = true;
    }

    // assign the lines new numbers, while preserving the orignal number
    // keeping a line's newindex to -1 discards the line. Thus if a blob has the
    // same line as the original of a line set to -1 then that blob will
    // not be assigned a line. a new line index needs to be decremented
    // by however many lines below it that have been discarded
    for(int j = 0; j < lines.length(); j++) {
      Line* l = lines[j];
      if(l->has_something) {
        l->newindex = l->originalindex;
        // count how many below it have nothing
        int decrement = 0;
        for(int k = 0; k < lines.length(); k++) {
          Line* l_ = lines[k];
          if(l_->originalindex < l->originalindex) {
            if(!l_->has_something)
              decrement++;
          }
        }
        l->newindex -= decrement;
      }
    }
    // pass 2: assign the blobs to the text lines that weren't previously discarded
    //         for not having any contents (thus only assigning blobs to valid lines)
    bigs.StartRectSearch(cprect);
    b = NULL;
    while((b = bigs.NextRectSearch()) != NULL) {
      int line_rel_colpart = b->line_rel_colpart;
      if(line_rel_colpart == -1) { // the blob wasn't recognized and thus not assigned a line
        b->linestrindex = -1;
        continue;
      }
      for(int j = 0; j < lines.length(); j++) {
        Line* l = lines[j];
        if(line_rel_colpart == l->originalindex) {
          if(l->has_something) {
            int curlineindex = l->newindex + lineindex;
            b->linestrindex = curlineindex;
            int numwrdsline = recognized_lines_numwrds[curlineindex];
            if(b->linewordindex > numwrdsline)
              recognized_lines_numwrds[curlineindex] = b->linewordindex;
          }
        }
      }
    }
    for(int j = 0; j < lines.length(); j++) {
      delete lines[j];
      lines[j] = NULL;
    }
    lines.clear();
    lineindex += colpartlines;
    colpartnum++;
  }
#ifdef DBG_INFO_GRID
  line_sv = MakeWindow(100, 100, "BlobInfoGrid after assigning blobs to lines");
  DisplayBoxes(line_sv);
  mutils.waitForInput();
#endif
  // finished getting the lines of text for all colpartitions

  // 2: Now that the blobs have been separated into lines, the next step is to
  //    determine where the sentences are relative to those lines
  //    use the api to figure out if words are valid
  newapi->Init(tess->datadir.string(), tess->lang.string());
  int linenum = 0;
  bool sentence_found = false;
  for(int reclineindx = 0; reclineindx < recognized_lines.length();
      reclineindx++) {
    char* linetxt = recognized_lines[reclineindx];
    char prevchar = '\0';
    int curwrdindex = 0; // to track the word index on the current line
    for(int i = 0; i < strlen(linetxt); i++) {
      char curchar = linetxt[i];
      if(prevchar == ' ')
        curwrdindex++;
      if(!sentence_found) { // looking for the start of a sentence
        if((prevchar == '\0' || prevchar == ' ') && isupper(curchar)) {
          // if the uppercase letter is part of a valid word then we have the start of a sentence
          // grab the rest of the word
          int j = i;
          for(j = i; linetxt[j] != ' '; j++);
          int wrdlen = j - i;
          char* uppercaseword = new char[wrdlen+1];
          for(int k = 0; k < wrdlen; k++)
            uppercaseword[k] = linetxt[i+k];
          uppercaseword[wrdlen] = '\0';
          if(islower(uppercaseword[1])) { // uppercase should be followed by a lowercase letter
            if(newapi->IsValidWord(uppercaseword)) { // make sure its a valid word
              // found the start of a sentence!!
              Sentence* sentence = new Sentence;
              sentence->start_line_num = linenum;
              sentence->startchar_index = i;
              sentence->startwrd_index = curwrdindex;
              recognized_sentences.push_back(sentence);
              sentence_found = true;
            }
          }
          delete [] uppercaseword;
          uppercaseword = NULL;
        }
      }
      else { // looking for the end of the current sentence
             // for now I'll assume a '.' or '?' is the end of the sentence
             // (also I'll count the last character on the page as the end
             // of a sentence if the page ends while a sentence ending is
             // still being looked for)
        if((curchar == '.') || (curchar == '?') ||
            ((recognized_lines.length() == (reclineindx+1))
            && (strlen(linetxt) == (i+1)))) {
          Sentence* sentence = (Sentence*)recognized_sentences.back();
          sentence->end_line_num = linenum;
          sentence->endchar_index = i;
          sentence->endwrd_index = curwrdindex;
          // need to copy all the text found in the sentence over to the structure
          const int startln = sentence->start_line_num;
          const int startchr = sentence->startchar_index;
          const int endln = sentence->end_line_num;
          const int endchr = sentence->endchar_index;
          int charcount = 0;
          bool done = false;
          // first need to count the characters
          for(int i = startln; i <= endln; i++) {
            char* ln = recognized_lines[i];
            for(int j = (i == startln ? startchr : 0); j < strlen(ln); j++) {
              charcount++;
              if(i == endln && j == endchr) {
                done = true;
                break;
              }
            }
            if(done)
              break;
          }
          char* newsentence = new char[charcount+1]; // allocate the memory
          int charindex = 0;
          done = false;
          // now copy everything over
          for(int i = startln; i <= endln; i++) {
            char* ln = recognized_lines[i];
            for(int j = (i == startln ? startchr : 0); j < strlen(ln); j++) {
              newsentence[charindex] = ln[j];
              if(i == endln && j == endchr) {
                done = true;
                break;
              }
              charindex++;
            }
            if(done)
              break;
          }
          newsentence[charcount] = '\0';
          sentence->sentence_txt = newsentence; // finally set the datastructure
          sentence_found = false;
        }
      }
      prevchar = curchar;
    }
    linenum++;
  }

  // Done finding the sentences, optional debugging below
#ifdef DBG_INFO_GRID
  cout << "...............................................\n";
  cout << "here are the " << recognized_sentences.length()
       << " sentences that were found:\n";
  for(int i = 0; i < recognized_sentences.size(); i++) {
    char* sentence = recognized_sentences[i]->sentence_txt;
    cout << i << ": " << ((sentence != NULL) ? sentence : "NULL")
         << endl << "------\n";
  }
  cout << "................................................\n";
  mutils.waitForInput();
#endif

  // 3. Now the next step is to assign each blob in the BlobInfoGrid to a sentence
  //    if it belongs to a textline that's part of one
#ifdef DBG_INFO_GRID
  const int dbgline = -1;
#endif
  bigs.StartFullSearch();
  BLOBINFO* bb = NULL;
  char* prevwrdstr = NULL;
  BLOBINFO* prevbb = NULL;
  int prevwrdindex = 0;
  int prevlineindex = -1;
  while((bb = bigs.NextFullSearch()) != NULL) {
    const int lineindex = bb->linestrindex; // the text line this blob belongs to
    // first find all the candidate sentences, these are the sentences that
    // are on the same line as this blob. If there is only one or none of
    // these then the job is done. If there are more than one then there's
    // more than one sentence on the line this blob belongs to and we need
    // to figure out which one
    if(lineindex == -1)
      continue;
    GenericVector<int> sentence_candidates;
    int sentence_index = 0;
    for(int i = 0; i < recognized_sentences.length(); i++) {
      Sentence* cursentence = recognized_sentences[i];
      int cs_startln = cursentence->start_line_num;
      int cs_endln = cursentence->end_line_num;
      if((cs_startln <= lineindex) && (lineindex <= cs_endln))
        sentence_candidates.push_back(sentence_index);
      sentence_index++;
    }

#ifdef DBG_INFO_GRID
    if(lineindex == dbgline) {
      cout << "displaying a bounding box that was set to line " << lineindex << endl;
      mutils.dispHlBlobInfoRegion(bb, img);
      mutils.waitForInput();
    }
#endif
    char* curwrdstr = bb->wordstr;
#ifdef DBG_INFO_GRID
    if(lineindex == dbgline) {
      cout << "--------------------------\nthe candidates for the following line:\n"
           << recognized_lines[lineindex] << endl;
      for(int i = 0; i < sentence_candidates.length(); i++) {
        int candidate = sentence_candidates[i];
        cout << i << ": " << recognized_sentences[candidate]->sentence_txt << endl;
      }
      cout << "-----\n";
      cout << "displaying the blob at coordinates:\n";
      BOX* bbbox = mutils.getBlobInfoBox(bb, img);
      mutils.dispBoxCoords(bbbox);
      mutils.dispBlobInfoRegion(bb, img);
      mutils.dispHlBlobInfoRegion(bb, img);
      mutils.waitForInput();
      boxDestroy(&bbbox);
    }
#endif
    sentence_index = 0;
    if(sentence_candidates.empty())
      goto moveon; // the blob has no sentence, move on
    if(sentence_candidates.size() == 1) {
      // make sure the word index is within the sentence's bounds
      int wordindex = bb->linewordindex; // this gives an estimate but may be wrong
#ifdef DBG_INFO_GRID
      if(lineindex == dbgline)
        cout << "there's only one sentence candidate for the word, "
             << ((bb->wordstr == NULL) ? "NULL" : bb->wordstr) << ", with wordindex: "
             << wordindex << endl;
#endif
      Sentence* candidate = recognized_sentences[sentence_candidates[0]];
#ifdef DBG_INFO_GRID
      if(lineindex == dbgline) {
        cout << " a blob with the following line is a candidate for the following sentence::\n";
        cout << "line index: " << lineindex << endl;
        cout << "line:\n" << recognized_lines[lineindex] << endl;
        cout << "sentence:\n" << recognized_sentences[sentence_candidates[0]]->sentence_txt << endl;
        cout << "the sentence's start line index is "
             << recognized_sentences[sentence_candidates[0]]->start_line_num
             << " and the end line is "
             << recognized_sentences[sentence_candidates[0]]->end_line_num << endl;
        cout << "the sentence is sentence number " << sentence_candidates[0] << endl;
        mutils.waitForInput();
      }
#endif
      if(candidate->start_line_num == candidate->end_line_num) {
#ifdef DBG_INFO_GRID
        if(lineindex == dbgline)
          cout << "the sentence starts and ends on the same line!\n";
#endif
        if(wordindex >= candidate->startwrd_index && wordindex <= candidate->endwrd_index) {
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "blob's word is within the sentence's bounds! so found it!\n";
#endif
          bb->sentenceindex = sentence_candidates[0]; // found it!
        }
      }
      else {
#ifdef DBG_INFO_GRID
        if(lineindex == dbgline)
#endif
        if(lineindex > candidate->start_line_num && lineindex < candidate->end_line_num) {
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "the blob is in between the start and end line! Got it!\n";
#endif
          bb->sentenceindex = sentence_candidates[0]; // found it!
        }
        if(lineindex == candidate->start_line_num) {
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "the blob is on the start line!\n";
#endif
          if(wordindex >= candidate->startwrd_index) {
#ifdef DBG_INFO_GRID
            if(lineindex == dbgline)
              cout << "the blob is after the start word so got it!\n";
#endif
            bb->sentenceindex = sentence_candidates[0]; // found it!
          }
        }
        if(lineindex == candidate->end_line_num) {
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "the blob is on the end line!\n";
#endif
          if(wordindex <= candidate->endwrd_index) {
#ifdef DBG_INFO_GRID
            if(lineindex == dbgline)
              cout << "the blob is before the end of the sentence so got it!\n";
#endif
            bb->sentenceindex = sentence_candidates[0]; // found it!
          }
        }
      }
    }
    else { // more than one sentence candidate for this blob
      if(bb->wordstr == NULL) { // if we get to this point we'll need the string of the
                                // recognized word belonging to the blob in order to move forward
        // without it we just move on
        goto moveon;
      }
#ifdef DBG_INFO_GRID
      if(lineindex == dbgline) {
        cout << "sentence candidates:\n";
        cout << "the candidates for the following line:\n" << recognized_lines[lineindex] << endl;
        cout << "and the following word on that line: " << bb->wordstr << endl;
        int canindex = 0;
        for(int i = 0; i < sentence_candidates.length(); i++) {
          int candidate = sentence_candidates[i];
          cout << canindex << ": " << recognized_sentences[candidate]->sentence_txt << endl;
          canindex++;
        }
      }
#endif
      // theres more than one candidate, figure out where on its text
      // line the blob is
      int wordindex = bb->linewordindex; // this gives an estimate but may be wrong
#ifdef DBG_INFO_GRID
      if(lineindex == dbgline)
        cout << "with wordindex: " << wordindex << endl;
#endif
      if(wordindex == -1)
        goto moveon;

      // find the word this wordindex corresponds to on the blob's line
      char* linetxt = recognized_lines[lineindex];
      int wordcount = recognized_lines_numwrds[lineindex];
      if(wordindex > wordcount) {
#ifdef DBG_INFO_GRID
        cout << "the blob corresponding to the word " << bb->wordstr
             << " one line # " << lineindex
             << " has a word index outside the bounds of its line!\n";
        cout << "here is the line:\n";
        cout << linetxt << endl;
        cout << "here is the word index: " << wordindex << endl;
        cout << "here is the number of words expected on teh line: " << wordcount << endl;
        cout << "updating for now...\n";
        mutils.waitForInput();
#endif
        recognized_lines_numwrds[lineindex] = wordindex + 1;
      }
      // found the blob's word on the line, now figure out which
      // candidate sentence contains the word at this word index
      const int linewordindex = wordindex;
      int can_index = 0;
      for(int i = 0; i < sentence_candidates.length(); i++) {
        int candidate = sentence_candidates[i];
        Sentence* c_sentence = recognized_sentences[candidate];
#ifdef DBG_INFO_GRID
        if(lineindex == dbgline)
          cout << "checking candidate " << can_index << endl;
#endif
        // if its on a line in between the start and end line of this setence we know
        // this is the one
        if(lineindex != c_sentence->start_line_num
            && lineindex != c_sentence->end_line_num) {
          // found the sentence!! set it and break
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "detected as on a line in between the start and end line of sentence # "
                 << candidate << ", sentence found!!\n";
#endif
          bb->sentenceindex = candidate;
          break;
        }
        // if the sentence starts and ends on the same line
        if(c_sentence->start_line_num == c_sentence->end_line_num) {
          // and we're at an index before it ends and after it starts we've got it
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "the sentence starts and ends on the same line!\n";
#endif
          if(lineindex == c_sentence->start_line_num
              && linewordindex <= c_sentence->endwrd_index
              && linewordindex >= c_sentence->startwrd_index) {
#ifdef DBG_INFO_GRID
            if(lineindex == dbgline)
              cout << "at an index before sentence # " << candidate
                   << " ends but after it starts! found it!!\n";
#endif
            // found the sentence!!
            bb->sentenceindex = candidate;
            break;
          }
        }
        // if the sentence doesn't start and end on the same line
        else {
          // if we're on the sentence's start line just need to make sure that
          // we're at an index after the sentence starts
#ifdef DBG_INFO_GRID
          if(lineindex == dbgline)
            cout << "the sentence starts and ends on different lines\n";
#endif
          if(lineindex == c_sentence->start_line_num) {
#ifdef DBG_INFO_GRID
            if(lineindex == dbgline)
              cout << "on the sentence's start line!\n";
#endif
            if(linewordindex >= c_sentence->startwrd_index) {
#ifdef DBG_INFO_GRID
              if(lineindex == dbgline)
                cout << " the character is after the start of sentence # "
                     << candidate << ". found it!!!\n";
#endif
              // found it!!
              bb->sentenceindex = candidate;
              break;
            }
          }
          // if we're on the sentence's end line just make sure that we're at
          // an index before the sentence ends
          if(lineindex == c_sentence->end_line_num) {
#ifdef DBG_INFO_GRID
            if(lineindex == dbgline) {
              cout << "on the sentence's end line!!\n";
              cout << "the word index is " << linewordindex << " and the sentence ends on index " << c_sentence->endwrd_index << endl;
            }
#endif
            if(linewordindex <= c_sentence->endwrd_index) {
#ifdef DBG_INFO_GRID
              if(lineindex == dbgline)
                cout << "the character is before the end of sentence # "
                     << candidate << ". found it!!!\n";
#endif
              // found it!!
              bb->sentenceindex = candidate;
              break;
            }
          }
        }
        can_index++;
      }
    }
    moveon:
    prevbb = bb;
    prevwrdstr = curwrdstr;
    prevlineindex = lineindex;
    prevwrdindex = bb->linewordindex;
    sentence_candidates.clear();
  }
#ifdef DBG_INFO_GRID
  sentence_sv = MakeWindow(100, 100, "BlobInfoGrid after getting the sentences");
  DisplayBoxes(sentence_sv);
  mutils.waitForInput();
#endif
  newapi->End();
  getSentenceRegions();
}


void BlobInfoGrid::getSentenceRegions() {
  M_Utils mutils;
#ifdef DBG_INFO_GRID_S
  int dbgsentence = -1;
#endif
  // 1. Assign each sentence to a boxarray which gives the isothetic
  //    region corresponding to the sentence (each box is a line of
  //    that sentence)
  for(int j = 0; j < recognized_sentences.length(); j++) {
    Sentence* cursentence = recognized_sentences[j];
    int startline = cursentence->start_line_num;
    int endline = cursentence->end_line_num;
    int numlines = endline - startline + 1;
#ifdef DBG_INFO_GRID_S
    if(j == dbgsentence) {
      cout << "about to find line boxes for sentence " << j << ":\n";
      cout << recognized_sentences[j]->sentence_txt << endl;
      cout << "the lines of this sentence are " << startline << "-" << endline << endl;
      mutils.waitForInput();
    }
#endif
    // holds separate box for each line of the sentence
    Boxa* sentencelines = boxaCreate(numlines);
    BlobInfoGridSearch bigs(this);
    BLOBINFO* blob = NULL;
    bigs.StartFullSearch();
    GenericVector<int> lefts;
    GenericVector<int> rights;
    GenericVector<int> tops;
    GenericVector<int> bottoms;
    for(int k = 0; k < numlines; k++) {
      lefts.push_back(INT_MAX);
      rights.push_back(INT_MIN);
      tops.push_back(INT_MAX);
      bottoms.push_back(INT_MIN);
    }
    // find the bounding boxes for each line of the sentence
    while((blob = bigs.NextFullSearch()) != NULL) {
      if(blob->sentenceindex == j) {
        BOX* blobbox = mutils.getBlobInfoBox(blob, img);
        int blobline = blob->linestrindex;
        if(blobline < startline || blobline > endline) {
          // blob was incorrectly assigned to a sentence
          cout << "ERROR: A blob was incorrectly assigned to a sentence (ignoring for now)\n";
          boxDestroy(&blobbox);
          continue;
        }
#ifdef DBG_INFO_GRID_S
        if(j == dbgsentence) {
          cout << "the displayed blob belongs to the sentence\n";
          mutils.dispHlBlobInfoRegion(blob, img);
          if(blob->wordstr != NULL)
            cout << "it belongs to the following word: " << blob->wordstr << endl;
          cout << "it belongs to following line #: " << blobline << endl;
          cout << "the blob's coords are:\n";
          mutils.dispBoxCoords(blobbox);
          mutils.waitForInput();
        }
#endif
        int s_line = blobline - startline; // line relative to sentence
        if(blobbox->x < lefts[s_line])
          lefts[s_line] = blobbox->x;
        if((blobbox->x + blobbox->w) > rights[s_line])
          rights[s_line] = blobbox->x + blobbox->w;
        if(blobbox->y < tops[s_line])
          tops[s_line] = blobbox->y;
        if((blobbox->y + blobbox->h) > bottoms[s_line])
          bottoms[s_line] = blobbox->y + blobbox->h;
        boxDestroy(&blobbox);
      }
    }
    for(int k = 0; k < numlines; k++) { // add the line bounding boxes
      BOX* linebox = NULL;
      if(lefts[k] != INT_MAX && rights[k] != INT_MIN
          && tops[k] != INT_MAX && bottoms[k] != INT_MIN)
        linebox = boxCreate(lefts[k], tops[k], rights[k]-lefts[k], bottoms[k]-tops[k]);
      if(linebox == NULL)
        linebox = boxCreate(0,0,0,0); // empty box if the line had no blobs
      boxaAddBox(sentencelines, linebox, L_INSERT);
    }
    cursentence->lineboxes = sentencelines;
    lefts.clear();
    rights.clear();
    tops.clear();
    bottoms.clear();
  }

  // Reiterate through the grid to make sure each blob is assigned
  // to the sentence to which it belongs. If a blob's bounding box
  // intersects with a line of a sentence then it should belong to
  // that sentence (some blobs may have been missed during initial
  // assignment).
  BLOBINFO* curblob = NULL;
  BlobInfoGridSearch bigs(this);
  bigs.StartFullSearch();
  while((curblob = bigs.NextFullSearch()) != NULL) {
    // if it's already been assigned a sentence then move on
    if(curblob->sentenceindex != -1)
      continue;
    Box* bbox = mutils.getBlobInfoBox(curblob, img);
    bool found = false;
    for(int j = 0; j < recognized_sentences.length(); j++) {
      Boxa* cursentenceboxes = recognized_sentences[j]->lineboxes;
      for(int k = 0; k < cursentenceboxes->n; k++) {
        Box* linebox = boxaGetBox(cursentenceboxes, k, L_CLONE);
        int is_box_in_line = 0;
        boxIntersects(linebox, bbox, &is_box_in_line);
        boxDestroy(&linebox); // destroying the clone to decrement the reference count
        if(is_box_in_line) {
          curblob->sentenceindex = j;
          found = true;
          break;
        }
      }
      if(found)
        break;
    }
    boxDestroy(&bbox);
  }

#ifdef DBG_INFO_GRID_S
#ifdef DBG_INFO_GRID_SHOW_SENTENCE_REGIONS
  // for debugging, color the blobs for each sentence region and display the results
  // while showing the contents of the sentence on the terminal. do one at
  // a time requiring user input to continue in between displaying each sentence
  Lept_Utils lu;
  bool displayon = false;
  bool showlines = false; // if showlines is false then highlights the blobs
                          // belonging to each sentence, otherwise highlights
                          // the lines belonging to each sentence
  LayoutEval::Color color = LayoutEval::RED;
  Pix* dbgim = pixCopy(NULL, img);
  dbgim = pixConvertTo32(dbgim);
  static int sentencedbgimnum = 1;
  string sentencedbgname = (string)"SentenceDBG"
      + Basic_Utils::intToString(sentencedbgimnum);
  sentencedbgimnum++;
  string sentencefilename = sentencedbgname + ".txt";
  ofstream sentencedbgfile(sentencefilename.c_str());
  if(!sentencedbgfile.is_open()) {
    cout << "ERROR: Could not open debug file for writing in " \
         << sentencefilename << endl;
    exit(EXIT_FAILURE);
  }
  for(int j = 0; j < recognized_sentences.length(); j++) {
    Sentence* cursentence = recognized_sentences[j];
    sentencedbgfile << j << ": " << cursentence->sentence_txt
                    << "\n\n----------------------------------\n\n";
    BOXA* cursentencelines = cursentence->lineboxes;
    if(j == dbgsentence) {
      if(showlines) {
        cout << "about to display each individual line for the following sentence:\n"\
             << cursentence->sentence_txt << endl;
      }
      else {
        cout << "about to display each individual blob for the following sentence:\n"\
             << cursentence->sentence_txt << endl;
      }
      mutils.waitForInput();
    }
    if(showlines) {
      for(int k = 0; k < cursentencelines->n; k++) {
        BOX* sentenceline = boxaGetBox(cursentencelines, k, L_CLONE);
        mutils.dispBoxCoords(sentenceline);
        lu.fillBoxForeground(dbgim, sentenceline, color);
        if(j == dbgsentence) {
          cout << "highlighting blobs in line " << k + cursentence->start_line_num
               << " of sentence " << j << endl;
          cout << "here is the line's text:\n";
          cout << recognized_lines[k+cursentence->start_line_num] << endl;
          if(displayon) {
            pixDisplay(dbgim, 100, 100);
            mutils.waitForInput();
          }
        }
        boxDestroy(&sentenceline); // destroy the clone (reduce reference count back to 1)
      }
    }
    else {
      BLOBINFO* curblob = NULL;
      BlobInfoGridSearch bigs(this);
      bigs.StartFullSearch();
      while((curblob = bigs.NextFullSearch()) != NULL) {
        if(curblob->sentenceindex == j) {
          BOX* curbox = mutils.getBlobInfoBox(curblob, img);
          lu.fillBoxForeground(dbgim, curbox, color);
          boxDestroy(&curbox);
        }
      }
    }
    pixWrite((sentencedbgname+".png").c_str(), dbgim, IFF_PNG);
    if(displayon)
      cout << "displaying the " << (showlines ? " lines" : " blobs")
           << " for sentence " << j << endl;
    cout << j << endl;
    cout << "Sentence:\n" << cursentence->sentence_txt << endl;
    cout << "image: " << tess->imagebasename.string() << endl;
    cout << "lines: " << cursentence->start_line_num << "-" << cursentence->end_line_num << endl;
    if(displayon) {
      pixDisplay(dbgim, 100, 100);
      mutils.waitForInput();
    }
    color = (color == LayoutEval::GREEN) ? LayoutEval::RED
        : (LayoutEval::Color)(color + 1);
  }
  pixDestroy(&dbgim);
#endif
#endif
}


void BlobInfoGrid::HandleClick(int x, int y) {
  cout << "-----------------------------------\n";
  cout << "\nx,y: " << x << ", " << y << endl;
  BlobInfoGridSearch bigs(this);
  bigs.StartRadSearch(x, y, 0);
  BLOBINFO* bb = bigs.NextRadSearch();
  if(bb == NULL)
    return;
  cout << "the word corresponding to the blob you clicked:\n";
  if(bb->wordstr != NULL)
    cout << bb->wordstr << endl;
  else
    cout << "(NULL)\n";
  // find the colpartition we are on
/*  ColPartitionGridSearch cpgs(part_grid); // comment from here to
  ColPartition* cp = NULL;
  cpgs.StartFullSearch();
  int partnum = 1;
  cp = cpgs.NextFullSearch();
  while(cp != bb->original_part) {
    partnum++;
    cp = cpgs.NextFullSearch();
  }
  cout << "on partition number " << partnum << endl; // here ^*/
  if(bb->linestrindex != -1) {
    if(recognized_lines[bb->linestrindex] != NULL) {
      cout << "the blob you clicked on belongs to the following text line at line index " << bb->linestrindex << ":\n"
          << recognized_lines[bb->linestrindex] << endl;
    }
    else
      cout << "the blob you clicked on belongs to text line " << bb->linestrindex << ", which is NULL\n";
  }
  else
    cout << "the blob you clicked on was not assigned a text line\n";
  if(bb->sentenceindex == -1) {
    cout << "the blob you clicked on was not assigned a sentence\n......\n";
    return;
  }
  if(recognized_sentences[bb->sentenceindex] != NULL) {
    cout << "the blob you clicked on belongs to the following sentence:\n"
         << recognized_sentences[bb->sentenceindex]->sentence_txt << endl << "......\n";
  }
  else
    cout << "the blob you clicked belongs to sentence number " << bb->sentenceindex << ", which is NULL\n......\n";
}


} // end namespace tesseract
