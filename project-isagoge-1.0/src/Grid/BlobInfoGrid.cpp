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
#define SHOW_BLOB_WINDOW
//#define DBG_BASELINE


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
            tess(NULL), img(NULL), api(NULL), part_win(NULL),
            blobnboxgridview(NULL), rec_col_parts_sv(NULL),
            insertrblobs_sv(NULL), line_sv(NULL), sentence_sv(NULL),
            dbgfeatures(false) {
  Init(gridsize, bleft, tright);
}

// It's very important to delete everything that has been allocated
// to avoid memory corruption
BlobInfoGrid::~BlobInfoGrid() {
  // I've found that I have to manually delete each entry in order
  // for my blobinfo destructor to be called, so I do that here.
  BlobInfoGridSearch bigs(this);
  bigs.StartFullSearch();
  BLOBINFO* blob = bigs.NextFullSearch();
  while(blob != NULL) {
    BLOBINFO* next = bigs.NextFullSearch();
    delete blob; // manually deleting all the elements in the grid
    blob = next;
  }

  for(int i = 0; i < recognized_sentences.length(); i++) {
    Sentence* cursentence = recognized_sentences[i];
    if(cursentence != NULL) {
      if(cursentence->ngrams != NULL) {
        NGramRanker ng;
        ng.destroyNGramVecs(*(cursentence->ngrams));
        delete cursentence->ngrams;
        cursentence->ngrams = NULL;
      }
      delete cursentence;
      cursentence = NULL;
    }
  }
  recognized_sentences.clear();

  for(int i = 0; i < rows.length(); ++i) {
    ROW_INFO* cur_row = rows[i];
    if(cur_row != NULL) {
      delete cur_row;
      cur_row = NULL;
    }
  }
  rows.clear();

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

  if(api != NULL) {
    delete api;
    api = NULL;
  }
}

