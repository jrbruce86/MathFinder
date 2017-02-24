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
#include <MFinderResults.h>

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
  this->area = gridheight_ * gridwidth_;
}

// TODO: Make sure to deallocate things as necessary
BlobDataGrid::~BlobDataGrid() {
  for(int i = 0; i < tesseractBlocks.size(); ++i) {
    delete tesseractBlocks[i];
  }
  tesseractBlocks.clear();
  allRecognizedSentences.clear(); // these were just pointers managed by the blocks which should already have been deleted by now

  pixDestroy(&binaryImage);

  // First grab any shared (double) pointers. Have to delete them after the blobs are deleted
  GenericVector<BlobMergeData**> mergeDataShared;
  {
    BlobDataGridSearch search(this);
    search.SetUniqueMode(true);
    search.StartFullSearch();
    BlobData* blob = NULL;
    while((blob = search.NextFullSearch()) != NULL) {
      BlobMergeData** mergeData = blob->getMergeDataSharedPtr();
      if(mergeData != NULL) {
        if(!mergeDataShared.bool_binary_search(mergeData)) {
          mergeDataShared.push_back(mergeData);
          mergeDataShared.sort();
        }
      }
    }
  }

  // Now delete all of the blobs
  {
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

  // Now delete any shared pointers
  for(int i = 0; i < mergeDataShared.size(); ++i) {
    delete [] mergeDataShared[i];
  }

  // the segments are owned by the blobs (first one deleted who
  // has the shared pointer to the segment will delete that segment)
  segmentations.clear();
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

void BlobDataGrid::appendSegmentation(Segmentation* const segmentation) {
  segmentations.push_back(segmentation);
}

Segmentation* BlobDataGrid::getSegmentationWithBox(const TBOX& segBox, int excludeIndex) {
  for(int i = 0; i < segmentations.size(); ++i) {
    if(*(segmentations[i]->box) == segBox && excludeIndex != i) {
      return segmentations[i];
    }
  }
  return NULL;
}

void BlobDataGrid::removeSegmentation(Segmentation* const segmentation) {
  TBOX* const boxToRemove = segmentation->box;
  for(int i = 0; i < segmentations.size(); ++i) {
    TBOX* const curBox = segmentations[i]->box;
    if(*curBox == *boxToRemove) {
      segmentations.remove(i);
      break;
    }
  }
}

GenericVector<Segmentation*> BlobDataGrid::getSegmentsCopy() {
  GenericVector<Segmentation*> copyVec;
  for(int i = 0; i < segmentations.size(); ++i) {
    TBOX* const segBox = segmentations[i]->box;
    const RESULT_TYPE segRes = segmentations[i]->res;
    Segmentation* const segCopy = new Segmentation();
    segCopy->box = new TBOX(segBox->left(), segBox->bottom(), segBox->right(), segBox->top());
    segCopy->res = segRes;
    copyVec.push_back(segCopy);
  }
  return copyVec;
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

MathExpressionFinderResults* BlobDataGrid::getDetectionResults(
    const std::string& resultsDirName) {
  return MathExpressionFinderResultsBuilder()
      .setResults(getDetectionSegments())
      ->setVisualResultsDisplay(getVisualDetectionResultsDisplay())
      ->setVisualEvalResultsDisplay(getVisualDetectionResultsDisplay(false))
      ->setResultsName(getImageName())
      ->setResultsDirName(resultsDirName)
      ->setRunMode(DETECT)
      ->build();
}

GenericVector<Segmentation*> BlobDataGrid::getDetectionSegments() {
  GenericVector<Segmentation*> detectedSegments;
  BlobDataGridSearch search(this);
  search.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = search.NextFullSearch()) != NULL) {
    if(blob->getMathExpressionDetectionResult()) {
      Segmentation* seg = new Segmentation();
      seg->box = new TBOX(
          blob->getBoundingBox().left(),
          blob->getBoundingBox().bottom(),
          blob->getBoundingBox().right(),
          blob->getBoundingBox().top());
      seg->res = DISPLAYED; // just fixing to displayed for detection results
      detectedSegments.push_back(seg);
    }
  }
  return detectedSegments; // returns a copy (ok since its just a vec of pointers)
}

Pix* BlobDataGrid::getVisualDetectionResultsDisplay(const bool drawBox) {

  Pix* display = pixCopy(NULL, getBinaryImage());
  display = pixConvertTo32(display);

  BlobDataGridSearch search(this);
  search.StartFullSearch();
  BlobData* blob = NULL;
  while((blob = search.NextFullSearch()) != NULL) {
    if(blob->getMathExpressionDetectionResult()) {
      M_Utils::drawHlBlobDataRegion(blob, display, LayoutEval::RED);
      if(drawBox) {
        Lept_Utils::drawBox(display, blob->getBoundingBox(), LayoutEval::RED, 7);
      }
    }
  }

  return display;
}

MathExpressionFinderResults* BlobDataGrid::getSegmentationResults(
    const std::string& resultsDirName) {
  return MathExpressionFinderResultsBuilder()
      .setResults(getSegmentsCopy())
      ->setVisualResultsDisplay(getVisualSegmentationResultsDisplay())
      ->setVisualEvalResultsDisplay(getVisualSegmentationEvalResultsDisplay())
      ->setResultsName(getImageName())
      ->setResultsDirName(resultsDirName)
      ->setRunMode(FIND)
      ->build();
}

LayoutEval::Color BlobDataGrid::getColorFromRes(const RESULT_TYPE restype) {
  return ((restype == DISPLAYED) ?
      LayoutEval::RED : (restype == EMBEDDED) ? LayoutEval::BLUE
          : LayoutEval::GREEN);
}

Pix* BlobDataGrid::getVisualSegmentationResultsDisplay(const bool drawBox) {
  Pix* display = pixCopy(NULL, getBinaryImage());
  display = pixConvertTo32(display);
  for(int i = 0; i < segmentations.length(); ++i ) {
    const Segmentation* seg = segmentations[i];
    BOX* bbox = M_Utils::tessTBoxToImBox(seg->box, display);
    const RESULT_TYPE& restype = seg->res;
    M_Utils::drawHlBoxRegion(bbox, display, getColorFromRes(restype));
    if(drawBox) {
      Lept_Utils::drawBox(display, bbox, getColorFromRes(restype), 10);
    }
    boxDestroy(&bbox);
  }
  return display;
}

Pix* BlobDataGrid::getVisualSegmentationEvalResultsDisplay() {
  return getVisualSegmentationResultsDisplay(false);
}

void BlobDataGrid::show() {
  ScrollView* sv = MakeWindow(image->w, image->h,
      imageName.c_str());
  DisplayBoxes(sv);
  std::cout << "Displaying scroll view. Make sure to close it when finished viewing.\n";
  M_Utils::waitForInput();
  delete sv;
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
  if(bb->belongsToBadRegion()) {
    std::cout << "The blob belongs to a 'bad' region.\n";
  } else {
    std::cout << "The blob does not belong to a 'bad' region.\n";
  }
  if(bb->getParentWordstr() != NULL) {
    std::cout << bb->getParentWordstr() << std::endl;
    std::cout << "Word certainty: " << bb->getWordRecognitionConfidence() << std::endl;
    std::cout << "Average blob certainty in parent word: " << bb->getWordAvgRecognitionConfidence() << std::endl;
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
  std::cout << "The tesseract blob boundingbox in normal coords: \n";
  TBOX t = bb->bounding_box();
  M_Utils::dispTBoxLeptCoords(t, getBinaryImage());
  std::cout << "In tesseract coords:\n";
  std::cout << "(left, bottom, right, top) = " << t.left() << ", " << t.bottom() << ", " << t.right() << ", " << t.top() << std::endl;
  if(bb->getParentWord() != NULL) {
    std::cout << "The tesseract word bounding box: \n";
    M_Utils::dispTBoxLeptCoords(bb->getParentWord()->getBoundingBox(), getBinaryImage());
  }
  if(bb->belongsToRecognizedNormalRow()) {
    std::cout << "The blob is on a row that is considered 'normal' paragraph text based on average vertical spacing on the page.\n";
    std::cout << "The baseline for the blob is at y = " << bb->getParentRow()->row()->base_line(bb->left()) << std::endl;
    std::cout << "The blob's bottom y is at " << bb->bottom() << std::endl;
    //ScrollView* baselineview = new ScrollView("baseline", 300, 100,
    //                        img->w, img->h, img->w, img->h, false);
    //bb->row->plot_baseline(baselineview, ScrollView::GREEN);
  }
  else {
    std::cout << "The blob is on a row with no valid words!\n";
  }
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
}

int BlobDataGrid::getArea() {
  return area;
}
