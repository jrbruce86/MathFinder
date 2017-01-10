/*
 * TesseractBlockData.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#include <BlockData.h>

#include <M_Utils.h>

#include <stddef.h>
#include <assert.h>
#include <climits>

#include <OldDebugMethods.h>

#include <SentenceData.h>
#include <RowData.h>
#include <WordData.h>
#include <BlobDataGrid.h>

TesseractBlockData::TesseractBlockData(BLOCK_RES* blockRes,
    BlobDataGrid* parentGrid) {
  this->blockRes = blockRes;
  this->parentGrid = parentGrid;
}

TesseractBlockData::~TesseractBlockData() {
  for(int i = 0; i < tesseractSentences.size(); ++i) {
    delete tesseractSentences[i];
  }
  tesseractSentences.clear();

  for(int i = 0; i < tesseractRows.size(); ++i) {
    delete tesseractRows[i];
  }
  tesseractRows.clear();
}

BLOCK_RES* TesseractBlockData::getBlockRes() {
  return blockRes;
}

std::vector<TesseractRowData*>& TesseractBlockData::getTesseractRows() {
  return tesseractRows;
}



// here a sentence is simply any group of one or more words starting with
// a capital letter, valid first word, and ending with a period or question mark.
void TesseractBlockData::findRecognizedSentences(
    tesseract::TessBaseAPI* api,
    BlobDataGrid* blobDataGrid) {

  BlobDataGridSearch bdgs(blobDataGrid);

  // Determine where the sentences are relative to each row
  // use the api to figure out if words are valid
  //dbgDisplayRowText();
  // walk through the rows
  bool sentence_found = false;
  for(int i = 0; i < tesseractRows.size(); ++i) {
    // walk through the words on each row
    TesseractRowData* rowinfo = tesseractRows[i];
    GenericVector<TesseractWordData*>& words = rowinfo->getTesseractWords();
    for(int j = 0; j < words.length(); ++j) {
      if(words[j] == NULL)
        continue;
      if(words[j]->wordstr() == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(!sentence_found) { // looking for the start of a sentence
        if((tesseractRows.size() == (i+1))
            && (words.length() == (j+1)))
          continue; // don't create an empty sentence if at the end of the block
        if(isupper(wordstr[0]) && islower(wordstr[1])) {
          // see if the uppercase word is valid or not based on the api
          if(api->IsValidWord(wordstr)) {
            // found the start of a sentence!!
            tesseractSentences.push_back(
                new TesseractSentenceData(this, i, j));
            sentence_found = true;
          }
        }
      }
      else {  // looking for the end of the current sentence
        // for now I'll assume a '.' or '?' is the end of the sentence
        // (also I'll count the last character on the block as the end
        // of a sentence if the block ends while a sentence ending is
        // still being looked for)
        char lastchar = wordstr[strlen(wordstr) - 1];
        if((lastchar == '.') || (lastchar == '?') ||
            ((tesseractRows.size() == (i+1))
                && (words.length() == (j+1)))) {
          // found the end of a sentence!!
          TesseractSentenceData* sentence = tesseractSentences.back();
          sentence->readInBlockText(i, j);
          if(sentence->sentence_txt == NULL) {
            // if it's empty then get rid of it
            tesseractSentences.pop_back(); // remove the last element
            delete sentence;
            sentence = NULL;
          }
          sentence_found = false; // look for a new sentence
        }
      }
    }
  }

  // if a sentence was started near the end of the page and followed by all null
  // words then there'll be an empty sentence at the end of the page. if there is
  // then delete it here
  TesseractSentenceData* lastsentence = tesseractSentences.back();
  if(lastsentence->getSentenceText() == NULL) {
    tesseractSentences.pop_back();
    delete lastsentence;
    lastsentence = NULL;
  }

  // Done finding the sentences, optional debugging below
#ifdef DBG_INFO_GRID
  std::cout << "...............................................\n";
  std::cout << "here are the " << tesseractSentences.size()
           << " sentences that were found:\n";
  for(int i = 0; i < tesseractSentences.size(); i++) {
    char* sentence = recognized_sentences[i]->getSentenceText();
    std::cout << i << ": " << ((sentence != NULL) ? sentence : "NULL")
             << std::endl << "------\n";
  }
  std::cout << "................................................\n";
  M_Utils::waitForInput();
#endif

  // Now walk through the rows and words again, this time assigning all of the
  // blobs to the sentence to which they belong.
  int sentence_index = 0;
  for(int i = 0; i < tesseractRows.size(); ++i) {
    TesseractRowData* const rowinfo = tesseractRows[i];
    GenericVector<TesseractWordData*> words = rowinfo->getTesseractWords();
    for(int j = 0; j < words.length(); ++j) {
      TesseractWordData* const wordinfo = words[j];
      // find which sentence this word belongs to if any based on the
      // current row and word indices.
      int sentence_index = findSentence(i, j);
      if(sentence_index < 0) {
        continue;
      }
      wordinfo->setSentenceIndex(sentence_index);
    }
  }
#ifdef DBG_INFO_GRID
  sentence_sv = MakeWindow(100, 100, "BlobInfoGrid after getting the sentences");
  DisplayBoxes(sentence_sv);
  M_Utils::waitForInput();
#endif

  getSentenceRegions();

  doOldDebugMethods(); // only invokes if turned on during compilation
}

std::vector<TesseractSentenceData*>& TesseractBlockData::getRecognizedSentences() {
  return tesseractSentences;
}

int TesseractBlockData::findSentence(const int& rowindex, const int& wordindex) {
  for(int i = 0; i < tesseractSentences.size(); ++i) {
    TesseractSentenceData* const cursentence = tesseractSentences[i];
    const int s_start_row = cursentence->getStartRowIndex();
    const int s_end_row = cursentence->getEndRowIndex();
    const int s_start_word = cursentence->getStartWordIndex();
    const int s_end_word = cursentence->getEndWordIndex();
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

Box* TesseractBlockData::createLeptBox(TBOX tbox) {
  return M_Utils::tessTBoxToImBox(&tbox, parentGrid->getImage());
}

void TesseractBlockData::getSentenceRegions() {
#ifdef DBG_INFO_GRID_S
  int dbgsentence = -1;
#endif
  // 1. Assign each sentence to a boxarray which gives the isothetic
  //    region corresponding to the sentence (each box in the array
  //    is a row or portion of a row for that sentence)
  for(int j = 0; j < tesseractSentences.size(); ++j) {

#ifdef DBG_INFO_GRID_S
    if(j == dbgsentence) {
      std::cout << "about to find line boxes for sentence " << j << ":\n";
      std::cout << recognizedSentences[j]->sentence_txt << std::endl;
      std::cout << "the lines of this sentence are " << startRowIndex << "-" << endRowIndex << std::endl;
      M_Utils::waitForInput();
    }
#endif

    TesseractSentenceData* const cursentence = tesseractSentences[j];

    // Grab the convenience variables up front
    const int startRowIndex = cursentence->getStartRowIndex();
    TesseractRowData* const startRow = tesseractRows[startRowIndex];
    const int startWordIndex = cursentence->getStartWordIndex();
    TesseractWordData* const startWord = startRow->getTesseractWords()[startWordIndex];
    const int endRowIndex = cursentence->getEndRowIndex();
    TesseractRowData* const endRow = tesseractRows[endRowIndex];
    const int endWordIndex = cursentence->getEndWordIndex();
    TesseractWordData* const endWord = endRow->getTesseractWords()[endWordIndex];
    const int lastWordRight = endWord->getBoundingBox().right();
    const int firstWordLeft = startWord->getBoundingBox().left();
    TBOX startRowBox = startRow->getBoundingBox();
    const int numRows = endRowIndex - startRowIndex + 1;

    // holds separate box for each row of the sentence
    Boxa* sentencelines = boxaCreate(numRows);

    // find the bounding boxes corresponding to the sentences within
    // their rows. This'll be a subset of the starting and ending row
    // but should contain the entirety of the rows in between if
    // there are any

    // single row just has one box (all of or a subset of that row's box)
    if(numRows == 1) {
      TBOX sentenceBox(firstWordLeft, startRowBox.bottom(), lastWordRight, startRowBox.top());
      boxaAddBox(sentencelines,createLeptBox(sentenceBox), L_INSERT);
    } else { // sentence spans two or more rows
      // add the box for the first row of the sentence
      TBOX firstBox(firstWordLeft, startRowBox.bottom(), startRowBox.right(), startRowBox.top());
      boxaAddBox(sentencelines, createLeptBox(firstBox), L_INSERT);
      if(numRows > 2) {
        // add the boxes in between if there are any
        for(int k = startRowIndex + 1; k < endRowIndex; ++k) {
          boxaAddBox(sentencelines,
              createLeptBox(tesseractRows[k]->getBoundingBox()),
              L_INSERT);
        }
      }
      // add the box for the last row of the sentence
      TBOX endRowBox = endRow->getBoundingBox();
      TBOX lastBox(endRowBox.left(), endRowBox.bottom(), lastWordRight, endRowBox.top());
      boxaAddBox(sentencelines, createLeptBox(lastBox), L_INSERT);
    }
    cursentence->setRowBoxes(sentencelines);
  }
}


void TesseractBlockData::dbgDisplayRowText() {
  for(int i = 0; i < tesseractRows.size(); ++i) {
    TesseractRowData* const rowinfo = tesseractRows[i];
    GenericVector<TesseractWordData*>& words = rowinfo->getTesseractWords();
    for(int j = 0; j < words.length(); ++j) {
      if(words[j] == NULL)
        continue;
      const char* wordstr = words[j]->wordstr();
      if(wordstr == NULL)
        continue;
      std::cout << wordstr << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "\n------------------------------\n\n";
}


