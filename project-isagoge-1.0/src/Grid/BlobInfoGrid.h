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

class ROW_INFO;

class BLOBINFO;

static TBOX getBlobTBox(BLOBINFO* blob);



struct WORD_INFO {
  WORD_INFO() : rowinfo(NULL), wordres(NULL),
      sentence_start(false), sentence_end(false) {}
  ~WORD_INFO() {
    rowinfo = NULL;
    wordres = NULL;
    for(int i = 0; i < blobs.length(); ++i)
      blobs[i] = NULL;
    blobs.clear();
  }

  inline const char* wordstr() {
    if(bestchoice() != NULL)
      return bestchoice()->unichar_string().string();
    return NULL;
  }

  inline WERD_CHOICE* bestchoice() {
    return wordres->best_choice;
  }
  inline WERD* word() {
    return wordres->word;
  }

  // removes the blob from the word and returns the index
  // at which the blob resided. if the blob wasn't in the word
  // in the first place then returns -1. if remove is false
  // just returns in the index which can also be helpful
  inline int removeBlob(BLOBINFO* blob, bool remove) {
    TBOX rmbox = getBlobTBox(blob);
    int index = -1;
    for(int i = 0; i < blobs.length(); ++i) {
      TBOX curbox = getBlobTBox(blobs[i]);
      if(rmbox == curbox) {
        index = i;
        break;
      }
    }
    if(index >= 0) {
      if(remove)
        blobs.remove(index);
    }
    return index;
  }
  ROW_INFO* rowinfo;
  WERD_RES* wordres;
  GenericVector<BLOBINFO*> blobs;
  bool sentence_start; // true if this word is at the start of a sentence
  bool sentence_end; // true if this word is at the end of a sentence
};

//enum ROW_TYPE {NORMAL, ABNORMAL};
struct ROW_INFO {
  ROW_INFO() : rowres(NULL), has_valid_word(false),
      avg_baselinedist((double)0), words(NULL), rowid(-1) {}

  ~ROW_INFO() {
    rowres = NULL;
    for(int i = 0; i < wordinfovec.length(); ++i) {
      if(wordinfovec[i] != NULL) {
        delete wordinfovec[i];
        wordinfovec[i] = NULL;
      }
    }
    wordinfovec.clear();
  }

  string getRowText() {
    string str;
    for(int i = 0; i < wordinfovec.length(); ++i) {
      if(wordinfovec[i]->wordstr() != NULL)
        str += (string)((wordinfovec[i]->wordstr()) + (string)" ");
    }
    return str;
  }

  // this provides the bounding box, baseline, and various other
  // useful information about the row as found in the tesseract api.
  inline ROW* row() {
    return rowres->row;
  }

  // unmodified words found directly by the tesseract api
  WERD_RES_LIST* words;

  // list of the same words as found by tesseract api but also
  // holding pointers to all of the BLOBINFO elements contained in that
  // word.
  inline void push_back_wordinfo(WORD_INFO* wordinfo) {
    wordinfovec.push_back(wordinfo);
  }
  inline WORD_INFO* get_wordinfo_last() {
    return wordinfovec.back();
  }
  GenericVector<WORD_INFO*> wordinfovec;

  ROW_RES* rowres; // tesseract api information on the row
  bool has_valid_word;
  double avg_baselinedist; // average distance of a blob from
                           // the row's baseline (only non-zero if
                           // row has at leat one valid word.
  int rowid;
};


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
//    to recognize all normal text on the page (so that basic information like
//    the dot in an "i" is quickly and painlessly ascertained). Every
//    BLOBINFO object will have at least one BLOBNBOX, but may not have a C_BLOB as
//    explained in (2) if a recognition result was not ascertained. A
//    BLOBINFO object may also have multiple BLOBNBOXes. The total BLOBNBOX objects
//    over the entire BlobInfoGrid should amount to the total number of BLOBNBOXes in the
//    BlobGrid.
// 4. The WERDCHOICE to which the blob belongs. this gives the word the blob belongs to
//    (if the blob was recognized as part of a word) and also a certainty for the
//    word which is set to the lowest certainty for all the blobs recognized in the
//    word. This will be NULL for the case where no recognition result was ascertained.
// 5. Pointer to the WORD_INFO object to which this blob belongs. This gives the blob's
//    location on the row it belongs to only if it belongs to a word and row as found during
//    Tesseract's page recognition.
// 6. A pointer to the sentence to which the blob's word belongs (if it belongs to a
//    sentence)
class BLOBINFO;
ELISTIZEH (BLOBINFO)
class BLOBINFO:public ELIST_LINK {
 public:
  // constructor sets the box's grid coordinates
  BLOBINFO(TBOX boxgridc) : box(boxgridc), original_part(NULL),
           recognized_blob(NULL), unrecognized_blobs(NULL),
           werdres(NULL), validword(false), dbgjustadded(false),
           sentenceindex(-1), word_index(-1), reinserted(false),
           row_index(-1), block_index(-1), has_sup(false), has_sub(false), is_sup(false),
           is_sub(false), row_has_valid(false), wordinfo(NULL),
           cosbabp(0), cosbabp_processed(false), ismathword(false),
           is_italic(false),
           has_nested(false), certainty((double)-20), features_extracted(false),
           dist_above_baseline((double)0), predicted_math(false) {}