void BlobInfoGrid::partGridToBBGrid() {
  ColPartitionGridSearch colsearch(part_grid);
#ifdef DBG_INFO_GRID
  part_win = part_grid->MakeWindow(100, 300, "Tesseract Column Partitions");
  part_grid->DisplayBoxes(part_win);
  M_Utils::waitForInput();
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
  M_Utils::waitForInput();
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
void BlobInfoGrid::recognizePage() {
  // Set up the new api so that it operates on the same image we are analyzing
  // and we assume it is in the same language we have been using as well.
  // The api is run on the entire page rather than the individual column partitions
  // in order to get the best results. The api is thus utilized for the entire scope
  // of the remainder of the BlobInfoGrid's lifetime. There is no need to make any deep
  // copies of objects taken from the api, however it is necessary to NULLify pointers
  // from the api before this class's destruction to avoid dangling pointer issues!
  // run the language-specific OCR on the entire page
  api->Init(tess->datadir.string(), tess->lang.string());
  // need to tell it to save the blob choices so I have access
  api->SetVariable("save_blob_choices", "true");
  api->SetImage(img);
  char* page_result_str = api->GetUTF8Text();
  const PAGE_RES* pr = api->extGetPageResults();
  // The Page_Res is sort of like a Russian doll. There are many layers:
  // BLOCK_RES -> ROW_RES -> WERD_RES -> WERD_CHOICE -> BLOB_CHOICE
  //                         WERD_RES -> WERD        -> C_BLOB
  // Iterate through the block(s) of text inside the page
  int cur_row_index = 0;
  const BLOCK_RES_LIST* block_results = &(pr->block_res_list);
  BLOCK_RES_IT bres_it((BLOCK_RES_LIST*)block_results);
  bres_it.move_to_first();
  for (int i = 0; i < block_results->length(); ++i) {
    BLOCK_RES* block_res = bres_it.data();
    // Iterate through the row(s) of text inside the text block
    ROW_RES_LIST* rowreslist = &(block_res->row_res_list);
    ROW_RES_IT rowresit(rowreslist);
    rowresit.move_to_first();
    for(int j = 0; j < rowreslist->length(); j++) {
      ROW_RES* rowres = rowresit.data();
      // find what colpartition this row is inside of (if any)
      TBOX rowbox = rowres->row->bounding_box();
      ColPartition* row_colpart = M_Utils::getTBoxColPart(part_grid, rowbox, img);
      // now iterate through the words to create all the blobinfo objects
      WERD_RES_LIST* wordreslist = &(rowres->word_res_list);
      // initialize the ROW_INFO object for this row which stores a shallow copy of it
      // and also contains various other useful information about the row
      initRowInfo(rowres);
      rows.back()->rowid = cur_row_index;
      rows.back()->words = wordreslist;
      bool row_has_valid = findValidWordOnRow(rows.back());
      WERD_RES_IT wordresit(wordreslist);
      wordresit.move_to_first();
      for(int k = 0; k < wordreslist->length(); k++) {
        WERD_RES* wordres = wordresit.data();
        addWordToLastRowInfo(wordres);
        const FontInfo* font = wordres->fontinfo;
        bool is_italic = false;
        if(font != NULL)
          is_italic = wordres->fontinfo->is_italic();
        WERD* word = wordres->word;
        // Iterate through the blobs that are within the current word
        C_BLOB_LIST* bloblist = word->cblob_list();
        C_BLOB_IT blob_it(bloblist);
        blob_it.move_to_first();
        for(int l = 0; l < bloblist->length(); l++) {
          C_BLOB* recblob = blob_it.data();
          BLOBINFO* blobinfo = new BLOBINFO(recblob->bounding_box());
          blobinfo->wordinfo = rows.back()->get_wordinfo_last();
          blobinfo->werdres = wordres; // ptr to the wordres to which the blob belongs
          blobinfo->original_part = row_colpart; // shallow copy, since this is owned outside
                                                 // this class scope (could also be NULL though)
          blobinfo->block_index = i; // the blob's block index on the entire page
          blobinfo->row_index = cur_row_index; // the blob's row index within the entire page
          blobinfo->word_index = k; // the blob's word index within its row
          blobinfo->recognized_blob = recblob; // shallow copy, remember to NULLIFY on destruction
          blobinfo->row_has_valid = row_has_valid;
          blobinfo->is_italic = is_italic;
          if(blobinfo->wordchoice() != NULL) {
            blobinfo->certainty = blobinfo->wordchoice()->certainty();
            if(strcmp(blobinfo->wordstr(), "=") == 0
                || strcmp(blobinfo->wordstr(), "i") == 0) {
              blobinfo->validword = true;
            }
            else
              blobinfo->validword = api->IsValidWord(blobinfo->wordstr());
          }
          // Now to insert the blobinfo object into the BlobInfoGrid
          InsertBBox(true, true, blobinfo);
          // insert a pointer to the blob into the ROW_INFO_BLOBS object for this row
          addBlobToLastRowWord(blobinfo);
          blob_it.forward();
        }
        wordresit.forward();
      }
      rowresit.forward();
      ++cur_row_index;
    }
    bres_it.forward();
  }
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
    BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
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
      BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "BLOBNBOX at the following coordinates:\n";
        M_Utils::dispBoxCoords(b);
        cout << "overlaps something in the blobinfogrid at the following coordinates:\n";
        TBOX t = blinfo->bounding_box();
        BOX* blb = M_Utils::tessTBoxToImBox(&t, img);
        M_Utils::dispBoxCoords(blb);
        boxDestroy(&blb);
      }
      boxDestroy(&b);
#endif
    }
    else {
#ifdef DBG_INFO_GRID
      BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "BLOBNBOX at the following coordinates:\n";
        M_Utils::dispBoxCoords(b);
        cout << "doesn't overlap anything\n";
      }
      boxDestroy(&b);
