/*
 * MathExpressionFinderMain.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jake
 */

#include <MathExpressionFinderMain.h>

#include <MathExpressionFinder.h>
#include <MFinderProvider.h>
#include <Utils.h>
#include <MainMenu.h>
#include <Usage.h>
#include <FinderInfo.h>
#include <InfoFileParser.h>
#include <GeometryCat.h>
#include <RecCat.h>

#include <allheaders.h> // leptonica

#include <string>

// for testing
#include <DetMenu.h>
#include <SegMenu.h>
#include <FeatSelMenuMain.h>
#include <FeatExt.h>
#include <FeatExtFac.h>
#include <DatasetMenu.h>
#include <FTPathsFactory.h>
#include <TrainerForMathExpressionFinder.h>
#include <DoTrainingMenu.h>
#include <DetFac.h>
#include <SegFac.h>
#include <baseapi.h>

/**
 * Application point of entry
 */
int main(int argc, char* argv[]) {

  if(argc == 2) {
    if(std::string(argv[1]) == std::string("-m")) { // interactive menu
      runInteractiveMenu();
      return 0;
    } else {
      runFinder(argv[1]); // assume given path as second arg
      return 0;
    }
  } else if(argc == 3) {
    if(std::string(argv[1]) == std::string("-d")) {
      runFinder(argv[2], true); // assume path is third arg
      return 0;
    }
  }
  // if gets here then input wasn't expected
  MathExpressionFinderUsage::printUsage();
}

void runInteractiveMenu() {
  GeometryBasedExtractorCategory spatialCategory;
  RecognitionBasedExtractorCategory recognitionCategory;
  MainMenu* const mainMenu = new MainMenu(
      &spatialCategory,
      &recognitionCategory);
  MenuBase* menuSelected = (MenuBase*)mainMenu;
  while(menuSelected->isNotExit()) {
    menuSelected = menuSelected->getNextSelected();
  }
  delete mainMenu;
}

void runFinder(char* path, bool doJustDetection) {
  const std::string trainedFinderPath =
      FinderTrainingPaths::getTrainedFinderRoot();
  Utils::exec(std::string("mkdir -p ") + trainedFinderPath, true);
  std::vector<std::string> trainedFinders =
      Utils::getFileList(trainedFinderPath);

  if(trainedFinders.empty()) {
    std::cout << "There is currently no trained MathFinder available on the system. "
        << "Loading the interactive menu.\n";
    return runInteractiveMenu();
  }

  std::string finderName;
  if(trainedFinders.size() > 1) {
    std::cout << "More than one MathFinder has been trained on this system. Select one of the following."
        << " To see more information run this program in the interactive menu mode (i.e., MathFinder -m):\n";
    finderName = trainedFinders[Utils::promptSelectStrFromLabeledMatrix(trainedFinders, 2)];
  } else {
    finderName = trainedFinders[0];
  }

  FinderInfo* finderInfo =
      TrainingInfoFileParser().readInfoFromFile(finderName);
  std::string imagePath = std::string(path);
  Pixa* images = pixaCreate(0);
  std::vector<std::string> imageNames;
  // if the image path is a directory, then read in all of the files in that
  // directory (assumes they are images)
  if(Utils::existsDirectory(imagePath)) {
    std::vector<std::string> imagePaths =
        DatasetSelectionMenu::findImagePaths(imagePath);
    for(int i = 0; i < imagePaths.size(); ++i) {
      std::cout << "image path " << imagePaths[i] << std::endl;;
      pixaAddPix(images,
          Utils::leptReadAndBinarizeImg(imagePaths[i]),
          L_INSERT);
      imageNames.push_back(DatasetSelectionMenu::getFileNameFromPath(imagePaths[i]));
    }
  } else if(Utils::existsFile(imagePath)) {
    pixaAddPix(images, Utils::leptReadAndBinarizeImg(imagePath), L_INSERT);
    imageNames.push_back(DatasetSelectionMenu::getFileNameFromPath(imagePath));
  } else {
    std::cout << "Unable to read in the image(s) on the given path." << std::endl;
    return MathExpressionFinderUsage::printUsage();
  }

  GeometryBasedExtractorCategory spatialCategory;
  RecognitionBasedExtractorCategory recognitionCategory;
  MathExpressionFinder* finder =
      MathExpressionFinderProvider().createMathExpressionFinder(
          &spatialCategory,
          &recognitionCategory,
          finderInfo);

  std::vector<MathExpressionFinderResults*> results;
  if(!doJustDetection) {
    results = finder->findMathExpressions(images, imageNames);
  } else {
    results = finder->detectMathExpressions(images, imageNames);
  }

  pixaDestroy(&images); // destroy finished image(s)

  // Display the results
  for(int i = 0; i < results.size(); ++i) {
    results[i]->displaySegmentationResults();
  }

  // Write the results to a directory in the current location (creates
  // the directory)
  std::string resultsDirName = getResultsNameFromPath(imagePath);
  if(doJustDetection) {
    resultsDirName = resultsDirName + "_detection_only";
  }
  MathExpressionFinderResults::printResultsToFiles(results,
      resultsDirName);

  // Destroy results
  for(int i = 0; i < results.size(); ++i) {
    delete results[i];
  }

  // Destroy the finder
  delete finder;
  delete finderInfo;
}

