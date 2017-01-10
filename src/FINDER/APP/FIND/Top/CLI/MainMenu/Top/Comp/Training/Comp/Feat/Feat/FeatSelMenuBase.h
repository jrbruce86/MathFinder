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

class FeatureSelectionMenuBase : public virtual MenuBase {

 public:

  FeatureSelectionMenuBase();

  virtual std::string getName() const = 0;

  void doTask();

 protected:

  virtual BlobFeatureExtractorCategory* getCategory() = 0;

  virtual std::vector<BlobFeatureExtractorFactory*>& getSelectedFactories() = 0;

 private:

  void addFactoryToSelection(
      BlobFeatureExtractorFactory* const factoryToAdd,
      std::vector<BlobFeatureExtractorFactory*>* const selection);
};


#endif /* FEATURESELECTIONMENUBASE_H_ */
