/*
 * SpatialFeatureMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef SPATIALFEATUREMENU_H_
#define SPATIALFEATUREMENU_H_

#include <vector>

#include <FeatSelMenuBase.h>

class MainMenu;
class FeatureSelectionMenuMain;
class GeometryBasedExtractorCategory;
class BlobFeatureExtractorCategory;
class BlobFeatureExtractorFactory;

class SpatialFeatureMenu : public virtual FeatureSelectionMenuBase {

 public:

  SpatialFeatureMenu(FeatureSelectionMenuMain* const back,
      MainMenu* const mainMenu);

  std::string getName() const;

  /**
   * Returns copy of the selected factories list of pointers. Doesn't allow modifying
   * internal list as this creates a copy of it
   */
  std::vector<BlobFeatureExtractorFactory*> getSelectedFactoriesCopy();

 protected:

  BlobFeatureExtractorCategory* getCategory();

  /**
   * Returns reference to the list of selected factories (allows base
   * class to modify)
   */
  std::vector<BlobFeatureExtractorFactory*>& getSelectedFactories();

 private:

  GeometryBasedExtractorCategory* spatialCategory;

  std::vector<BlobFeatureExtractorFactory*> selectedFactories;
};


#endif /* SPATIALFEATUREMENU_H_ */