  // Copy construtor is simply not used and avoided here so was not implemented

  // make copies of BLOBNBOXes and return them
  BLOBNBOX_LIST* copyBlobNBoxes() {
    BLOBNBOX_LIST* new_unrecognized_blobs = new BLOBNBOX_LIST();
    BLOBNBOX_LIST* cpyblobs = unrecognized_blobs;
    BLOBNBOX_IT bbit(cpyblobs);
    bbit.move_to_first();
    for(int i = 0; i < cpyblobs->length(); ++i) {
      BLOBNBOX* cur = bbit.data();
      BLOBNBOX* newcur = new BLOBNBOX(*cur); // uses the default copy constructor
      new_unrecognized_blobs->add_sorted(SortByBoxLeft<BLOBNBOX>, true, newcur);
      bbit.forward();
    }
    return new_unrecognized_blobs;
  }

  ~BLOBINFO() {
    clear();
  }

  inline void clear() {
    werdres = NULL;
    original_part = NULL;
    recognized_blob = NULL;
    if(unrecognized_blobs != NULL) {
      unrecognized_blobs->clear();
      delete unrecognized_blobs;
      unrecognized_blobs = NULL;
    }
    wordinfo = NULL;
    features.clear();
  }

  ColPartition* original_part;
  C_BLOB* recognized_blob;
  BLOBNBOX_LIST* unrecognized_blobs;
  // Best I can do is give each blob a word to which it can be associated
  // there is no way of knowing exactly what CBLOB a BLOB_CHOICE corresponds
  // to using the api. The certainty of the word (which is the same as the
  // worst certainty of all of its blobs) is used as a metric for my classifier
  WERD_RES* werdres;

  // this gives access the word's specific location within it's row, the row it is on
  // as well as all of the other words on the row and all of the other blobs within
  // the word that this blob is in
  WORD_INFO* wordinfo;

  inline ROW_INFO* rowinfo() {
    if(wordinfo == NULL)
      return NULL;
    return wordinfo->rowinfo;
  }

  inline ROW* row() {
    if(wordinfo == NULL)
      return NULL;
    return wordinfo->rowinfo->row();
  }

  // the recognized word to which this blob belongs (can be NULL)
  inline WERD_CHOICE* wordchoice() {
    if(werdres == NULL)
      return NULL;
    return werdres->best_choice;
  }

  inline WERD* word() {
    if(werdres == NULL)
      return NULL;
    return werdres->word;
  }

  // TODO: Add a method to BlobInfoGrid which sorts all of the blobs
  //       in each word in left to right fashion and then replace these
  //       inefficient functions with simply grabbing the right and left
  //       most element in the sorted vector.
  // true if this blob is the right-most blob in its word, if this
  // blob isn't the rightmost or it doesn't belong to a word then
  // returns false.
  inline bool isRightmostInWord() {
      if(wordinfo == NULL)
        return false;
    const TBOX thisbox = box;
    GenericVector<BLOBINFO*> blobs = wordinfo->blobs;
    for(int i = 0; i < blobs.length(); ++i) {
      const TBOX otherbox = blobs[i]->bounding_box();
      if(otherbox == thisbox)
        continue;
      if(otherbox.right() > thisbox.right())
        return false;
    }
    return true;
  }
  // returns true if this blob is the left-most in its word
  inline bool isLeftmostInWord() {
    if(wordinfo == NULL)
      return false;
    const TBOX thisbox = box;
    GenericVector<BLOBINFO*> blobs = wordinfo->blobs;
    for(int i = 0; i < blobs.length(); ++i) {
      const TBOX otherbox = blobs[i]->bounding_box();
      if(otherbox == thisbox)
        continue;
      if(otherbox.left() < thisbox.left())
        return false;
    }
    return true;
  }

  // the recognized word to which the blob belongs
  inline const char* wordstr() {
    if(!wordchoice())
      return NULL;
    return wordchoice()->unichar_string().string();
  }

  int block_index; // the blob's block index with respect to the entire page
  int row_index; // the blob's row index with respect to the entire page
  int word_index; // the word index on the current row
  int sentenceindex; // index to the BlobInfoGrid's sentence to which this blob belongs
  bool validword; // true if the word corresponding to this blob is valid,
                  // false if it's invalid or doesn't exist
  float certainty; // # from [-20,0] indicating Tesseract's worst blob recognition confidence
                   // for the word to which this blob belongs

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

  bool reinserted; // true if the blob was inserted after initial page recognition
  bool dbgjustadded;

 private:
  TBOX box; // The bounding box (in Tesseract coordinates
            // relative to the entire image)
};

CLISTIZEH(BLOBINFO)

TBOX getBlobTBox(BLOBINFO* blob) {
  return blob->bounding_box();
}

