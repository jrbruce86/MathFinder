/*
 * AllFeatMenu.h
 *
 *  Created on: Jan 14, 2017
 *      Author: jake
 */

#ifndef ALLFEATMENU_H_
#define ALLFEATMENU_H_

#include <MenuBase.h>

class FeatureSelectionMenuMain;
class SpatialFeatureMenu;
class RecognitionFeatureMenu;

class AllFeatMenu : public virtual MenuBase {

 public:

  AllFeatMenu(SpatialFeatureMenu* const spatialMenu,
      RecognitionFeatureMenu* const recMenu,
      FeatureSelectionMenuMain* const back);

  std::string getName() const;

  void doTask();

 private:

  SpatialFeatureMenu* spatialMenu;
  RecognitionFeatureMenu* recMenu;
};


#endif /* ALLFEATMENU_H_ */
