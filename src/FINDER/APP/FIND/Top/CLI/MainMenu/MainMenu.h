/*
 * MainMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include <MenuBase.h>

#include <string>

class GeometryBasedExtractorCategory;
class RecognitionBasedExtractorCategory;

class MainMenu : public virtual MenuBase {

 public:

  MainMenu(GeometryBasedExtractorCategory* const spatialCategory,
      RecognitionBasedExtractorCategory* const recCategory);

  ~MainMenu();

  std::string getName() const;

  static std::string MainMenuName();

  /**
   * The feature extractor category for spatial feature extractors
   * (manages the memory for the factories and descriptions)
   */
  GeometryBasedExtractorCategory* getSpatialCategory();

  /**
   * The feature extractor category for recognition feature extractors
   * (manages the memory for the factories and descriptions)
   */
  RecognitionBasedExtractorCategory* getRecognitionCategory();

 private:

  // The feature extraction categories (manages memory for feature extractor
  // factories and the feature extractor descriptions
  GeometryBasedExtractorCategory* spatialCategory;
  RecognitionBasedExtractorCategory* recCategory;
};


#endif /* MAINMENU_H_ */
