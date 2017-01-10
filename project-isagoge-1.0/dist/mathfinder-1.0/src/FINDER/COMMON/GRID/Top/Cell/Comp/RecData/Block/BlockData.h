/*
 * TesseractBlockData.h
 *
 *  Created on: Dec 18, 2016
 *      Author: jake
 */

#ifndef TESSERACTBLOCKDATA_H_
#define TESSERACTBLOCKDATA_H_

#include <baseapi.h>

#include <vector>

#include <pageres.h>
#include <BlobDataGrid.h>

#include <SentenceData.h>

class TesseractRowData;
class BlobDataGrid;

/**
 * Wrapper around a Tesseract Block (group of sentences, paragraphs or other
 * contents on a page that were seen as logically grouped during
 * layout analysis and recognition).
 */
class TesseractBlockData {

 public:

  TesseractBlockData(BLOCK_RES* const blockRes, BlobDataGrid* const parentGrid);

  ~TesseractBlockData();

  BLOCK_RES* getBlockRes();

  std::vector<TesseractRowData*>& getTesseractRows();

  /**
   * Finds the start and end of all sentences recognized by Tesseract
   * and stores the sentence data in a list within this object. Updates
   * the blobs on the grid so they have references to the sentences
   * to which they belong (if they belong to a sentence).
   */
  void findRecognizedSentences(tesseract::TessBaseAPI* tessBaseApi,
      BlobDataGrid* blobDataGrid);

  std::vector<TesseractSentenceData*>& getRecognizedSentences();

 private:

  BlobDataGrid* parentGrid;
  std::vector<TesseractRowData*> tesseractRows;
  BLOCK_RES* blockRes;
  std::vector<TesseractSentenceData*> tesseractSentences;

  /**
   * Private methods
   */
  int findSentence(const int& rowindex, const int& wordindex);

  void getSentenceRegions();

  Box* createLeptBox(TBOX tbox);


  /**
   * Debug methods
   */
  // walks through the rows and prints each word in them with a new line after the
  // end of each row.
  void dbgDisplayRowText();
};


#endif /* TESSERACTBLOCKDATA_H_ */
