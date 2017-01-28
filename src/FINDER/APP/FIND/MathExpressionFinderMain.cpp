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
    } else {
      runFinder(argv[1]);
    }
  } else {
    MathExpressionFinderUsage::printUsage();
  }

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

void runFinder(char* path) {

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
        << " To see more information run this program in the interactive menu mode (i.e., MathFinder -menu):\n";
    finderName = trainedFinders[Utils::promptSelectStrFromLabeledMatrix(trainedFinders, 2)];
  } else {
    finderName = trainedFinders[0];
  }

  FinderInfo* finderInfo =
      TrainingInfoFileParser().readInfoFromFile(finderName);

  std::string imagePath = std::string(path);
  Pixa* images = pixaCreate(0);
  // if the image path is a directory, then read in all of the files in that
  // directory (assumes they are images)
  if(Utils::existsDirectory(imagePath)) {
    std::vector<std::string> imageNames = Utils::getFileList(imagePath);
    for(int i = 0; i < imageNames.size(); ++i) {
      pixaAddPix(images, Utils::leptReadImg(imagePath), L_INSERT);
    }
  } else if(Utils::existsFile(imagePath)) {
    pixaAddPix(images, Utils::leptReadImg(imagePath), L_INSERT);
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

  std::vector<MathExpressionFinderResults*> results =
      finder->findMathExpressions(images, Utils::getFileList(imagePath));

  pixaDestroy(&images); // destroy finished image(s)

  // Destroy the finder
  delete finder;
  delete finderInfo;

  // Display the results and/or write to path
  for(int i = 0; i < results.size(); ++i) {
    results[i]->displayResults();
    results[i]->printResultsToFiles();
  }

  // Destroy results
  for(int i = 0; i < results.size(); ++i) {
    delete results[i];
  }
}

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
    TrainerForMathExpressionFinder trainer(
        finderInfo,
        mathExpressionFeatureExtractor,
        MathExpressionDetectorFactory().createMathExpressionDetector(finderInfo),
        MathExpressionSegmentorFactory().createMathExpressionSegmentor(
            finderInfo,
            mathExpressionFeatureExtractor));

    // Run the trainer, indicate success/failure
    trainer.runTraining();
}

