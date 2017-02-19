/*
 * BlobDataGrid.h
 *
 *  Created on: Nov 11, 2016
 *      Author: jake
 */

#ifndef BLOBDATAGRID_H_
#define BLOBDATAGRID_H_

#include <string>
#include <vector>
#include <baseapi.h>
#include <tesseractclass.h>
#include <bbgrid.h>

#include <BlobMergeData.h>

#include <Lept_Utils.h>

class TesseractRowData;
class TesseractBlockData;
class TesseractSentenceData;
class BlobMergeData;
class Segmentation;
class MathExpressionFinderResults;

class BlobData;
CLISTIZEH(BlobData)
typedef tesseract::GridSearch<BlobData, BlobData_CLIST, BlobData_C_IT> BlobDataGridSearch;

class BlobDataGrid : public tesseract::BBGrid<BlobData, BlobData_CLIST, BlobData_C_IT> {
 public:

  BlobDataGrid(const int& gridsize,
      const ICOORD& bleft,
      const ICOORD& tright,
      tesseract::TessBaseAPI* const tessBaseAPI,
      PIX* const image,
      std::string imageName);

  ~BlobDataGrid();

  std::vector<TesseractBlockData*>& getTesseractBlocks();

  Pix* getImage();

  Pix* getBinaryImage();

  std::string getImageName();

  tesseract::TessBaseAPI* getTessBaseAPI();

  /**
   * Gets list containing all of the sentences recognized on the page (includes
   * the sentences from each and every block if there is more than one block)
   */
  std::vector<TesseractSentenceData*>& getAllRecognizedSentences();

  /**
   * Gets a list containing all of the rows recognized by Tesseract (includes
   * the rows from each and every block if there is more than one block)
   */
  std::vector<TesseractRowData*>& getAllTessRows();

  /**
   * Ratio of non-italicized to total blobs on the page based on tesseract results
   */
  double getNonItalicizedRatio();
  void setNonItalicizedRatio(double nonItalicizedRatio);

  /**
   * Appends a segmentation to this grid's list of segmentations
   */
  void appendSegmentation(Segmentation* const segmentation);

  /**
   * Removes the segmentation from this grid's list of segmentations
   */
  void removeSegmentation(Segmentation* const segmentation);

  /**
   * Return a deep copy of all of the segments
   */
  GenericVector<Segmentation*> getSegmentsCopy();

  /**
   * Return an entry with the exact same bounding box as the one provided
   * if such an entry exists. If not, return NULL.
   */
  BlobData* getEntryWithBoundingBox(const TBOX box);

  /**
   * Builds out and returns the results of detection. The allocated memory
   * is owned by the caller.
   */
  MathExpressionFinderResults* getDetectionResults(
      const std::string& resultsDirName);

  /**
   * Returns the blobs detected as math as "segment" objects.
   * Although the segmentation algorithm wouldn't have been carried out
   * at this stage, the detection results are effectively segments all the
   * same. Just a whole ton of very small ones that have yet to be merged
   * properly into expressions.
   */
  GenericVector<Segmentation*> getDetectionSegments();

  /**
   * Gets a visual display of the results of detection.
   * The pix memory is allocated on the heap and owned by the caller
   */
  Pix* getVisualDetectionResultsDisplay();

  /**
   * Builds out and returns the results of segmentation. The allocated memory
   * is owned by the caller.
   */
  MathExpressionFinderResults* getSegmentationResults(
      const std::string& resultsDirName);

  LayoutEval::Color getColorFromRes(const RESULT_TYPE restype);

  /**
   * Gets a visual display of the results of segmentation.
   * The pix memory is allocated on the heap and owned by the caller
   */
  Pix* getVisualSegmentationResultsDisplay();

  /**
   * Show the grid, waiting for enter to be pressed before deleting the
   * display.
   */
  void show();

  void HandleClick(int x, int y);

  int getArea();

 private:

  tesseract::TessBaseAPI* tessBaseAPI; // the api this grid relies on

  Pix* image; // the document image used as input to generate this grid (not owned by the grid)

  Pix* binaryImage; // the image after being thresholded by Tesseract

  std::string imageName;

  // list of wrappers around groups of sentences/paragraphs or other contents
  // logically grouped together by Tesseract during layout analysis and recognition
  std::vector<TesseractBlockData*> tesseractBlocks;

  // list containing all of the sentences recognized on the page (includes
  // the sentences from each and every block if there is more than one block)
  std::vector<TesseractSentenceData*> allRecognizedSentences;

  // list containing all of the rows recognized on the page (includes
  // the rows from each and every block if there is more than one block)
  std::vector<TesseractRowData*> allTessRows;

  // ratio of non italicized blobs to total blobs on the page according to
  // tesseract recognition results
  double nonItalicizedRatio;

  // results of segmentation. each segment owned by first blob that has a reference
  // to it that is deleted from the grid. The results thus needs to create a copy
  // of this to avoid memory issues.
  GenericVector<Segmentation*> segmentations;

  int area;
};


#endif /* BLOBDATAGRID_H_ */
