/*
 * TrainingSampleExtractor.cpp
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#include <TrainingSampleExtractor.h>

#include <FinderInfo.h>
#include <FeatExt.h>
#include <Sample.h>
#include <GTParser.h>
#include <Utils.h>
#include <M_Utils.h>
#include <SampleFileParser.h>
#include <BlobDataGrid.h>
#include <BlobDataGridFactory.h>
#include <DatasetMenu.h>

#include <baseapi.h>

#include <allheaders.h>

#include <vector>
#include <assert.h>
#include <stddef.h>

#define DBG;

TrainingSampleExtractor::TrainingSampleExtractor(FinderInfo* const finderInfo,
    MathExpressionFeatureExtractor* const featureExtractor)
: samples_extracted(std::vector<std::vector<BLSample*> >()),
  samples_read(std::vector<std::vector<BLSample*> >()) {
  this->finderInfo = finderInfo;
  this->featureExtractor = featureExtractor;
}


// Will invoke the feature extractors to get new samples on every image
// of the dataset if the samples were not already generated and written
// to a file. If the samples can be read in from a file, will prompt the
// user if they want to read in the old ones or generate new ones. When
// finished generating new samples, writes them to a file to speed things
// up later. Returns a list where each entry of the list holds the samples
// extracted from one image (the indices of the images in this list correspond
// with the image names, for instance image 0.png will be index 0, etc.).
// Note:
// The calling class is responsible for deallocating the samples created here
std::vector<std::vector<BLSample*> > TrainingSampleExtractor::getSamples() {
  if(!samples_extracted.empty() || !samples_read.empty()) {
    std::cout << "ERROR: samples have already been extracted but not deleted "
         << "prior to calling getSamples(). Make sure samples are "
         << "deleted prior to calling this function.\n";
    assert(false);
  }
  // get the samples
  std::string sample_path = finderInfo->getFinderTrainingPaths()->getSampleFilePath();
  bool generateNewSamples = true;
  if(Utils::existsFile(sample_path)) {
    // Prompt to see if ok to just read in old files
    std::cout << "The training samples were already previously extracted for this Finder "
        "and stored to a file. Would you like to re-use the previously extracted samples "
        "for this training? ";
    generateNewSamples = !Utils::promptYesNo();
  }

  if(generateNewSamples) {
    getNewSamples();
    return samples_extracted;
  } else {
    samples_read = SampleFileParser::readOldSamples(
        sample_path,
        featureExtractor->getBlobFeatureExtractors());
    return samples_read;
  }
}


// does feature extraction on all images in the groundtruth data set and writes
// the resulting features to a file
void TrainingSampleExtractor::getNewSamples(bool writeToFile) {
  // assumes all n files in the training dir are images and are named as 0, 1, 2, etc.
  // iterate the images in the dataset, getting the features
  // from each and appending them to the samples vector

  // initialize any feature extraction parameters which need to do
  // precomputations on the entire training set prior to any feature
  // extraction. this may or may not be applicable depending on the
  // feature extraction implementations being used

  featureExtractor->doTrainerInitialization();


  // Extract the features for each image in the groundtruth dataset
  for(int i = 0; i < finderInfo->getGroundtruthImagePaths().size(); ++i) {
    tesseract::TessBaseAPI api; // the tesseract api that will be used for features which require it during feature extraction

    const std::string imagePath = finderInfo->getGroundtruthImagePaths()[i];
    Pix* image = Utils::leptReadImg(imagePath);

    BlobDataGrid* blobDataGrid = BlobDataGridFactory().createBlobDataGrid(image, &api, Utils::getNameFromPath(imagePath));
#ifdef DBG
    string winname = "BlobDataGrid for Image " +  Utils::getNameFromPath(imagePath);
    ScrollView* gridviewer = blobDataGrid->MakeWindow(100, 100, winname.c_str());
    blobDataGrid->DisplayBoxes(gridviewer);
#endif


    // now to get the features from the grid and append them to the
    // samples vector.
    std::vector<BLSample*> img_samples = getGridSamples(blobDataGrid, i);

#ifdef DBG
    cout << "Finished grabbing samples.\n";
    M_Utils::waitForInput();
    delete gridviewer;
    gridviewer = NULL;
#endif

#ifdef DBG_MEDS_TRAINER_SHOW_TRAINDATA
    // to debug I'll color all the blobs that are labeled as math
    // red and all the other ones as blue
    Pix* colorimg = Utils::leptReadImg(img_filepath);
    colorimg = pixConvertTo32(colorimg);
    for(int j = 0; j < img_samples.size(); j++) {
      BLSample* sample = img_samples[j];
      GroundTruthEntry* entry = sample->entry;
      if(sample->label)
        Lept_Utils::fillBoxForeground(colorimg, sample->blobbox, LayoutEval::RED);
      else
        Lept_Utils::fillBoxForeground(colorimg, sample->blobbox, LayoutEval::BLUE);
    }
    string dbgname = "dbg_training_im" + Utils::intToString(i) + ".png";
    pixWrite(dbgname.c_str(), colorimg, IFF_PNG);
#ifdef DBG_DISPLAY
    pixDisplay(colorimg, 100, 100);
    M_Utils::waitForInput();
#endif
    pixDestroy(&colorimg);
#endif


    // now append the samples found for the current image to the
    // the vector which holds all of them. For now I have this organized
    // as a vector for each image
    samples_extracted.push_back(img_samples);
    pixDestroy(&image); // destroy finished image

#ifdef DBG_MEDS_TRAINER
    delete gridviewer;
    gridviewer = NULL;
#endif
    std::cout << "Finished acquiring " << img_samples.size()
         << " samples for image " << finderInfo->getGroundtruthImagePaths()[i] << std::endl;
  }
  if(writeToFile) {
    SampleFileParser::writeSamples(
        finderInfo->getFinderTrainingPaths()->getSampleFilePath(),
        samples_extracted);
  }
}

// When doing training it is necessary to first get all the samples
// up front. This method does feature extraction on all the blobs in
// the given blobinfogrid and returns the feature vector (sample)
// corresponding to each blob. This returns a vector of BLSamples
// (Binary Labeled Samples) which includes both the features and the
// binary label (true for math, false for non-math). The features are
// stored inside a BLOBINFO object as a vector of doubles. What features
// these doubles represent varies based upon what feature extractor
// implementation is being utilized.
std::vector<BLSample*> TrainingSampleExtractor::getGridSamples(BlobDataGrid* const blobDataGrid,
    int image_index) {

  featureExtractor->extractFeatures(blobDataGrid);
  BlobDataGridSearch bdgs(blobDataGrid);
  bdgs.StartFullSearch();
  BlobData* blob = NULL;
  std::vector<BLSample*> samples;
  int blobnum = 0;
  while((blob = bdgs.NextFullSearch()) != NULL) {
    if(blob->getExtractedFeatures().empty()) {
      std::cout << "ERROR: Attempting to create a training sample from a blob "
           << "from which features haven't been extracted!>:-[\n";
      assert(false);
    }
    BLSample* lsample = new BLSample; // labeled sample
    lsample->features = blob->getExtractedFeatures();
    lsample->entry = getBlobGTEntry(blob, image_index, blobDataGrid->getImage());
    TBOX tbox = blob->getBoundingBox();
    lsample->blobbox = M_Utils::tessTBoxToImBox(&tbox, blobDataGrid->getImage());
    if(lsample->entry == NULL)
      lsample->label = false;
    else
      lsample->label = true;
    lsample->imageName = finderInfo->getGroundtruthImagePaths()[image_index].substr(finderInfo->getGroundtruthDirPath().size());
    lsample->imageIndex = image_index;
    assert(DatasetSelectionMenu::getFileNumFromPath(finderInfo->getGroundtruthImagePaths()[image_index]) == image_index); // sanity
    samples.push_back(lsample);
    ++blobnum;
  }
  std::cout << "num blobs (samples) for image " << image_index << ": " << blobnum << std::endl;
  return samples;
}


// If the given blob in the given image is contained within any of the groundtruth
// entry rectangles, then return a pointer to the entry it's contained in. Otherwise
// just return NULL.
GroundTruthEntry* TrainingSampleExtractor::getBlobGTEntry(BlobData* const blob, const int image_index, Pix* const img) {
  // open the groundtruth file
  std::ifstream gtfile;
  std::string gtfilename = finderInfo->getGroundtruthImagePaths()[image_index];
  gtfile.open(gtfilename.c_str(), std::ifstream::in);
  if((gtfile.rdstate() & std::ifstream::failbit) != 0) {
    std::cout << "ERROR: Could not open Groundtruth.dat in " \
         << finderInfo->getGroundtruthDirPath() << std::endl;
    assert(false);
  }
  int max = 55;
  char* curline = new char[max];
  bool found = false;
  GroundTruthEntry* entry = NULL;
  while(!gtfile.eof()) {
    gtfile.getline(curline, max);
    if(curline == NULL)
      continue;
    std::string curlinestr = (std::string)curline;
    assert(curlinestr.length() < max);
    entry = GtParser::parseGTLine(curlinestr);
    if(entry == NULL)
      continue;
    if(entry->image_index == image_index) {
      // see if the entry's rectangle overlaps this blob
      Box* blob_bb = M_Utils::getBlobDataBox(blob, img);
      int bb_intersects = 0; // this will be 1 if blob_bb intersects math
      boxIntersects(entry->rect, blob_bb, &bb_intersects);
      if(bb_intersects == 1) {
        // found a math blob!
        found = true;
      }
    }
    if(found)
      break;
    delete entry; // delete the entry when we're done with it
    entry = NULL;
  }
  delete [] curline;
  curline = NULL;
  gtfile.close();
  return entry;
}

// verifies that the samples extracted are the same as the samples being read
void TrainingSampleExtractor::sampleReadVerify() {
  std::cout << "Verifying that the samples read in from a previous run and the samples "
       << "extracted on the current run are the same.\n";
  std::cout << "-- Reading in the old samples.\n";
  samples_read = SampleFileParser::readOldSamples(finderInfo->getFinderTrainingPaths()->getSampleFilePath(), featureExtractor->getBlobFeatureExtractors());
  std::cout << "-- Extracting features from training set to create new samples.\n";
  getNewSamples(false);
  std::cout << "-- Comparing the read samples to the extracted ones.\n";
  assert(samples_extracted.size() == samples_read.size());
  for(int i = 0; i < samples_extracted.size(); ++i) {
    assert(samples_extracted[i].size() == samples_read[i].size());
    for(int j = 0; j < samples_extracted[i].size(); ++j) {
      BLSample* newsample = samples_extracted[i][j];
      BLSample* oldsample = samples_read[i][j];
      assert(*newsample == *oldsample);
    }
  }
  std::cout << "Success!\n";
  destroySamples(samples_extracted);
  destroySamples(samples_read);
}

void TrainingSampleExtractor::destroySamples(std::vector<std::vector<BLSample*> >& samples) {
  for(int i = 0; i < samples.size(); ++i) {
    for(int j = 0; j < samples[i].size(); ++j) {
      BLSample* sample = samples[i][j];
      if(sample != NULL) {
        delete sample;
        sample = NULL;
      }
    }
    samples[i].clear();
  }
  samples.clear();
}

