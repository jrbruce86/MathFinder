/*
 * SpatialFeatureMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <SpatialMenu.h>

#include <MainMenu.h>
#include <FeatSelMenuMain.h>
#include <BlobFeatExtCat.h>
#include <GeometryCat.h>
#include <BlobFeatExtFac.h>

#include <vector>

SpatialFeatureMenu::SpatialFeatureMenu(
    FeatureSelectionMenuMain* back,
    MainMenu* mainMenu) {
  this->subMenus.push_back(back);
  this->spatialCategory = mainMenu->getSpatialCategory();
}

std::string SpatialFeatureMenu::getName() const {
  return "Spatial-based Features Menu";
}

BlobFeatureExtractorCategory* SpatialFeatureMenu::getCategory() {
  return spatialCategory;
}

std::vector<BlobFeatureExtractorFactory*>& SpatialFeatureMenu::getSelectedFactories() {
  return selectedFactories;
}

std::vector<BlobFeatureExtractorFactory*> SpatialFeatureMenu::getSelectedFactoriesCopy() {
  return selectedFactories;
}
