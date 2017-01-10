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

//// make virtual in header!!
//void BlobInfoGrid::HandleClick(int x, int y) {
//  cout << "-----------------------------------\n";
//  cout << "\nx,y: " << x << ", " << y << endl;
//  BlobInfoGridSearch bigs(this);
//  bigs.StartRadSearch(x, y, 0);
//  BLOBINFO* bb = bigs.NextRadSearch();
//  if(bb == NULL)
//    return;  // deallocate the blocks and everythhing inside them!!!!
//  cout << "the word corresponding to the blob you clicked:\n";
//  if(bb->wordstr() != NULL) {
//    cout << bb->wordstr() << endl;
//    if(bb->validword)
//      cout << "the blob is in a valid word!\n";
//    else
//      cout << "the blob is in an invalid word!\n";
//  }
//  else
//    cout << "(NULL)\n";
//  cout << "The tesseract boundingbox: \n";
//  TBOX t = bb->bounding_box();
//  M_Utils::dispTBoxCoords(&t);
//  if(bb->row_has_valid) {
//    cout << "The blob is on a row with atleast one valid word!\n";
//    cout << "The baseline for the blob is at y = " << bb->row()->base_line(bb->left()) << endl;
//    cout << "The blob's bottom y is at " << bb->bottom() << endl;
//    //ScrollView* baselineview = new ScrollView("baseline", 300, 100,
//    //                        img->w, img->h, img->w, img->h, false);
//    //bb->row->plot_baseline(baselineview, ScrollView::GREEN);
//  }
//  else
//    cout << "The blob is on a row with no valid words!\n";
//  cout << "blob has " << bb->nestedcount << " nested blobs within it\n";
//  // find the colpartition we are on
///*  ColPartitionGridSearch cpgs(part_grid); // comment from here to
//  ColPartition* cp = NULL;
//  cpgs.StartFullSearch();
//  int partnum = 1;
//  cp = cpgs.NextFullSearch();
//  while(cp != bb->original_part) {
//    partnum++;
//    cp = cpgs.NextFullSearch();
//  }
//  cout << "on partition number " << partnum << endl; // here ^*/
//  if(bb->row() != NULL) {
//    string rowstr = bb->rowinfo()->getRowText();
//    if(rowstr.empty())
//      cout << "blob belongs to a row that didn't have any recognized text\n";
//    else
//      cout << "blob belongs to row recognized wtih the follwing text:\n" << rowstr << endl;
//  }
//  else
//    cout << "\nthe blob you clicked on was not assigned a text line\n";
//  if(bb->sentenceindex == -1)
//    cout << "\nthe blob you clicked on was not assigned a sentence\n......\n";
//  else if(recognized_sentences[bb->sentenceindex] != NULL) {
//    cout << "\nthe blob you clicked on belongs to the following sentence:\n"
//         << recognized_sentences[bb->sentenceindex]->sentence_txt << endl << "......\n";
//  }
//  else
//    cout << "the blob you clicked belongs to sentence number " << bb->sentenceindex << ", which is NULL\n......\n";
//  if(bb->getParagraph() != NULL) {
//    cout << "The blob belongs to a word in a paragraph with the following traits:\n";
//    PARA* paragraph = bb->getParagraph();
//    if(paragraph->model == NULL)
//      cout << "the model is null!!\n";
//    else
//      cout << paragraph->model->ToString().string() << endl;
//    cout << "api has " << api->getParagraphModels()->length() << " models\n";
//  }
//  else
//    cout << "The blob doesn't belong to any paragraph.\n";
//#ifdef SHOW_BLOB_WINDOW
//    M_Utils::dispHlBlobInfoRegion(bb, img);
//    M_Utils::dispBlobInfoRegion(bb, img);
//#endif
//    if(dbgfeatures)
//      dbgDisplayBlobFeatures(bb);
//}
//
//void BlobInfoGrid::dbgDisplayBlobFeatures(BLOBINFO* blob) {
//  if(!blob->features_extracted) {
//    cout << "ERROR: Feature extraction hasn't been carried out on the blob. "
//         << "Disable dbgfeatures to ignore this.\n";
//    assert(false);
//  }
//  vector<double> features = blob->features;
//  if(features.size() != featformat.length())
//    cout << "features.size(): " << features.size() << ". featformat.length(): "
//         << featformat.length() << endl;
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

