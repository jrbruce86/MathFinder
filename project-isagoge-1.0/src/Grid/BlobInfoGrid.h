/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:	  BlobInfoGrid.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Oct 4, 2013 12:42:18 PM
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
#ifndef BLOBINFOGRID_H
#define BLOBINFOGRID_H

#include <M_Utils.h>
#include <colpartitiongrid.h>
#include <baseapi.h>
#include <Basic_Utils.h>
#include <NGramRanker.h>

class WERD_CHOICE;

namespace tesseract {

class Tesseract;
class ColPartition;
class ColPartitionGrid;
class ColPartitionSet;

// Each blobinfo object has the following:
// 1. A pointer to the original ColPartition in which the blob is contained after
//    all previous document layout analysis.
// 2. A single C_BLOB which, in some cases may be two or more connected components
//    combined into one symbol (i.e. in the case of an "i" or an "="). If this is
//    empty then that means that nothing was recognized, the BLOBNBOX(es) in (3) are
//    all the information that is available in this case.
// 3. Pointers to the BLOBNBOX(es) corresponding to the C_BLOB. The BLOBNBOX(es) are
//    what is in the original ColPartitionGrid provided to this function after page
//    layout analysis, and what this module is expected to return a ColPartitionGrid
//    consisting of. The C_BLOB's on the other hand are the result of using the API
//    to recognize all normal text in partitions (so that basic information like that
//    the dot in an "i" is unimportant is quickly and painlessly ascertained. Every
//    BLOBINFO object will have at least one BLOBNBOX, but may not have a C_BLOB as
//    explained in (2) as that would mean a recognition result was ascertained. A
//    BLOBINFO object may also have multiple BLOBNBOXes. The total BLOBNBOX objects
//    over the entire grid should amount to the total number of BLOBNBOXes in the
//    BlobGrid.
// 4. The WERDCHOICE to which the blob belongs. this gives the word the blob belongs to
//    (if the blob was recognized as part of a word) and also a certainty for the
//    word which is set to the lowest certainty for all the blobs recognized in the
//    word. This will be NULL for the case where no recognition result was ascertained.
// 5. A pointer to the sentence to which the blob's word belongs (if it belongs to a
//    sentence)
class BLOBINFO;
ELISTIZEH (BLOBINFO)
class BLOBINFO:public ELIST_LINK {
 public:
  // constructor sets the box's grid coordinates
  BLOBINFO(TBOX boxgridc) : box(boxgridc), original_part(NULL),
           recognized_blob(NULL), unrecognized_blobs(NULL),
           sentence(NULL), word(NULL), validword(false), dbgjustadded(false),
           wordstr(NULL), linestrindex(-1), sentenceindex(-1), linewordindex(-1),
           line_rel_colpart(-1), has_sup(false), has_sub(false), is_sup(false),
           is_sub(false), row_has_valid(false), row(NULL),
           cosbabp(0), cosbabp_processed(false), ismathword(false),
           is_italic(false), blobindex_inword(-1), word_lastblob(-1),
           has_nested(false), certainty((double)-20), features_extracted(false),
           dist_above_baseline((double)0), predicted_math(false) {}
  // TODO: Make sure this copies everything (if necessary)!!!
  // for now I'm just using it to copy the BLOBNBOXes and that's it
  BLOBINFO(const BLOBINFO& copy) : original_part(copy.original_part),
      recognized_blob(NULL), sentence(NULL), word(NULL),
      validword(copy.validword), box(copy.box), linestrindex(-1),
      unrecognized_blobs(NULL), dbgjustadded(false), wordstr(NULL),
      sentenceindex(-1), linewordindex(-1), line_rel_colpart(-1),
      has_sup(false), has_sub(false), is_sup(false), is_sub(false),
      row_has_valid(copy.row_has_valid), row(copy.row), cosbabp(copy.cosbabp),
      cosbabp_processed(copy.cosbabp_processed), ismathword(copy.ismathword),
      is_italic(copy.is_italic), blobindex_inword(-1), word_lastblob(-1),
      has_nested(false), certainty((double)-20), features_extracted(false),
      dist_above_baseline(copy.dist_above_baseline),
      predicted_math(copy.predicted_math) {
    // cout << "in blobinfo copy constrctor!!\n";
    unrecognized_blobs = new BLOBNBOX_LIST();
    BLOBNBOX_LIST* cpyblobs = copy.unrecognized_blobs;
    BLOBNBOX_IT bbit(cpyblobs);
    bbit.move_to_first();
    for(int i = 0; i < cpyblobs->length(); i++) {
      BLOBNBOX* cur = bbit.data();
      BLOBNBOX* newcur = new BLOBNBOX(*cur); // uses the default copy constructor
      unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, newcur);
      bbit.forward();
    }
  }