std::string getResultsNameFromPath(std::string path) {
  if(Utils::existsDirectory(path)) {
    if(path.at(path.size() - 1) == '/') {
      path.erase(path.size() - 1, 1);
    }
    return path.substr(path.find_last_of('/') + 1);
  } else {
    return DatasetSelectionMenu::getFileNameFromPath(path);
  }
}

/**
 * Method put in for quickly debugging a trainer (not meant to be invoked
 * outside of development purposes)
 */
void runTrainer() {
  // Testing out features in isolation
  RecognitionBasedExtractorCategory recCategory;
  GeometryBasedExtractorCategory spatialCategory;
  MainMenu mainMenu(&spatialCategory, &recCategory);
  DetectorSelectionMenu detSel(NULL);
  SegmentorSelectionMenu segSel(NULL);
  FeatureSelectionMenuMain featureSelection(NULL, &mainMenu);
  featureSelection.selectAllFeatures();
  std::vector<BlobFeatureExtractorFactory*> featureFactories =
      featureSelection.getSelectedFactories();
  std::string groundtruthDirPath = FinderTrainingPaths::getGroundtruthRoot() + std::string("advcalc1/");
  FinderInfo* finderInfo = FinderInfoBuilder().setFinderName("a")
        ->setFeatureExtractorUniqueNames(DoTrainingMenu::getFeatureExtractorUniqueNames(featureFactories))
        ->setDetectorName(detSel.getDefaultDetectorName())
        ->setSegmentorName(segSel.getDefaultSegmentorName())
        ->setDescription("a")
        ->setGroundtruthName("advcalc1")
        ->setGroundtruthDirPath(groundtruthDirPath)
        ->setGroundtruthFilePath(DatasetSelectionMenu::findGroundtruthFilePath(groundtruthDirPath))
        ->setFinderTrainingPaths(FinderTrainingPathsFactory().createFinderTrainingPaths("a"))
        ->setGroundtruthImagePaths(DatasetSelectionMenu::findGroundtruthImagePaths(groundtruthDirPath))
        ->build();

    // Build the trainer (let the trainer own the finder info, the feature extractor,
    // the detector, and the segmentor and destroy them all when done)

    MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor =
        MathExpressionFeatureExtractorFactory().createMathExpressionFeatureExtractor(
            finderInfo, featureFactories);
    TrainerForMathExpressionFinder* trainer =
        TrainerForMathExpressionFinderFactory().create(
        finderInfo,
        mathExpressionFeatureExtractor);

    // Run the trainer, indicate success/failure
    trainer->runTraining();

    delete trainer;
}

