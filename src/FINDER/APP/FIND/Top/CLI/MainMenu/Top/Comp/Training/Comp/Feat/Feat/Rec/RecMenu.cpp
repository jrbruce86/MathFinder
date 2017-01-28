/*
 * RecognitionFeatureMenu.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#include <RecMenu.h>

#include <MainMenu.h>
#include <FeatSelMenuMain.h>
#include <RecCat.h>

#include <vector>
#include <string>

RecognitionFeatureMenu::RecognitionFeatureMenu(
    FeatureSelectionMenuMain* const back,
    MainMenu* const mainMenu) {
  this->subMenus.push_back(back);
  this->recognitionCategory = mainMenu->getRecognitionCategory();
}

std::string RecognitionFeatureMenu::getName() const {
  return "Recognition-based Features Menu";
}

BlobFeatureExtractorCategory* RecognitionFeatureMenu::getCategory() {
  return recognitionCategory;
}

std::vector<BlobFeatureExtractorFactory*>& RecognitionFeatureMenu::getSelectedFactories() {
  return selectedFactories;
}

std::vector<BlobFeatureExtractorFactory*> RecognitionFeatureMenu::getSelectedFactoriesCopy() {
  return selectedFactories;
}


