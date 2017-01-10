/*
 * OldDebugMethods.h
 *
 *  Created on: Dec 21, 2016
 *      Author: jake
 */

#ifndef OLDDEBUGMETHODS_H_
#define OLDDEBUGMETHODS_H_

void doOldDebugMethods() {
#ifdef SHOW_BLOB_ROWS
  PIX* dbg_blob_row_im = pixCopy(NULL, img);
  dbg_blob_row_im = pixConvertTo32(dbg_blob_row_im);
  static int b_rows_dbg_num = 1;
  string rows_dbg_name = (string)"blob_rows_" + Basic_Utils::intToString(b_rows_dbg_num) +
      (string)".png";
  curblob = NULL;
  bigs.StartFullSearch();
  while((curblob = bigs.NextFullSearch()) != NULL) {
    if(curblob->rowinfo() != NULL)
      M_Utils::drawHlBlobInfoRegion(curblob, dbg_blob_row_im, LayoutEval::RED);
  }
  pixWrite(rows_dbg_name.c_str(), dbg_blob_row_im, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(dbg_blob_row_im, 100, 100);
  cout << "Displaying all blobs which belong to rows colored in red, and saving to "
      << rows_dbg_name << endl;
  M_Utils::waitForInput();
#endif
  ++b_rows_dbg_num;
  pixDestroy(&dbg_blob_row_im);
#endif

#ifdef SHOW_BLOB_WORDS
  PIX* dbg_blob_word_im = pixCopy(NULL, img);
  dbg_blob_word_im = pixConvertTo32(dbg_blob_word_im);
  static int b_word_dbg_num = 1;
  string words_dbg_name = (string)"blob_words_" + Basic_Utils::intToString(b_word_dbg_num) +
      (string)".png";
  curblob = NULL;
  bigs.StartFullSearch();
  while((curblob = bigs.NextFullSearch()) != NULL) {
    if(curblob->wordinfo != NULL)
      M_Utils::drawHlBlobInfoRegion(curblob, dbg_blob_word_im, LayoutEval::RED);
  }
  pixWrite(words_dbg_name.c_str(), dbg_blob_word_im, IFF_PNG);
#ifdef DBG_DISPLAY
  pixDisplay(dbg_blob_word_im, 100, 100);
  cout << "Displaying all blobs which belong to words colored in red, and saving to "
      << words_dbg_name << endl;
  M_Utils::waitForInput();
#endif
  ++b_word_dbg_num;
  pixDestroy(&dbg_blob_word_im);
#endif

#ifdef DBG_INFO_GRID_S
#ifdef DBG_INFO_GRID_SHOW_SENTENCE_REGIONS
  // for debugging, color the blobs for each sentence region and display the results
  // while showing the contents of the sentence on the terminal. do one at
  // a time requiring user input to continue in between displaying each sentence
  bool displayon = false;
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
    assert(false);
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
          cout << "highlighting blobs in line " << k + cursentence->startRowIndex
              << " of sentence " << j << endl;
          cout << "here is the line's text:\n";
          cout << tesseractRows[k+cursentence->startRowIndex]->getRowText() << endl;
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
    cout << "lines: " << cursentence->startRowIndex << "-" << cursentence->endRowIndex << endl;
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


#endif /* OLDDEBUGMETHODS_H_ */
