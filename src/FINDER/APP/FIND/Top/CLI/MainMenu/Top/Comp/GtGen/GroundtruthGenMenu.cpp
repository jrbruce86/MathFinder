/*
 * GroundtruthGenMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <GroundtruthGenMenu.h>

#include <MainMenu.h>
#include <Utils.h>

#include <string>
#include <iostream>


GroundtruthGenMenu::GroundtruthGenMenu(MainMenu* const back) {
  subMenus.push_back(back);
}

std::string GroundtruthGenMenu::getName() const {
  return "Groundtruth Generation Tool";
}

void GroundtruthGenMenu::doTask() {
  std::cout << "Start the Groundtruth Generation Tool? ";
  if(Utils::promptYesNo()) {
    Utils::exec("MathFinderGtGen &");
  }
}

