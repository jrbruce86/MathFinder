/*
 * FinderInfoMenu.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#include <FinderInfoMenu.h>

#include <MainMenu.h>
#include <Utils.h>
#include <FinderTrainingPaths.h>
#include <FinderInfo.h>
#include <InfoFileParser.h>

#include <string>
#include <vector>

FinderInfoMenu::FinderInfoMenu(MainMenu* const back) {
  subMenus.push_back(back);
}

std::string FinderInfoMenu::getName() const {
  return "Info on Trained Math Finders";
}

void FinderInfoMenu::doTask() {
  std::vector<std::string> trainedFinders = Utils::getFileList(FinderTrainingPaths::getTrainingRoot());
  if(trainedFinders.empty()) {
    std::cout << "There are currently no trained finders on the system. To train a new one select the training menu.\n";
    return;
  }
  while(true) {
    std::cout << "To see information on a Math Finder, type its index in the below menu and press enter.\n";
    const int selectedIndex=  Utils::promptSelectStrFromLabeledMatrix(trainedFinders, 3);
    FinderInfo* finderInfo = TrainingInfoFileParser().readInfoFromFile(trainedFinders[selectedIndex]);
    finderInfo->displayInformation();
    std::cout << "\n\nThe information for the selected finder is displayed above. Select another? ";
    if(!Utils::promptYesNo()) {
      break;
    }
  }
}

