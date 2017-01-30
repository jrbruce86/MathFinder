/*
 * TrainingMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef TRAININGMENU_H_
#define TRAININGMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>
#include <NameMenu.h>

#include <string>

class NameSelectionMenu;
class FeatureSelectionMenuMain;
class DetectorSelectionMenu;
class SegmentorSelectionMenu;
class DatasetSelectionMenu;

class TrainingMenu : public virtual MenuBase {

 public:

  TrainingMenu(MainMenu* back);

  ~TrainingMenu();

  std::string getName() const;

  static std::string TrainingMenuName();

  std::string getInstructions();

  void doTask();

  FeatureSelectionMenuMain* getFeatureSelectionMenu();
  DetectorSelectionMenu* getDetectorSelectionMenu();
  SegmentorSelectionMenu* getSegmentorSelectionMenu();
  DatasetSelectionMenu* getDatasetSelectionMenu();
  MainMenu* getMainMenu();

 private:

  NameSelectionMenu* nameMenu;
  FeatureSelectionMenuMain* featureSelectionMenu;
  DetectorSelectionMenu* detectorSelectionMenu;
  SegmentorSelectionMenu* segmentorSelectionMenu;
  DatasetSelectionMenu* datasetSelectionMenu;
  MainMenu* mainMenu;
};


#endif /* TRAININGMENU_H_ */
