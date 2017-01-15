/*
 * AllFeatMenu.cpp
 *
 *  Created on: Jan 14, 2017
 *      Author: jake
 */

#include <AllFeatMenu.h>

#include <FeatSelMenuMain.h>
#include <SpatialMenu.h>
#include <RecMenu.h>

AllFeatMenu::AllFeatMenu(SpatialFeatureMenu* const spatialMenu,
    RecognitionFeatureMenu* const recMenu,
    FeatureSelectionMenuMain* const back) {
  subMenus.push_back(back);
  this->spatialMenu = spatialMenu;
  this->recMenu = recMenu;
}

std::string AllFeatMenu::getName() const {
  return "Use all features";
}

void AllFeatMenu::doTask() {
  recMenu->selectAllFactories();
  spatialMenu->selectAllFactories();
  std::cout << "All feature extractors have been selected. To see more info, "
      << " navigate to the specific selection menus.\n";
  }
