/*
 * MathExpressionFinder.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: jake
 */
#include <MathExpressionFinder.h>

MathExpressionFinder::MathExpressionFinder(
    MathExpressionFeatureExtractor* mathExpressionFeatureExtractor,
    MathExpressionDetector* mathExpressionDetector,
    MathExpressionSegmentor* mathExpressionSegmentor) : init(false) {
  this->mathExpressionFeatureExtractor = mathExpressionFeatureExtractor;
  this->mathExpressionDetector = mathExpressionDetector;
  this->mathExpressionSegmentor = mathExpressionSegmentor;
}


MathExpressionFinder::~MathExpressionFinder() {
  delete mathExpressionFeatureExtractor;
  delete mathExpressionDetector;
  delete mathExpressionSegmentor;
}

/**
 * Finds the math expressions in one or more images provided, returning the
 * results as a vector where the result vector indexes correspond with the
 * image ones (i.e., the result at the first index is for the image at that
 * same index in its array).
 */
std::vector<MathExpressionFinderResults*> MathExpressionFinder
::findMathExpressions(Pixa* const images, std::vector<std::string> imageNames) {

  //TODO: Incorporate option to find math expressions using the default tesseract
  //      implementation


  //TODO: (Very low priority) Investigate whether or not it's do-able to add in
  //      feature to sub in my equation detector in place of Tesseract's default
  //      one and then see tesseract's results with that

  // Initialize the results vector
  std::vector<MathExpressionFinderResults*> results;

  // Call the finder initialization logic if not already called
  if(!init) {
    mathExpressionFeatureExtractor->doFinderInitialization();
    init = true;
  }

  assert(pixaGetCount(images) == imageNames.size());

  for(int i = 0; i < images->n; ++i) {

    Pix* image = pixaGetPix(images, i, L_CLONE);

    /**
     * ---------------
     * Stage 1: Run Tesseract OCR and get the data
     * ---------------
     * Create a grid containing the connected components in the image and
     * their character results from running Tesseract's OCR with auto page
     * segmentation.
     */
    tesseract::TessBaseAPI api;
    BlobDataGrid* const blobDataGrid =
        BlobDataGridFactory().createBlobDataGrid(image, &api, Utils::getNameFromPath(imageNames[i]));

    /**
     * ---------------
     * Stage 2: Extract my features from each blob based upon that data
     * ---------------
     * Now that I have a grid containing the raw connected components, their basic
     * data, and recognition data from Tesseract, I am ready to carry out my own
     * feature extraction. My feature extractor will iterate over each blob and
     * run whatever feature extractors were set from the command line for them.
     * The results of the feature extraction are stored within the blob's grid entry
     * within the grid that is passed into the extraction method.
     */
    mathExpressionFeatureExtractor->extractFeatures(blobDataGrid);

    /**
     * ---------------
     * Stage 3: Detect low-level math expressions within the blobs using extracted
     *          features
     * ---------------
     * Using the features extracted in the previous stage (along with any other data
     * already stored in each entry that might be helpful) I now classify each
     * individual blob as either being math or non-math. Depending on the detector
     * being used I may also further categorize a blob as being a displayed math,
     * embedded math, or a label for math.
     */
    mathExpressionDetector->detectMathExpressions(blobDataGrid);

    /*if(res == DETECTION) {
      // print the detection results to both file and image
      BLOBINFO* blob;
      BlobInfoGridSearch bigs(blobinfogrid);
      bigs.StartFullSearch();
      while((blob = bigs.NextFullSearch()) != NULL) {
        if(blob->predicted_math) {
          BOX* bbox = M_Utils::getBlobInfoBox(blob, img);
          rectstream << dbg_img_index << ".png embedded "
                     << bbox->x << " " << bbox->y << " "
                     << bbox->x + bbox->w << " " << bbox->y + bbox->h << endl;
          M_Utils::drawHlBlobInfoRegion(blob, dbgimg, LayoutEval::RED);
          boxDestroy(&bbox);
        }
      }
    }

    /**
     * ---------------
     * Stage 4: Segment the detection results into math expressions (also uses
     *          features extracted)
     * ---------------
     * Starting from the detection results from the previous stage, in this stage
     * I figure out how each result should be segmented into math expressions (or
     * labels for them if applicable). This involves combining neighboring blobs
     * into single math expressions which will be the output of the program.
     */
    results.push_back(mathExpressionSegmentor->runSegmentation(blobDataGrid));

    // TODO: do all necessary deallocations.. make sure blob data grid destructor
    //       invoked and does necessary stuff
    // be careful with leptonica stuff!!!!! make sure to call the destroy method on all references!!!!
    delete blobDataGrid;
    pixDestroy(&image);
  }

  return results;
}

MathExpressionFeatureExtractor* MathExpressionFinder::getFeatureExtractor() {
  return mathExpressionFeatureExtractor;
}
