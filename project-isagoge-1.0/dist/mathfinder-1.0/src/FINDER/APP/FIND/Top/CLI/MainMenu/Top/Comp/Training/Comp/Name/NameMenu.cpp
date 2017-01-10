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

#include <string>

NameSelectionMenu::NameSelectionMenu(TrainingMenu* const back)
: finderName("") {
  subMenus.push_back(back);
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
    if(this->finderName.empty() || enterNew) {
      this->finderName = Utils::promptForValueNotOnList(
          "Enter a name for this finder: ",
          Utils::getFileList(FinderTrainingPaths::getTrainingRoot()));
    }
    std::cout << "You have selected '" << this->finderName << "' as the name. This ok? ";
    enterNew = !Utils::promptYesNo();
  } while (enterNew);


}

std::string NameSelectionMenu::getFinderName() {
  return finderName;
}

