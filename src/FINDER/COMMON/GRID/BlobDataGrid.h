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

class TesseractRowData;
class TesseractBlockData;
class TesseractSentenceData;
class BlobMergeData;
class Segmentation;

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

  GenericVector<Segmentation*>& getSegments();

  /**
   * Return an entry with the exact same bounding box as the one provided
   * if such an entry exists. If not, return NULL.
   */
  BlobData* getEntryWithBoundingBox(const TBOX box);

  void HandleClick(int x, int y);

 private:

  tesseract::TessBaseAPI* tessBaseAPI; // the api this grid relies on

  Pix* image; // the document image used as input to generate this grid

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

  GenericVector<Segmentation*> Segments; // results of segmentation! owned by the results data structure
};


#endif /* BLOBDATAGRID_H_ */
