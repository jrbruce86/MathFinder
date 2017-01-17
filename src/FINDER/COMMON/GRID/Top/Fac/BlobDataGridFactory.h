/*
 * BlobDataGridFactory.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef BLOBDATAGRIDFACTORY_H_
#define BLOBDATAGRIDFACTORY_H_

#include <allheaders.h>
#include <baseapi.h>
#include <string>

class BlobDataGrid;
class TesseractRowData;
class TesseractBlockData;
class TesseractWordData;
class TesseractCharData;
class BlobData;

class BlobDataGridFactory {
 public:

  /**
   * Runs Tesseract's recognition on the given image with auto page segmentation,
   * inserts the raw connected components of the image into the grid, finds the
   * Tesseract character results corresponding to each raw connected component
   * inserting them into their appropriate entry in the grid, and returns the
   * created grid. The grid created is owned by the caller who should delete its
   * memory when finished with it. The parameters passed into this factory are
   * also owned by the caller.
   */
  BlobDataGrid* createBlobDataGrid(Pix* image,
      tesseract::TessBaseAPI* tessBaseApi, const std::string imageName);

 private:

  // Returns the recognized word if theres a 'valid' one on the row,
  // otherwise returns NULL
  char* getRowValidTessWord(
      TesseractRowData* const rowData,
      tesseract::TessBaseAPI* const tessBaseApi);

  // Determines some basic characteristics for each Tesseract row based on an
  // analysis of all the rows on the page. A row can then be considered as
  // "normal" or "abnormal" from a Tesseract perspective based upon this
  // analysis.
  void findAllRowCharacteristics(BlobDataGrid* const blobDataGrid);

  void dbgDisplayHierarchy(
      TesseractBlockData* tesseractBlockData,
      TesseractRowData* tesseractRowData,
      TesseractWordData* tesseractWordData,
      TesseractCharData* tesseractCharData,
      BlobData* blob, Pix* image);
};


#endif /* BLOBDATAGRIDFACTORY_H_ */
