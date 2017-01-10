/*
 * FeatureSelectionMenuMain.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <FeatSelMenuMain.h>

#include <TrainingMenu.h>
#include <SpatialMenu.h>
#include <RecMenu.h>
#include <AboutFeatMenu.h>

#include <BlobFeatExtFac.h>

#include <string>
#include <vector>

FeatureSelectionMenuMain::FeatureSelectionMenuMain(
    TrainingMenu* trainingMain) {
  this->spatialMenu = new SpatialFeatureMenu(this);
  this->recognitionMenu = new RecognitionFeatureMenu(this);

  this->subMenus.push_back(new AboutFeatureMenu(this));
  this->subMenus.push_back(this->spatialMenu);
  this->subMenus.push_back(this->recognitionMenu);
  this->subMenus.push_back(trainingMain);
}

FeatureSelectionMenuMain::~FeatureSelectionMenuMain() {
  for(int i = 0; i < subMenus.size(); ++i) {
    if(subMenus[i]->getName() != TrainingMenu::TrainingMenuName()) {
      delete subMenus[i];
    }
  }
  subMenus.clear();
}

std::string FeatureSelectionMenuMain::getName() const {
  return "Choose the blob feature extractors";
}

/**
 * Combine the selected factories from all menus into a single list and return it
 */
std::vector<BlobFeatureExtractorFactory*> FeatureSelectionMenuMain::getSelectedFactories() {
  // Grab the factories
  const std::vector<BlobFeatureExtractorFactory*> spatialFactories =
      spatialMenu->getSelectedFactoriesCopy();
  const std::vector<BlobFeatureExtractorFactory*> recognitionFactories =
      recognitionMenu->getSelectedFactoriesCopy();

  // Combine the factories from all menus into one vector
  std::vector<BlobFeatureExtractorFactory*> allFactories;
  for(int i = 0; i < spatialFactories.size(); ++i) {
    allFactories.push_back(spatialFactories[i]);
  }
  for(int i = 0; i < recognitionFactories.size(); ++i) {
    allFactories.push_back(recognitionFactories[i]);
  }

  // Return all of the factories
  return allFactories;
}
