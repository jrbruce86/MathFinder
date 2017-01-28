/*
 * BlobDataGrid.cpp
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#include <BlobDataGrid.h>

#include <BlobMergeData.h>
#include <BlobData.h>
#include <RowData.h>
#include <BlockData.h>
#include <WordData.h>
#include <CharData.h>
#include <SentenceData.h>
#include <M_Utils.h>

#include <baseapi.h>

BlobDataGrid::BlobDataGrid(const int& gridsize,
    const ICOORD& bleft,
    const ICOORD& tright,
    tesseract::TessBaseAPI* const tessBaseAPI,
    PIX* const image,
    std::string imageName): nonItalicizedRatio(-1) {
  this->Init(gridsize, bleft, tright);
  this->tessBaseAPI = tessBaseAPI;
  this->image = image;
  this->imageName = imageName;
  this->binaryImage = NULL;
}

// TODO: Make sure to deallocate things as necessary
BlobDataGrid::~BlobDataGrid() {
  for(int i = 0; i < tesseractBlocks.size(); ++i) {
    delete tesseractBlocks[i];
  }
  tesseractBlocks.clear();
  allRecognizedSentences.clear(); // these were just pointers managed by the blocks which should already have been deleted by now

  if(binaryImage != NULL) {
    delete binaryImage;
  }

  BlobDataGridSearch search(this);
  search.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = search.NextFullSearch()) != NULL) {
    if(blob != NULL) {
      delete blob;
      blob = NULL;
    }
  }
}

std::vector<TesseractBlockData*>& BlobDataGrid::getTesseractBlocks() {
  return tesseractBlocks;
}

Pix* BlobDataGrid::getImage() {
  return image;
}

Pix* BlobDataGrid::getBinaryImage() {
  if(binaryImage == NULL) {
    binaryImage = tessBaseAPI->GetThresholdedImage();
  }
  return binaryImage;
}

std::string BlobDataGrid::getImageName() {
  return imageName;
}

tesseract::TessBaseAPI* BlobDataGrid::getTessBaseAPI() {
  return tessBaseAPI;
}

std::vector<TesseractSentenceData*>& BlobDataGrid::getAllRecognizedSentences() {
  return allRecognizedSentences;
}

std::vector<TesseractRowData*>& BlobDataGrid::getAllTessRows() {
  return allTessRows;
}

double BlobDataGrid::getNonItalicizedRatio() {
  return nonItalicizedRatio;
}
void BlobDataGrid::setNonItalicizedRatio(double nonItalicizedRatio) {
  this->nonItalicizedRatio = nonItalicizedRatio;
}

GenericVector<Segmentation*>& BlobDataGrid::getSegments() {
  return Segments;
}

BlobData* BlobDataGrid::getEntryWithBoundingBox(const TBOX box) {
  BlobDataGridSearch search(this);
  search.SetUniqueMode(true);
  search.StartRectSearch(box);
  BlobData* b = search.NextRectSearch();
  while(b != NULL) {
    if(box == b->getBoundingBox()) {
      return b;
    }
    b = search.NextRectSearch();
  }
  return NULL;
}

void BlobDataGrid::HandleClick(int x, int y) {
  std::cout << "-----------------------------------\n";
  std::cout << "\nx,y: " << x << ", " << y << std::endl;
  BlobDataGridSearch bdgs(this);
  bdgs.StartRadSearch(x, y, 0);
  BlobData* bb = bdgs.NextRadSearch();
  if(bb == NULL)
    return;  // deallocate the blocks and everythhing inside them!!!!
  std::cout << "the character corresponding to the blob you clicked:\n";
  if(bb->getParentChar() != NULL) {
    std::cout << bb->getParentChar()->getUnicode() << std::endl;
    std::cout << "Character certainty: " << bb->getCharRecognitionConfidence() << std::endl;
  } else std::cout << "NULL\n";
  std::cout << "the word corresponding to the blob you clicked:\n";
  if(bb->getParentWordstr() != NULL) {
    std::cout << bb->getParentWordstr() << std::endl;
    std::cout << "Word certainty: " << bb->getWordRecognitionConfidence() << std::endl;
    if(bb->belongsToRecognizedWord())
      std::cout << "the blob is in a recognized 'valid' word!\n";
    else
      std::cout << "the blob is in an 'invalid' word!\n";
  }
  else
    std::cout << "(NULL)\n";
  if(bb->isMarkedAsTesseractSplit()) {
    std::cout << "Marked as split by Tesseract.\n";
  }
  std::cout << "The tesseract boundingbox: \n";
  TBOX t = bb->bounding_box();
  M_Utils::dispTBoxCoords(&t);
  if(bb->belongsToRecognizedNormalRow()) {
    std::cout << "The blob is on a row with atleast one valid word!\n";
    std::cout << "The baseline for the blob is at y = " << bb->getParentRow()->row()->base_line(bb->left()) << std::endl;
    std::cout << "The blob's bottom y is at " << bb->bottom() << std::endl;
    //ScrollView* baselineview = new ScrollView("baseline", 300, 100,
    //                        img->w, img->h, img->w, img->h, false);
    //bb->row->plot_baseline(baselineview, ScrollView::GREEN);
  }
  else
    std::cout << "The blob is on a row with no valid words!\n";
  //std::cout << "blob has " << bb->getVa << " nested blobs within it\n";
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
  if(bb->getParentRow() != NULL) {
    std::string rowstr = bb->getParentRow()->getRowText();
    if(rowstr.empty())
      std::cout << "blob belongs to a row that didn't have any recognized text\n";
    else
      std::cout << "blob belongs to row recognized wtih the follwing text:\n" << rowstr << std::endl;
  }
  else
    std::cout << "\nthe blob you clicked on was not assigned a text line\n";
  if(bb->getParentWord() != NULL) {
    TesseractWordData* parentWord = bb->getParentWord();
    if(parentWord->getSentenceIndex() == -1)
      std::cout << "\nthe blob you clicked on was not assigned a sentence\n......\n";
    else {
      TesseractBlockData* parentBlock = bb->getParentBlock();
      if(parentBlock == NULL) {
        std::cout << "\nthe blob you clicked on was assigned a sentence but has no parent block.\n";
      } else {

        int sentenceIndex = parentWord->getSentenceIndex();
        TesseractSentenceData* sentenceData = parentBlock->getRecognizedSentences()[sentenceIndex];
        if(sentenceData != NULL) {
          std::cout << "\nthe blob you clicked on belongs to the following sentence:\n";
          std::cout << sentenceData->getSentenceText() << std::endl << "......\n";
        }
      }
    }
  }
#ifdef SHOW_BLOB_WINDOW
  M_Utils::dispHlBlobInfoRegion(bb, img);
  M_Utils::dispBlobInfoRegion(bb, img);
#endif
//  if(dbgfeatures)
//    dbgDisplayBlobFeatures(bb);
}

      /// Maybe TODO later...
//void BlobInfoGrid::dbgDisplayBlobFeatures(BLOBINFO* blob) {
//  if(!blob->features_extracted) {
//    cout << "ERROR: Feature extraction hasn't been carried out on the blob. "
//        << "Disable dbgfeatures to ignore this.\n";
//    assert(false);
//  }
//  vector<double> features = blob->features;
//  if(features.size() != featformat.length())
//    cout << "features.size(): " << features.size() << ". featformat.length(): "
//    << featformat.length() << endl;
//  assert(features.size() == featformat.length());
//  cout << "Displaying extracted blob features\n------------------\n";
//  for(int i = 0; i < featformat.length(); i++)
//    cout << featformat[i] << ": " << features[i] << endl;
//#ifdef DBG_BASELINE
//  if(!blob->row_has_valid) {
//    cout << "blob doesn't belong to a valid row\n";
//    return;
//  }
//  int rowindex = blob->row_index;
//  cout << "rowindex: " << rowindex << endl;
//  assert(rowindex != -1);
//  assert(rows[rowindex]->rowid == blob->rowinfo()->rowid);
//  cout << "\n\nrow's average baseline distance: " << rows[rowindex]->avg_baselinedist << endl;
//  cout << "blob's distance from baseline: " << blob->dist_above_baseline << endl;
//  cout << "blob's bottom: " << blob->bottom() << endl;
//  cout << "row's baseline at blob location: " << rows[rowindex]->row()->base_line(blob->centerx()) << endl;
//  cout << "row's height: " << rows[rowindex]->row()->bounding_box().height() << endl;
//#endif
//  cout << "------------------------------------------------------\n";
//}