#endif
      blinfo = new BLOBINFO(box);
      blinfo->original_part = blob->owner();
      blinfo->reinserted = true;
      InsertBBox(true, true, blinfo); // insert the new blobinfo object into the grid
    }
    if(blinfo->unrecognized_blobs == NULL)
      blinfo->unrecognized_blobs = new BLOBNBOX_LIST();
    C_BLOB* newcblob = C_BLOB::deep_copy(blob->cblob());
    BLOBNBOX* blobcpy = new BLOBNBOX(newcblob);
    blinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, blobcpy);
#ifdef DBG_INFO_GRID
    b = M_Utils::getBlobBoxImCoords(blobcpy, img);
    if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
      cout << "BLOBNBOX at the following coordinates:\n";
      M_Utils::dispBoxCoords(b);
      cout << "should be added to tbe BLOBNBOX list for the blobinfo element at the following coordinates:\n";
      TBOX t = blinfo->bounding_box();
      BOX* blb = M_Utils::tessTBoxToImBox(&t, img);
      M_Utils::dispBoxCoords(blb);
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
    BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
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
      BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
      if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
        cout << "on second pass, blobnbox overlaps blobinfo element at: \n";
        TBOX t = blinfo->bounding_box();
        BOX* blb = M_Utils::tessTBoxToImBox(&t, img);
        M_Utils::dispBoxCoords(blb);
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
        BOX* b = M_Utils::getBlobBoxImCoords(blob, img);
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
              BOX* b = M_Utils::tessTBoxToImBox(&tbox, img);
              M_Utils::dispBoxCoords(b);
              boxDestroy(&b);
#endif
              if(curblob->bounding_box() == blinfo->bounding_box())
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
        WERD_RES* werdres = blinfo->werdres;
        WORD_INFO* wordinfo = blinfo->wordinfo;
        int block_index = blinfo->block_index;
        int row_index = blinfo->row_index; // the row to assign any new blobs created here
        int word_index = blinfo->word_index;
        float certainty = blinfo->certainty;
        bool row_has_valid = blinfo->row_has_valid;
        bool is_italic = blinfo->is_italic;
        int replace_index = -1;
        BlobIndices replacement_indices = removeAllBlobOccurrences(blinfo);
        RemoveBBox(blinfo); // this doesn't modify blinfo pointer, just removes it from the grid...
        bblist = blinfo->copyBlobNBoxes();
        delete blinfo;
        blinfo = NULL;

        BLOBNBOX_IT bbit(bblist);
        bbit.move_to_first();
        for(int i = 0; i < bblist->length(); i++) {
          BLOBNBOX* bbox = bbit.data();
#ifdef DBG_INFO_GRID
          BOX* b = M_Utils::getBlobBoxImCoords(bbox, img);
          if(b->y == dbgBLOBNBOXtop && b->x == dbgBLOBNBOXleft) {
            cout << "new blob info element being created for BLOBNBOX at\n";
            M_Utils::dispBoxCoords(b);
          }
          boxDestroy(&b);
#endif
          BLOBNBOX* newbbox = new BLOBNBOX(*bbox);
          BLOBINFO* newblinfo = new BLOBINFO(newbbox->bounding_box());
          newblinfo->original_part = bbox->owner();
          newblinfo->werdres = werdres;
          newblinfo->wordinfo = wordinfo;
          newblinfo->block_index = block_index;
          newblinfo->row_index = row_index;
          newblinfo->word_index = word_index;
          newblinfo->certainty = certainty;
          newblinfo->reinserted = true;
          newblinfo->row_has_valid = row_has_valid;
          newblinfo->is_italic = is_italic;
          newblinfo->unrecognized_blobs = new BLOBNBOX_LIST();
          newblinfo->unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, newbbox);
          newblinfo->dbgjustadded = true;
          // insert new shallow copy replacement back into each of the wordinfo
          // objects from which the "parent" was removed
          insertBlobReplacements(newblinfo, replacement_indices, i);
          InsertBBox(true, true, newblinfo); // insert into the grid
          bbit.forward();
        }
        if(bblist != NULL) {
          bblist->clear();
          delete bblist;
          bblist = NULL;
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
  M_Utils::waitForInput();
#endif
}