struct BlobIndex {
  int rowindex;
  int wordindex;
  int blobindex;
};
typedef GenericVector<BlobIndex> BlobIndices;

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
    assert(api != NULL); // A freshly allocated api is needed to have been
                         // provided by the owner of this class by calling
                         // the setTessApi() method defined below. This api
                         // is entirely owned by this class.
    setColPart(partgrid_, bcd);
    setTessAndPix(tess_);
    // deep copy all the blobs in the partsgrid to a blobgrid
    partGridToBBGrid();
    // do language-specific recognition on all the partitions found through
    // previous layout analysis done by the tesseract framework
    recognizePage();
    // keep word segmentation results of language regions (recognized with
    // high confidence by language OCR) and split up regions not recognized
    // well into separate blobs. This generally preserves blobs that should
    // be combined into a single symbol (like 'i' or '=' for instance) while
    // splitting symbols improperly combined during language recognition.
    insertRemainingBlobs();
    // The next step is to separate the lines of text recognized into sentences
    findSentences();
  }

  inline void setColPart(ColPartitionGrid* partgrid_, ColPartitionSet** bcd) {
    part_grid = partgrid_;
    best_col_divisions = bcd;
  }
  inline void setTessAndPix(Tesseract* tess_) {
    tess = tess_;
    img = tess->pix_binary();
  }

  void partGridToBBGrid();

  // Recognizes the entire page, organizes the results into a list of rows
  // holding all of their recognized blobs grouped into words.
  void recognizePage();

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

  // Called by findsentences. Does two things:
  // 1. Assigns an array of boxes to each sentences, where each box corresponds
  //    to a line of that sentence
  // 2. (Optionally depending on whether or not debug define is commented out)
  //    Display which blobs are assigned to which sentence for debugging or
  //    display the regions by highlighting foreground pixels.
  void getSentenceRegions();

  inline void setTessAPI(TessBaseAPI* api_) {
    api = api_;
  }

  inline Pix* getImg() {
    return img;
  }

  inline GenericVector<Sentence*> getSentences() {
    return recognized_sentences;
  }

  inline GenericVector<ROW_INFO*> getRows() {
    return rows;
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
  // allocates ROW_INFO corresponding to the given ROW_RES. Appends the
  // resulting ROW_INFO pointer to this class's "rows" vector.
  void initRowInfo(ROW_RES* rowres);
  // iterates the words on the given row and if any valid word is found
  // then sets the row's has_valid_word flag to true
  bool findValidWordOnRow(ROW_INFO* row);
  // adds the given word to the last ROW_INFO element on the rows vector
  void addWordToLastRowInfo(WERD_RES* word);
  // adds the given blob the last WORD_INFO element on the last ROW_INFO element
  // of the rows vector
  void addBlobToLastRowWord(BLOBINFO* blob);
  // removes all shallow copies of the given blob from the ROW_INFO vector. A blob
  // may occur multiple times and thus if it is deleted all pointers to it must
  // be deleted as well in order to prevent dangling pointers. Further, if a blob
  // is being replaced with multiple children blobs then the replacement must occur
  // for all indices at which a shallow copy of the blob resides. The BlobIndices
  // at which the shallow copy/copies were found in returned so that the children
  // blobs can be inserted back into the appropriate positions on the page.
  BlobIndices removeAllBlobOccurrences(BLOBINFO* blob, bool remove);
  // inserts shallow copies of given blob into the ROW_INFO vector for all of the
  // provided replacement indices. this complements the removeAllBlobOccurrences
  // method in that it is used to reinsert the "children" of the "parent" blob
  // that was removed by it. The removed blob is called a "parent" because it is
  // being broken up into two or more smaller blobs ("children") to facilitate
  // more detailed analysis.
  void insertBlobReplacements(BLOBINFO* blob, const BlobIndices& replacement_indices,
      const int& replacement_num);
  // walks through the rows and prints each word in them with a new line after the
  // end of each row.
  void dbgDisplayRowText();
  // copies text in the rows vector into the sentence based upon the
  // indices found previously. assumes that the start row, end row,
  // start word, and end word indices have already been found.
  void copyRowTextIntoSentence(Sentence* sentence);
  // finds the sentence at the given row and word index and returns it's index.
  // if the sentence doesn't exist then returns -1
  int findSentence(const int& rowindex, const int& wordindex);

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


  GenericVector<Sentence*> recognized_sentences; // this holds the same text as
                                                 // recognized_text/lines, but now
                                                 // split into sentences.

  GenericVector<ROW_INFO*> rows; // list of all rows on the page including their
                                 // BLOBINFO elements grouped into their words
                                 // depending on how Tesseract initially segmented them

  Tesseract* tess; // the language-specific OCR engine used
  TessBaseAPI* api; // I instantiate a new api for recognizing normal text
                      // This api will not do any equation detection, just
                      // normal language detection. Once I have extracted
                      // all the information I need from it, it will be
                      // discarded.
  PIX* img; // the image on which all processing is carried out

  // scrollviews for debugging
  ScrollView* part_win;
  ScrollView* blobnboxgridview;
  ScrollView* rec_col_parts_sv;
  ScrollView* insertrblobs1_sv;
  ScrollView* insertrblobs2_sv;
  ScrollView* line_sv;
  ScrollView* sentence_sv;
};

} // end tesseract namespace

#endif

