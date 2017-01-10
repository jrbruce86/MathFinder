/*
 * DoTrainingMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */
#include <DoTrainingMenu.h>

#include <TrainingMenu.h>
#include <NameMenu.h>
#include <FeatSelMenuMain.h>
#include <DetMenu.h>
#include <SegMenu.h>
#include <DatasetMenu.h>

#include <FeatExt.h>
#include <FeatExtFac.h>
#include <FinderInfo.h>
#include <FTPathsFactory.h>
#include <InfoFileParser.h>
#include <TrainerForMathExpressionFinder.h>
#include <DetFac.h>
#include <SegFac.h>
#include <BlobFeatExtFac.h>
#include <FeatExtFlagDesc.h>

#include <fstream>
#include <string>
#include <vector>

DoTrainingMenu::DoTrainingMenu(
    TrainingMenu* const back,
    NameSelectionMenu* const nameSelection,
    FeatureSelectionMenuMain* const featureSelection,
    DetectorSelectionMenu* const detectorSelection,
    SegmentorSelectionMenu* const segmentorSelection,
    DatasetSelectionMenu* const datasetSelection) {

  // Set up the menu
  subMenus.push_back(back);

  // Grab pointers to the menus needed for building the trainer
  this->nameSelection = nameSelection;
  this->featureSelection = featureSelection;
  this->detectorSelection = detectorSelection;
  this->segmentorSelection = segmentorSelection;
  this->datasetSelection = datasetSelection;
}

std::string DoTrainingMenu::getName() const {
  return "Do the training";
}

void DoTrainingMenu::doTask() {

  // Populate what was entered
  std::string finderName = nameSelection->getFinderName();
  std::vector<BlobFeatureExtractorFactory*> featureFactories =
      featureSelection->getSelectedFactories();
  std::string detectorName = detectorSelection->getSelectedDetectorName();
  std::string segmentorName = segmentorSelection->getSelectedSegmentorName();
  std::string groundtruthName = datasetSelection->getGroundtruthName();
  std::string groundtruthDirPath = datasetSelection->getGroundtruthDirPath();
  std::string groundtruthFilePath = datasetSelection->getGroundtruthFilePath();
  std::vector<std::string> groundtruthImagePaths = datasetSelection->getGroundtruthImagePaths();

  // Make sure all of the necessary info has been entered
  while(finderName == "") {
    nameSelection->doTask();
    finderName = nameSelection->getFinderName();
  }
  if(featureFactories.size() < 1) {
    std::cout << "No features have been selected for this trainer. "
        << "Need to select one or more features to extract by "
         <<   "returning to the previous menu and selecting features to use. \n";
    return;
  }
  if(detectorSelection->getSelectedDetectorName() == "") {
    detectorSelection->doTask();
    detectorName = detectorSelection->getSelectedDetectorName();
  }
  if(segmentorSelection->getSelectedSegmentorName() == "") {
    segmentorSelection->doTask();
    segmentorName = segmentorSelection->getSelectedSegmentorName();
  }
  if(groundtruthName == "" || groundtruthDirPath == "" || groundtruthFilePath == ""
      || groundtruthImagePaths.empty()) {
    datasetSelection->doTask();
    if(!datasetSelection->isComplete()) {
      return;
    }
    groundtruthName = datasetSelection->getGroundtruthName();
    groundtruthDirPath = datasetSelection->getGroundtruthDirPath();
    groundtruthFilePath = datasetSelection->getGroundtruthFilePath();
    groundtruthImagePaths = datasetSelection->getGroundtruthImagePaths();
  }

  // Prompt for a brief description about the finder being trained
  bool descriptionDone = false;
  std::string description = "";
  while(true) {
    std::cout << "Type in an optional brief description on the math expression finder being trained and press enter when finished (or just press enter if description N/A): \n";
    std::cin.ignore();
    std::getline(std::cin, description);
    if(description == "") {
      break;
    }
    std::cout << "The description you entered shown below will be stored with other info on the Math Finder being trained.\n" <<
        description << std::endl;
    std::cout << "Ok to continue? ";
    if(Utils::promptYesNo()) {
      break;
    }
  }

  // Build the finder info
  FinderInfo* finderInfo = FinderInfoBuilder().setFinderName(finderName)
      ->setFeatureExtractorUniqueNames(getFeatureExtractorUniqueNames(featureFactories))
      ->setDetectorName(detectorName)
      ->setSegmentorName(segmentorName)
      ->setDescription(description)
      ->setGroundtruthName(groundtruthName)
      ->setGroundtruthDirPath(groundtruthDirPath)
      ->setGroundtruthFilePath(groundtruthFilePath)
      ->setFinderTrainingPaths(FinderTrainingPathsFactory().createFinderTrainingPaths(finderName))
      ->setGroundtruthImagePaths(groundtruthImagePaths)
      ->build();

  TrainingInfoFileParser().writeInfoToFile(finderInfo);

  // Build the trainer (let the trainer own the finder info, the feature extractor,
  // the detector, and the segmentor and destroy them all when done)
  MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor =
      MathExpressionFeatureExtractorFactory().createMathExpressionFeatureExtractor(
          finderInfo, featureFactories);
  TrainerForMathExpressionFinder trainer(
      finderInfo,
      mathExpressionFeatureExtractor,
      MathExpressionDetectorFactory().createMathExpressionDetector(finderInfo),
      MathExpressionSegmentorFactory().createMathExpressionSegmentor(finderInfo, mathExpressionFeatureExtractor));

  // Run the trainer, indicate success/failure
  trainer.runTraining();

  std::cout << "All resources generated during training were written to "
      << finderInfo->getFinderTrainingPaths()->getTrainingDirPath() << "."
      << " Please do not modify those directories. A copy of the info file "
      << "for the trained Finder is located at " << finderInfo->getFinderTrainingPaths()->getInfoFilePath()
      << ". To run this trained Finder, restart the application with the command line argument being the path to the image(s) to test. "
      << "If there is more than one trained Finder then the application will prompt to select one. "
      << "The name of the Finder that was just trained was set to " << finderInfo->getFinderName()
      << ", so if prompted and you want to test this Finder then select " << finderInfo->getFinderName() << ".\n";
}

std::vector<std::string> DoTrainingMenu::getFeatureExtractorUniqueNames(
    const std::vector<BlobFeatureExtractorFactory*>& featureFactories) {
  std::vector<std::string> retVec;
  for(int i = 0; i < featureFactories.size(); ++i) {
    std::string uniqueName = featureFactories[i]->getDescription()->getUniqueName();
    // append the flags to the unique name if there are any
    std::vector<FeatureExtractorFlagDescription*> flags = featureFactories[i]->getSelectedFlags();
    for(int j = 0; j < flags.size(); ++j) {
      uniqueName.append(TrainingInfoFileParser::FlagDelimiter() + flags[j]->getName());
    }
    retVec.push_back(uniqueName);
  }
  return retVec;
}
