/*
 * NameSelectionMenu.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#include <NameMenu.h>

#include <TrainingMenu.h>
#include <FinderTrainingPaths.h>
#include <Utils.h>
#include <InfoFileParser.h>
#include <TrainerForMathExpressionFinder.h>

#include <string>

NameSelectionMenu::NameSelectionMenu(TrainingMenu* const back)
: finderName("") {
  subMenus.push_back(back);
  this->trainingMenu = back;
}

std::string NameSelectionMenu::getName() const {
  return "Edit finder name";
}

void NameSelectionMenu::doTask() {
  promptSetFinderName();
}

void NameSelectionMenu::promptSetFinderName() {
  bool enterNew = false;
  do {
    if(finderName.empty() || enterNew) {
      std::cout << "Enter a name for this finder: ";
      finderName.clear();
      std::cin >> finderName;
      if(Utils::isStrOnList(
          finderName,
          Utils::getFileList(
              FinderTrainingPaths::getTrainedFinderRoot()))) {
        std::cout << "The name you have chosen is already in use. " <<
            "Press enter to see info on it.\n";
        Utils::waitForInput();
        std::string infoPath = Utils::checkTrailingSlash(
            FinderTrainingPaths::getTrainedFinderRoot()) +
            finderName + "/info";
        std::ifstream strm(infoPath.c_str());
        if(strm.is_open()) {
          std::cout << strm.rdbuf() << std::endl;
        } else {
          std::cout << "Error couldn't open the info file. "
              << "The finder you entered exists but is corrupted. "
              << "Try another.\n";
          enterNew = true;
          continue;
        }
        std::cout << "The information should be shown above. "
            << "Press enter to continue.\n";
        Utils::waitForInput();
        std::cout << "Enter 'y' if you would like to re-train the Finder "
            << "whose info is shown above to meet the same specifications "
            << "as shown. Enter 'n' if you'd like to instead train a new one "
            << "or re-train a different one. ";
        if(!Utils::promptYesNo()) {
          enterNew = true;
          continue;
        } else {
          std::cout << "About to start training for the finder shown above. "
              << "Are you sure you would like to continue?\n";
          if(!Utils::promptYesNo()) {
            enterNew = true;
            continue;
          }
          FinderInfo* finderInfo =
              TrainingInfoFileParser().readInfoFromFile(finderName);
          TrainerForMathExpressionFinder* trainer =
              TrainerForMathExpressionFinderFactory().create(finderInfo,
                  trainingMenu->getMainMenu()->getSpatialCategory(),
                  trainingMenu->getMainMenu()->getRecognitionCategory());
          trainer->runTraining();
          std::cout << "Completed training. Press enter to return to main menu.\n";
          Utils::waitForInput();
          return;
        }
      }
    }
    std::cout << "You have selected '" << this->finderName << "' as the name. This ok? ";
    enterNew = !Utils::promptYesNo();
  } while (enterNew);
}

std::string NameSelectionMenu::getFinderName() {
  return finderName;
}

