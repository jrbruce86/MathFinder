/*
 * MainMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#include <MainMenu.h>

#include <MenuBase.h>
#include <AboutMenu.h>
#include <TrainingMenu.h>
#include <FinderInfoMenu.h>
#include <GroundtruthGenMenu.h>
#include <EvaluatorMenu.h>

#include <string>

MainMenu::MainMenu(GeometryBasedExtractorCategory* const spatialCategory,
    RecognitionBasedExtractorCategory* const recCategory) {
  this->spatialCategory = spatialCategory;
  this->recCategory = recCategory;
  subMenus.push_back(new AboutMenu(this));
  subMenus.push_back(new TrainingMenu(this));
  subMenus.push_back(new FinderInfoMenu(this));
  subMenus.push_back(new GroundtruthGenMenu(this));
  subMenus.push_back(new EvaluatorMenu(this));
  subMenus.push_back(new ExitMenu());
}

MainMenu::~MainMenu() {
  for(int i = 0; i < subMenus.size(); ++i) {
    delete subMenus[i];
  }
  subMenus.clear();
}

std::string MainMenu::getName() const {
  return MainMenuName();
}

std::string MainMenu::MainMenuName() {
  return "Main Menu";
}

/**
 * The feature extractor category for spatial feature extractors
 * (manages the memory for the factories and descriptions)
 */
GeometryBasedExtractorCategory* MainMenu::getSpatialCategory() {
  return spatialCategory;
}

/**
 * The feature extractor category for recognition feature extractors
 * (manages the memory for the factories and descriptions)
 */
RecognitionBasedExtractorCategory* MainMenu::getRecognitionCategory() {
  return recCategory;
}

