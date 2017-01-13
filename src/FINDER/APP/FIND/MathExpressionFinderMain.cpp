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

#include <allheaders.h> // leptonica

#include <string>


// TODO: IMPORTANT: MAKE SURE THAT THE INSTALL FOR THIS APPLICATION
//       CREATES A SUBDIR IN /usr/bin and adds that subdir to the path
//       in ~/.bashrc. Also make sure to add in the tesserct stuff to
//       ~/.bashrc. This should all be automated. The subdir should include
//       the equation_labeler executable among anything else, maybe tesseract
//       too might as well...

/**
 * Application point of entry
 */
int main(int argc, char* argv[]) {

//  {
//    TessBaseAPI api;
//    Pix* p = Utils::leptReadImg("/home/jake/Desktop/a/0.png");
//    BlobDataGrid* g =
//        BlobDataGridFactory().createBlobDataGrid(p, &api, "0.png");
//
//  }

  if(argc == 2) {
    if(std::string(argv[1]) == std::string("-menu")) { // interactive menu
      runInteractiveMenu();
    } else {
      runFinder(argv[1]);
    }
  } else {
    MathExpressionFinderUsage::printUsage();
  }
}

void runInteractiveMenu() {
  MainMenu* const mainMenu = new MainMenu();
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
    finderName = trainedFinders[Utils::promptSelectStrFromLabeledMatrix(trainedFinders, 3)];
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
    std::cout << "Unable to read in the image(s) on the given path." << endl;
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

