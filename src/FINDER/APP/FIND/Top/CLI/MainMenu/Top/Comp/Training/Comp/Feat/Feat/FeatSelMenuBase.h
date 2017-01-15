/*
 * FeatureSelectionMenuBase.h
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#ifndef FEATURESELECTIONMENUBASE_H_
#define FEATURESELECTIONMENUBASE_H_

#include <MenuBase.h>

#include <vector>

class FeatureSelectionMenuMain;
class BlobFeatureExtractorCategory;
class BlobFeatureExtractorFactory;
class FeatureExtractorFlagDescription;

class FeatureSelectionMenuBase : public virtual MenuBase {

 public:

  FeatureSelectionMenuBase();

  virtual std::string getName() const = 0;

  void doTask();

  // Selects all of the factories
  void selectAllFactories();

 protected:

  virtual BlobFeatureExtractorCategory* getCategory() = 0;

  virtual std::vector<BlobFeatureExtractorFactory*>& getSelectedFactories() = 0;

 private:

  // Add factory to selection after prompting for flags
  void promptToAddFactoryToSelection(
      BlobFeatureExtractorFactory* const factoryToAdd,
      std::vector<BlobFeatureExtractorFactory*>* const selection);

  // Add the factory to the selection with all flags enabled
  void addFactoryToSelectionWithAllFlags(
      BlobFeatureExtractorFactory* const factoryToAdd,
      std::vector<BlobFeatureExtractorFactory*>* const selection);

  // Add factory to selection if it hasn't been added yet
  void addFactoryToSelection(
      BlobFeatureExtractorFactory* const factoryToAdd,
      std::vector<BlobFeatureExtractorFactory*>* const selection);

  // Adds the flag to the fatory if it hasn't been added yet
  void addFlagToFactory(
      FeatureExtractorFlagDescription* const flag,
      BlobFeatureExtractorFactory* const factory);
};


#endif /* FEATURESELECTIONMENUBASE_H_ */