// here a sentence is simply any group of one or more words starting with
// a capital letter, valid first word, and ending with a period or question mark.
void BlobInfoGrid::findSentences() {
  BlobInfoGridSearch bigs(this);
  // Determine where the sentences are relative to each row
  // use the api to figure out if words are valid
  //dbgDisplayRowText();
  // walk through the rows
  bool sentence_found = false;
  for(int i = 0; i < rows.length(); ++i) {
    // walk through the words on each row
    ROW_INFO* rowinfo = rows[i];
    GenericVector<WORD_INFO*> words = rowinfo->wordinfovec;
    for(int j = 0; j < words.length(); ++j) {
      if(words[j] == NULL)
        continue;
      if(words[j]->wordstr() == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(!sentence_found) { // looking for the start of a sentence
        if(isupper(wordstr[0]) && islower(wordstr[1])) {
          // see if the uppercase word is valid or not based on the api
          if(api->IsValidWord(wordstr)) {
            // found the start of a sentence!!
            Sentence* sentence = new Sentence;
            sentence->start_line_num = i; // start row index
            sentence->startwrd_index = j;
            recognized_sentences.push_back(sentence);
            sentence_found = true;
          }
        }
      }
      else {  // looking for the end of the current sentence
        // for now I'll assume a '.' or '?' is the end of the sentence
        // (also I'll count the last character on the page as the end
        // of a sentence if the page ends while a sentence ending is
        // still being looked for)
        char lastchar = wordstr[strlen(wordstr) - 1];
        if((lastchar == '.') || (lastchar == '?') ||
            ((rows.length() == (i+1))
                && (words.length() == (j+1)))) {
          // found the end of a sentence!!
          Sentence* sentence = (Sentence*)recognized_sentences.back();
          sentence->end_line_num = i;
          sentence->endwrd_index = j;
          copyRowTextIntoSentence(sentence);
          sentence_found = false;
        }
      }
    }
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
  M_Utils::waitForInput();
#endif

  // Now walk through the rows and words again, this time assigning all of the
  // blobs to the sentence to which they belong.
  int sentence_index = 0;
  for(int i = 0; i < rows.length(); ++i) {
    ROW_INFO* rowinfo = rows[i];
    GenericVector<WORD_INFO*> words = rowinfo->wordinfovec;
    for(int j = 0; j < words.length(); ++j) {
      WORD_INFO* wordinfo = words[j];
      // find which sentence this word belongs to if any based on the
      // current row and word indices.
      int sentence_index = findSentence(i, j);
      if(sentence_index < 0) {
        continue;
      }
      GenericVector<BLOBINFO*> blobs = wordinfo->blobs;
      for(int k = 0; k < blobs.length(); ++k)
        blobs[k]->sentenceindex = sentence_index;
    }
  }
#ifdef DBG_INFO_GRID
  sentence_sv = MakeWindow(100, 100, "BlobInfoGrid after getting the sentences");
  DisplayBoxes(sentence_sv);
  M_Utils::waitForInput();
#endif
  getSentenceRegions();
}


void BlobInfoGrid::getSentenceRegions() {

#ifdef DBG_INFO_GRID_S
  int dbgsentence = -1;
#endif
  // 1. Assign each sentence to a boxarray which gives the isothetic
  //    region corresponding to the sentence (each box is a line of
  //    that sentence)
  for(int j = 0; j < recognized_sentences.length(); j++) {
    Sentence* cursentence = recognized_sentences[j];
    const int startline = cursentence->start_line_num;
    const int endline = cursentence->end_line_num;
    const int numlines = endline - startline + 1;
#ifdef DBG_INFO_GRID_S
    if(j == dbgsentence) {
      cout << "about to find line boxes for sentence " << j << ":\n";
      cout << recognized_sentences[j]->sentence_txt << endl;
      cout << "the lines of this sentence are " << startline << "-" << endline << endl;
      M_Utils::waitForInput();
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
        BOX* blobbox = M_Utils::getBlobInfoBox(blob, img);
        int blobline = blob->row_index;
        if(blobline < startline || blobline > endline) {
          // blob was incorrectly assigned to a sentence
          cout << "ERROR: A blob was incorrectly assigned to a sentence (ignoring for now)\n";
          boxDestroy(&blobbox);
          continue;
        }
#ifdef DBG_INFO_GRID_S
        if(j == dbgsentence) {
          cout << "the displayed blob belongs to the sentence\n";
          M_Utils::dispHlBlobInfoRegion(blob, img);
          if(blob->wordstr() != NULL)
            cout << "it belongs to the following word: " << blob->wordstr() << endl;
          cout << "it belongs to following line #: " << blobline << endl;
          cout << "the blob's coords are:\n";
          M_Utils::dispBoxCoords(blobbox);
          M_Utils::waitForInput();
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

  // TODO: Decide whether or not to keep this... Likely is no longer applicable...
  // Reiterate through the grid to make sure each blob is assigned
  // to the sentence to which it belongs. If a blob's bounding box
  // intersects with a line of a sentence then it should belong to
  // that sentence (some blobs may have been missed during initial
  // assignment).
/*  BLOBINFO* curblob = NULL;
  BlobInfoGridSearch bigs(this);
  bigs.StartFullSearch();
  while((curblob = bigs.NextFullSearch()) != NULL) {
    // if it's already been assigned a sentence then move on
    if(curblob->sentenceindex != -1)
      continue;
    Box* bbox = M_Utils::getBlobInfoBox(curblob, img);
    bool found = false;
    for(int j = 0; j < recognized_sentences.length(); ++j) {
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
  }*/

#ifdef DBG_INFO_GRID_S
#ifdef DBG_INFO_GRID_SHOW_SENTENCE_REGIONS
  // for debugging, color the blobs for each sentence region and display the results
  // while showing the contents of the sentence on the terminal. do one at
  // a time requiring user input to continue in between displaying each sentence
  bool displayon = true;
  bool showlines = false; // if showlines is false then highlights the blobs
                          // belonging to each sentence, otherwise highlights
                          // the lines belonging to each sentence
  LayoutEval::Color color = LayoutEval::RED;
  Pix* dbgim = pixCopy(NULL, img);
  dbgim = pixConvertTo32(dbgim);
  static int sentencedbgimnum = 1;
  string sentencedbgname = (string)"SentenceDBG"
      + Basic_Utils::intToString(sentencedbgimnum++);
  string sentencefilename = sentencedbgname + ".txt";
  ofstream sentencedbgfile(sentencefilename.c_str());
  if(!sentencedbgfile.is_open()) {
    cout << "ERROR: Could not open debug file for writing in "
         << sentencefilename << endl;
    exit(EXIT_FAILURE);
  }
  for(int j = 0; j < recognized_sentences.length(); ++j) {
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
      M_Utils::waitForInput();
    }
    if(showlines) {
      for(int k = 0; k < cursentencelines->n; k++) {
        BOX* sentenceline = boxaGetBox(cursentencelines, k, L_CLONE);
        M_Utils::dispBoxCoords(sentenceline);
        Lept_Utils::fillBoxForeground(dbgim, sentenceline, color);
        if(j == dbgsentence) {
          cout << "highlighting blobs in line " << k + cursentence->start_line_num
               << " of sentence " << j << endl;
          cout << "here is the line's text:\n";
          cout << rows[k+cursentence->start_line_num]->getRowText() << endl;
          if(displayon) {
            pixDisplay(dbgim, 100, 100);
            M_Utils::waitForInput();
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
          BOX* curbox = M_Utils::getBlobInfoBox(curblob, img);
          Lept_Utils::fillBoxForeground(dbgim, curbox, color);
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
      M_Utils::waitForInput();
    }
    color = (color == LayoutEval::GREEN) ? LayoutEval::RED
        : (LayoutEval::Color)(color + 1);
  }
  pixDestroy(&dbgim);
#endif
#endif
}

void BlobInfoGrid::initRowInfo(ROW_RES* rowres) {
  ROW_INFO* row = new ROW_INFO;
  row->rowres = rowres;
  rows.push_back(row);
}

bool BlobInfoGrid::findValidWordOnRow(ROW_INFO* rowinfo) {
  WERD_RES_LIST* wordreslist = rowinfo->words;
  WERD_RES_IT wordresit1(wordreslist);
  wordresit1.move_to_first();
  bool row_has_valid = false;
  for(int k = 0; k < wordreslist->length(); k++) {
    WERD_RES* wordres = wordresit1.data();
    WERD_CHOICE* wordchoice = wordres->best_choice;
    if(wordchoice != NULL) {
      const char* wrd = wordchoice->unichar_string().string();
      if(api->IsValidWord(wrd)) {
        rowinfo->has_valid_word = true;
        return true;
      }
    }
    wordresit1.forward();
  }
  return false;
}

void BlobInfoGrid::addWordToLastRowInfo(WERD_RES* word) {
  ROW_INFO* rowinfo = rows.back();
  WORD_INFO* wordinfo = new WORD_INFO;
  wordinfo->wordres = word;
  wordinfo->rowinfo = rowinfo; // pointer to parent
  rowinfo->push_back_wordinfo(wordinfo);
}

void BlobInfoGrid::addBlobToLastRowWord(BLOBINFO* blob) {
  ROW_INFO* rowinfo = rows.back();
  WORD_INFO* wordinfo = rowinfo->get_wordinfo_last();
  wordinfo->blobs.push_back(blob);
}

BlobIndices BlobInfoGrid::removeAllBlobOccurrences(BLOBINFO* blob) {
  // walk through the page removing all occurrences of the given blob
  // and storing the index of each occurrence.
  BlobIndices indices;
  // for now I'll assume that the blob can only possibly appear on the
  // current row and the ones above and below it. if there's still segmentation
  // faults then I will need to expand the search window or take some other
  // precaution.
  const int blobrowindex = blob->row_index;
  const int startrowindex = (blobrowindex > 0) ? (blobrowindex-1) : 0;
  const int endrowindex = (blobrowindex < (rows.length()-1))
      ? (blobrowindex+1) : blobrowindex;
  for(int i = startrowindex; i <= endrowindex; ++i) {
    ROW_INFO* cur_row = rows[i];
    GenericVector<WORD_INFO*> words = cur_row->wordinfovec;
    for(int j = 0; j < words.length(); ++j) {
      WORD_INFO* word = words[j];
      int blob_removed_index = word->removeBlob(blob);
      if(blob_removed_index != -1) {
        BlobIndex blbidx;
        blbidx.blobindex = blob_removed_index;
        blbidx.wordindex = j;
        blbidx.rowindex = i;
        indices.push_back(blbidx);
        --j; // this is the worst kind of bug.. took > 3 hours to realize I needed this...
             // needed because a duplicate blob often occurs more than once in the same
             // word. Thus need to stay on the same word until all duplicates removed. could
             // have used a separate while loop instead but this works fine.
      }
    }
  }
  return indices;
}

void BlobInfoGrid::insertBlobReplacements(BLOBINFO* blob,
    const BlobIndices& replacement_indices, const int& replacement_num) {
  for(int i = 0; i < replacement_indices.length(); ++i) {
    BlobIndex r_index = replacement_indices[i];
    const int rowindex = r_index.rowindex;
    const int wordindex = r_index.wordindex;
    const int blobindex = r_index.blobindex;
    ROW_INFO* rowinfo = rows[rowindex];
    WORD_INFO* wordinfo = rowinfo->wordinfovec[wordindex];
    int wordsize = wordinfo->blobs.length();
    int newindex = blobindex + replacement_num;
    if(newindex < wordsize)
      wordinfo->blobs.insert(blob, newindex);
    else
      wordinfo->blobs.push_back(blob);
  }
}

void BlobInfoGrid::dbgDisplayRowText() {
  for(int i = 0; i < rows.length(); ++i) {
    ROW_INFO* rowinfo = rows[i];
    GenericVector<WORD_INFO*> words = rowinfo->wordinfovec;
    for(int j = 0; j < words.length(); ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      cout << wordstr << " ";
    }
    cout << endl;
  }
  cout << "\n------------------------------\n\n";
}

void BlobInfoGrid::copyRowTextIntoSentence(Sentence* sentence) {
  const int startln = sentence->start_line_num;
  const int endln = sentence->end_line_num;
  const int startwrd = sentence->startwrd_index;
  const int endwrd = sentence->endwrd_index;
  assert(startln > -1 && endln > -1 && startwrd > -1 && endwrd > -1);
  int charcount = 0;
  // first need to count the characters
  for(int i = startln; i <= endln; ++i) {
    ROW_INFO* rowinfo = rows[i];
    GenericVector<WORD_INFO*> words = rowinfo->wordinfovec;
    int start = 0, end = words.length() - 1;
    if(i == startln)
      start = startwrd;
    if(i == endln)
      end = endwrd;
    for(int j = start; j <= end; ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      charcount += strlen(wordstr);
      // add room for space and new line
      ++charcount;
    }
  }
  char* newsentence = new char[charcount+1]; // allocate the memory
  int charindex = 0;
  // now copy everything over
  for(int i = startln; i <= endln; ++i) {
    ROW_INFO* rowinfo = rows[i];
    GenericVector<WORD_INFO*> words = rowinfo->wordinfovec;
    int start = 0, end = words.length() - 1;
    if(i == startln)
      start = startwrd;
    if(i == endln)
      end = endwrd;
    for(int j = start; j <= end; ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      for(int k = 0; k < strlen(wordstr); ++k) {
        newsentence[charindex++] = wordstr[k];
      }
      if(j != end)
        newsentence[charindex++] = ' ';
      else
        newsentence[charindex++] = '\n';
    }
  }
  newsentence[charcount] = '\0';
  sentence->sentence_txt = newsentence; // finally set the datastructure
}

int BlobInfoGrid::findSentence(const int& rowindex, const int& wordindex) {
  for(int i = 0; i < recognized_sentences.length(); ++i) {
    Sentence* cursentence = recognized_sentences[i];
    const int s_start_row = cursentence->start_line_num;
    const int s_end_row = cursentence->end_line_num;
    const int s_start_word = cursentence->startwrd_index;
    const int s_end_word = cursentence->endwrd_index;
    if(rowindex < s_start_row || rowindex > s_end_row)
      continue;
    else if(rowindex == s_start_row && rowindex != s_end_row) {
      if(wordindex >= s_start_word)
        return i;
    }
    else if(rowindex == s_end_row && rowindex != s_start_row) {
      if(wordindex <= s_end_word)
        return i;
    }
    else if(rowindex == s_end_row && rowindex == s_start_row) {
      if(wordindex >= s_start_word && wordindex <= s_end_word)
        return i;
    }
    else if(rowindex > s_start_row && rowindex < s_end_row)
      return i;
  }
  return -1;
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
  if(bb->wordstr() != NULL) {
    cout << bb->wordstr() << endl;
    if(bb->validword)
      cout << "the blob is in a valid word!\n";
  }
  else
    cout << "(NULL)\n";
  cout << "The tesseract boundingbox: \n";
  TBOX t = bb->bounding_box();
  M_Utils::dispTBoxCoords(&t);
  if(bb->row_has_valid) {
    cout << "The blob is on a row with atleast one valid word!\n";
    cout << "The baseline for the blob is at y = " << bb->row()->base_line(bb->left()) << endl;
    cout << "The blob's bottom y is at " << bb->bottom() << endl;
    //ScrollView* baselineview = new ScrollView("baseline", 300, 100,
    //                        img->w, img->h, img->w, img->h, false);
    //bb->row->plot_baseline(baselineview, ScrollView::GREEN);
  }
  else
    cout << "The blob is on a row with no valid words!\n";
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
  if(bb->row() != NULL) {
    string rowstr = bb->rowinfo()->getRowText();
    if(rowstr.empty())
      cout << "blob belongs to a row that didn't have any recognized text\n";
    else
      cout << "blob belongs to row recognized wtih the follwing text:\n" << rowstr << endl;
  }
  else
    cout << "\nthe blob you clicked on was not assigned a text line\n";
  if(bb->sentenceindex == -1)
    cout << "\nthe blob you clicked on was not assigned a sentence\n......\n";
  else if(recognized_sentences[bb->sentenceindex] != NULL) {
    cout << "\nthe blob you clicked on belongs to the following sentence:\n"
         << recognized_sentences[bb->sentenceindex]->sentence_txt << endl << "......\n";
  }
  else
    cout << "the blob you clicked belongs to sentence number " << bb->sentenceindex << ", which is NULL\n......\n";
#ifdef SHOW_BLOB_WINDOW
    M_Utils::dispHlBlobInfoRegion(bb, img);
    M_Utils::dispBlobInfoRegion(bb, img);
#endif
    if(dbgfeatures)
      dbgDisplayBlobFeatures(bb);
}

void BlobInfoGrid::dbgDisplayBlobFeatures(BLOBINFO* blob) {
  if(!blob->features_extracted) {
    cout << "ERROR: Feature extraction hasn't been carried out on the blob\n";
    exit(EXIT_FAILURE);
  }
  vector<double> features = blob->features;
  if(features.size() != featformat.length())
    cout << "features.size(): " << features.size() << ". featformat.length(): "
         << featformat.length() << endl;
  assert(features.size() == featformat.length());
  cout << "Displaying extracted blob features\n------------------\n";
  for(int i = 0; i < featformat.length(); i++)
    cout << featformat[i] << ": " << features[i] << endl;
#ifdef DBG_BASELINE
  if(!blob->row_has_valid) {
    cout << "blob doesn't belong to a valid row\n";
    return;
  }
  int rowindex = blob->row_index;
  cout << "rowindex: " << rowindex << endl;
  assert(rowindex != -1);
  assert(rows[rowindex]->rowid == blob->rowinfo()->rowid);
  cout << "\n\nrow's average baseline distance: " << rows[rowindex]->avg_baselinedist << endl;
  cout << "blob's distance from baseline: " << blob->dist_above_baseline << endl;
  cout << "blob's bottom: " << blob->bottom() << endl;
  cout << "row's baseline at blob location: " << rows[rowindex]->row()->base_line(blob->centerx()) << endl;
  cout << "row's height: " << rows[rowindex]->row()->bounding_box().height() << endl;
#endif
  cout << "------------------------------------------------------\n";
}

void BlobInfoGrid::setFeatExtFormat(const string& trainpath,
    const string& featextname, const int numfeat) {
  string formatpath = trainpath + (string)"../../FeatureVectorFormats/"
      + featextname + (string)"_Format";
  ifstream s(formatpath.c_str());
  if(!s.is_open()) {
    cout << "ERROR: The debug feature extraction formatting file at " << formatpath
         << " couldn't be found >:-[\n";
    exit(EXIT_FAILURE);
  }
  int maxlen = 150;
  char line[maxlen];
  while(!s.eof()) {
    s.getline(line, maxlen);
    if(*line == '\0')
      continue;
    featformat.push_back((string)line);
  }
  assert(numfeat == featformat.length());
  dbgfeatures = true;
}

} // end namespace tesseract
