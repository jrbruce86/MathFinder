/*
 * RecognitionFeatureMenu.h
 *
 *  Created on: Dec 16, 2016
 *      Author: jake
 */

#ifndef RECOGNITIONFEATUREMENU_H_
#define RECOGNITIONFEATUREMENU_H_

#include <FeatSelMenuBase.h>

#include <string>

class FeatureSelectionMenuMain;

class BlobFeatureExtractorCategory;
class BlobFeatureExtractorFactory;
class RecognitionBasedExtractorCategory;

class RecognitionFeatureMenu : public virtual FeatureSelectionMenuBase {

 public:

  RecognitionFeatureMenu(FeatureSelectionMenuMain* back);

  ~RecognitionFeatureMenu();

  std::string getName() const;

  /**
   * Returns copy of the selected factories list of pointers. Doesn't allow modifying
   * internal list as this creates a copy of it
   */
  std::vector<BlobFeatureExtractorFactory*> getSelectedFactoriesCopy();

 protected:

  BlobFeatureExtractorCategory* getCategory();

  std::vector<BlobFeatureExtractorFactory*>& getSelectedFactories();

 private:

  RecognitionBasedExtractorCategory* recognitionCategory;

  std::vector<BlobFeatureExtractorFactory*> selectedFactories;
};


#endif /* RECOGNITIONFEATUREMENU_H_ */