  ~BLOBINFO() {
    clear();
  }

  inline void clear() {
    if(word != NULL) {
      delete word;
      word = NULL;
    }
    if(wordstr != NULL) {
      delete [] wordstr;
      wordstr = NULL;
    }
    if(unrecognized_blobs != NULL) {
      unrecognized_blobs->clear(); // this does a deep delete I believe TODO: Confirm...
      delete unrecognized_blobs;
      unrecognized_blobs = NULL;
    }
    if(recognized_blob != NULL) {
      delete recognized_blob;
      recognized_blob = NULL;
    }
    features.clear();
  }

  ColPartition* original_part;
  C_BLOB* recognized_blob;
  BLOBNBOX_LIST* unrecognized_blobs;
  char* sentence; // the sentence to which this blob belongs
  // Best I can do is give each blob a word to which it can be associated
  // there is no way of knowing exactly what CBLOB a BLOB_CHOICE corresponds
  // to using the api. The certainty of the word (which is the same as the
  // worst certainty of all of its blobs) is used as a metric for my classifier
  WERD_CHOICE* word; // the recognized word to which this blob belongs (can be NULL)
  char* wordstr; // the recognized word to which the blob belongs
  int blobindex_inword; // the blob's index within it's word
  int word_lastblob;
  int linestrindex; // index to the BlobInfoGrid's vector of line strings to which this blob belongs
  int line_rel_colpart; // the blob's row index in relation to the tesseract colpartition to which it belongs
  int linewordindex; // the word index on the current line
  int sentenceindex; // index to the BlobInfoGrid's sentence to which this blob belongs
  bool validword; // true if the word corresponding to this blob is valid,
                  // false if it's invalid or doesn't exist
  float certainty; // # from [-20,0] indicating Tesseract's worst blob recognition confidence
                   // for the word to which this blob belongs

  ROW* row; // pointer to the row on which this blob resides assuming it resides on
            // a row with at least one valid word.. otherwise this is NULL
  double dist_above_baseline; // the blob's distance above its row's baseline

  vector<double> features; // feature vector for detection binary classifier
  bool features_extracted; // true after feature extraction is completed for this blob

  bool ismathword; // true if the blob is on a finite list of math words

  // true only if the blob belongs to an italic word
  bool is_italic;

  // subscript/superscript features
  bool has_sup; // true if this blob has a superscript
  bool has_sub; // true if this blob has a subscript
  bool is_sup; // true if blob is a superscript
  bool is_sub; // true if blob is a subscript

  bool has_nested; // the blob has one or more nested elements

  // count of stacked blobs at blob position (cosbabp) feature
  int cosbabp;
  bool cosbabp_processed;

  // true if this blob is on a row with at least one valid word
  bool row_has_valid;

  // Required accessor:
  const TBOX& bounding_box() const {
    return box;
  }

  bool predicted_math; // result of the detector

  // quick accessors for convenience
  inline inT16 left() const {
    return box.left();
  }
  inline inT16 bottom() const {
    return box.bottom();
  }
  inline inT16 right() const {
    return box.right();
  }
  inline inT16 top() const {
    return box.top();
  }
  inline inT16 centerx() const {
    inT16 w = box.width();
    inT16 l = box.left();
    return l + (w / 2);
  }
  inline inT16 centery() const {
    inT16 h = box.height();
    inT16 b = box.bottom();
    return b + (h / 2);
  }
  inline inT16 height() const {
    return box.height();
  }
  inline inT16 width() const {
    return box.width();
  }

