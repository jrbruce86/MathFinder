/*
 * MathExpressionFinder.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: jake
 */
#include <MathExpressionFinder.h>

//#define SHOW_GRID

MathExpressionFinder::MathExpressionFinder(
    MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor,
    MathExpressionDetector* const mathExpressionDetector,
    MathExpressionSegmentor* const mathExpressionSegmentor,
    FinderInfo* const finderInfo) : init(false) {
  this->mathExpressionFeatureExtractor = mathExpressionFeatureExtractor;
  this->mathExpressionDetector = mathExpressionDetector;
  this->mathExpressionSegmentor = mathExpressionSegmentor;
  this->finderInfo = finderInfo;
}


MathExpressionFinder::~MathExpressionFinder() {
  delete mathExpressionFeatureExtractor;
  delete mathExpressionDetector;
  delete mathExpressionSegmentor;
}

std::vector<MathExpressionFinderResults*> MathExpressionFinder
::detectMathExpressions(Pixa* const images,
    std::vector<std::string> imageNames) {
  return getResultsInRunMode(DETECT, images, imageNames);
}

std::vector<MathExpressionFinderResults*> MathExpressionFinder
::findMathExpressions(Pixa* const images,
    std::vector<std::string> imageNames) {
  return getResultsInRunMode(FIND, images, imageNames);
}

MathExpressionFeatureExtractor* MathExpressionFinder::getFeatureExtractor() {
  return mathExpressionFeatureExtractor;
}

std::vector<MathExpressionFinderResults*> MathExpressionFinder
::getResultsInRunMode(
    RunMode runMode,
    Pixa* const images,
    std::vector<std::string> imageNames) {

  // Make sure the run mode is correct
  if(!(runMode == FIND || runMode == DETECT)) {
    std::cout << "Error: Unexpected run mode.\n";
    return std::vector<MathExpressionFinderResults*>();
  }

  // Call the finder initialization logic if not already called
  if(!init) {
    mathExpressionFeatureExtractor->doFinderInitialization();
    init = true;
  }

  assert(pixaGetCount(images) == imageNames.size());

  // Initialize the results vector
  std::vector<MathExpressionFinderResults*> results;

  /**
   * Get the results for each image, appending them to the vector
   */
  for(int i = 0; i < images->n; ++i) {

    std::cout << "Processing image " << imageNames[i] << ".\n";

    Pix* image = pixaGetPix(images, i, L_CLONE);

    /**
     * ---------------
     * Stage 1: Run Tesseract OCR and get the data
     * ---------------
     * Create a grid containing the connected components in the image and
     * their character results from running Tesseract's OCR with auto page
     * segmentation.
     */
    std::cout << "Creating blob grid.\n";
    tesseract::TessBaseAPI api;
    BlobDataGrid* const blobDataGrid =
        BlobDataGridFactory().createBlobDataGrid(image, &api, Utils::getNameFromPath(imageNames[i]));
#ifdef SHOW_GRID
    blobDataGrid->show();
#endif

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
    std::cout << "Extracting features.\n";
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
    std::cout << "Running detection.\n";
    mathExpressionDetector->detectMathExpressions(blobDataGrid);
    if(runMode == DETECT) {
      results.push_back(blobDataGrid->getDetectionResults(finderInfo->getFinderName()));
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
    std::cout << "Running segmentation.\n";
    if(runMode == FIND) {
      mathExpressionSegmentor->runSegmentation(blobDataGrid);
      results.push_back(blobDataGrid->getSegmentationResults(finderInfo->getFinderName()));
    }

    delete blobDataGrid;
    pixDestroy(&image);
  }

  return results;
}

