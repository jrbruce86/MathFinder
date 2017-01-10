/*
 * DoTrainingMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef DOTRAININGMENU_H_
#define DOTRAININGMENU_H_

#include <MenuBase.h>

#include <BlobFeatExtFac.h>

class TrainingMenu;
class NameSelectionMenu;
class FeatureSelectionMenuMain;
class DetectorSelectionMenu;
class SegmentorSelectionMenu;
class DatasetSelectionMenu;

class DoTrainingMenu : public virtual MenuBase {

 public:

  DoTrainingMenu(TrainingMenu* const back,
      NameSelectionMenu* const nameSelection,
      FeatureSelectionMenuMain* const featureSelection,
      DetectorSelectionMenu* const detectorSelection,
      SegmentorSelectionMenu* const segmentorSelection,
      DatasetSelectionMenu* const datasetSelection);

  std::string getName() const;

  void doTask();

 private:

  std::vector<std::string> getFeatureExtractorUniqueNames(
      const std::vector<BlobFeatureExtractorFactory*>& factories);

  NameSelectionMenu* nameSelection;
  FeatureSelectionMenuMain* featureSelection;
  DetectorSelectionMenu* detectorSelection;
  SegmentorSelectionMenu* segmentorSelection;
  DatasetSelectionMenu* datasetSelection;
};


#endif /* DOTRAININGMENU_H_ */