  // Required for displaying the grid
  ScrollView::Color BoxColor() const {
    return ScrollView::WHITE;
  }

  bool dbgjustadded;

 private:
  TBOX box; // The bounding box (in Tesseract coordinates
            // relative to the entire image)
};

CLISTIZEH(BLOBINFO)

struct Line {
  int originalindex;
  int newindex;
  bool has_something;
};

typedef GridSearch<BLOBINFO, BLOBINFO_CLIST, BLOBINFO_C_IT> BlobInfoGridSearch;

// The BlobInfoGrid is designed such that it
// will be relatively easy to obtain the features I will be needing for binary
// classification.
// This Grid uses the normal coordinate space, rather than the deskewed version
// which is seen in the ColPartitionGrid. Fortunately the normal, non-deskewed
// bounding boxes can still be accessed within a BLOBNBOX by grabbing the
// C_BLOB and looking at it's bounding box. The skewed version of the image will
// be suitable for my purposes in that I make no assumptions about the nearest
// neighbors being perfectly horizontally straight in reference to each other.
class BlobInfoGrid : public BBGrid<BLOBINFO, BLOBINFO_CLIST, BLOBINFO_C_IT> {
 public:
  BlobInfoGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright);

  // Make sure this deletes everything allocated!!!
  ~BlobInfoGrid();

  inline void deleteStringVector(GenericVector<char*>& strvec) {
    for(int i = 0; i < strvec.length(); i++) {
      if(strvec[i] != NULL) {
        delete [] strvec[i];
        strvec[i] = NULL;
      }
    }
    strvec.clear();
  }

  inline void freeScrollView(ScrollView* sv) {
    if(sv != NULL) {
      delete sv;
      sv = NULL;
    }
  }

  // The functionality of preparing the grid is moved here so that
  // the MEDS module isn't so cluttered by it.
  inline void prepare(ColPartitionGrid* partgrid_,
      ColPartitionSet** bcd, Tesseract* tess_) {
    setColPart(partgrid_, bcd);
    setTessAndPix(tess_);
    // deep copy all the blobs in the partsgrid to a blobgrid
    partGridToBBGrid();
    // do language-specific recognition on all the partitions found through
    // previous layout analysis done by the tesseract framework
    recognizeColParts();
    // keep word segmentation results of language regions (recognized with
    // high confidence by language OCR) and split up regions not recognized
    // well into separate blobs. This generally preserves blobs that should
    // be combined into a single symbol (like 'i' or '=' for instance) while
    // splitting symbols improperly combined during language recognition.
    insertRemainingBlobs();
    // The next step is to separate the lines of text recognized into sentences
    findSentences();
  }

  void setColPart(ColPartitionGrid* partgrid_, ColPartitionSet** bcd) {
    part_grid = partgrid_;
    best_col_divisions = bcd;
  }
  void setTessAndPix(Tesseract* tess_) {
    tess = tess_;
    img = tess->pix_binary();
  }
  void partGridToBBGrid();

  // recognize all the colpartitions using the language OCR and then
  // insert information ascertained from this recognition into the grid
  void recognizeColParts();


  // insertRemainingBlobs():
  // To be called after recognizeColParts(), insert whatever blobs weren't
  // recognized into the blobinfogrid (these blobs are all in the bbgrid
  // initially. After this method is called, every blobinfo object in the
  // blobinfogrid will have at least one BLOBNBOX from the bbgrid corresponding
  // with the C_BLOB which came from recognizeColParts. BLOBNBOXes corresponding
  // to unrecognized regions will have no information from recognizeColParts
  // (i.e. the BLOBINFO object for such blobs will just consist of the BLOBNBOX,
  // the ColPartition it belongs to, and nothing else). If a C_BLOB covering
  // multiple BLOBNBOXes wasn't recognized as anything or is junk then I need
  // to split the corresponding BLOBINFO object into its constituent BLOBNBOXES.
  // Otherwise if it is something (i.e. an 'i' which consists of two BLOBNBOXES)
  // then keep it as is.
  void insertRemainingBlobs();

  // This method separates the recognized text into sentences while keeping
  // track of which column partition each sentence belongs to. Then iterates
  // the BlobInfoGrid adding to each BlobInfo element the sentence to which
  // it belongs based upon which column partition it belongs to.
  void findSentences();

  // Called by findsentences. Does three things:
  // 1. Assigns an array of boxes to each sentences, where each box corresponds
  //    to a line of that sentence
  // 2. Reiterates through the grid, assigning each previously unassigned blob
  //    to a sentence if it overlaps any box (found in 1.) that is part of
  //    that sentence.
  // 3. (Optionally depending on whether or not debug define is commented out)
  //    Display which blobs are assigned to which sentence for debugging or
  //    display the regions by highlighting foreground pixels.
  void getSentenceRegions();

  inline void setTessAPI(TessBaseAPI* api) {
    newapi = api;
  }

  inline Pix* getImg() {
    return img;
  }

  inline GenericVector<Sentence*> getSentences() {
    return recognized_sentences;
  }

  inline GenericVector<char*> getTextLines() {
    return recognized_lines;
  }

  inline GenericVector<ROW*> getRows() {
    return valid_rows;
  }
  inline void appendAvgBaseline(double avg_baseline) {
    valid_rows_avg_baselinedist.push_back(avg_baseline);
  }

  inline Sentence* getBlobSentence(BLOBINFO* blob) {
    int sentence_index = blob->sentenceindex;
    if(sentence_index == -1)
      return NULL;
    return recognized_sentences[sentence_index];
  }

  // I'm overriding this to help in debugging. Making it so that when you click
  // on a blob it should show the sentence or text line it belongs to
  virtual void HandleClick(int x, int y);

  // display blob's extracted features for debugging
  void dbgDisplayBlobFeatures(BLOBINFO* blob);
  void setFeatExtFormat(const string& train_path, const string& featextname,
      const int numfeat);
  GenericVector<string> featformat;
  bool dbgfeatures;

 private:
  ColPartitionGrid* part_grid; // this should remain as the original
                              // from Tesseract's document layout
                              // analysis and not be modified until
                              // we have all of our results and are
                              // ready to feed them forward for
                              // further processing by Tesseract.
  ColPartitionSet** best_col_divisions;
  BlobGrid* bbgrid; // This grid contains pointers to all BLOBNBOXes that are in
                    // the partgrid, not partitioned in any way (just a grid of
                    // all the connected components). I don't have to worry about
                    // managing the memory of these as they are managed outside
                    // the scope of my module. They are all just shallow copies.

  GenericVector<char*> recognized_text; // this holds a list of
                                        // text results, each entry being
                                        // the string corresponding to a
                                        // colpartition (the entries are
                                        // in the order that their corresponding
                                        // partitions are visited by grid search).

  GenericVector<char*> recognized_lines;        // holds list of of all the recognized text on the
                                                // page, this time split into lines
  GenericVector<int> recognized_lines_numwrds;  // the number of words on each line in
                                                // recognized_lines

  GenericVector<Sentence*> recognized_sentences; // this holds the same text as
                                                 // recognized_text/lines, but now
                                                 // split into sentences.

  GenericVector<ROW*> valid_rows; // list of the rows on the page which have atleast one valid word
  GenericVector<double> valid_rows_avg_baselinedist;

  M_Utils mutils; // static class with assorted functions
  Tesseract* tess; // the language-specific OCR engine used
  TessBaseAPI* newapi; // I instantiate a new api for recognizing normal text
                      // This api will not do any equation detection, just
                      // normal language detection. Once I have extracted
                      // all the information I need from it, it will be
                      // discarded.
  PIX* img; // the image on which all processing is carried out

  // scrollviews for debugging
  ScrollView* part_win;
  ScrollView* blobnboxgridview;
  ScrollView* rec_col_parts_sv;
  ScrollView* insertrblobs_sv;
  ScrollView* line_sv;
  ScrollView* sentence_sv;
};

} // end tesseract namespace

#endif

