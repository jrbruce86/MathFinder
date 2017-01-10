/*
 * RecognitionFeatureMenu.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#include <RecMenu.h>

#include <FeatSelMenuMain.h>
#include <RecCat.h>

#include <vector>
#include <string>

RecognitionFeatureMenu::RecognitionFeatureMenu(
    FeatureSelectionMenuMain* back) {
  this->subMenus.push_back(back);
  this->recognitionCategory = new RecognitionBasedExtractorCategory();
}

RecognitionFeatureMenu::~RecognitionFeatureMenu() {
  delete recognitionCategory;
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


