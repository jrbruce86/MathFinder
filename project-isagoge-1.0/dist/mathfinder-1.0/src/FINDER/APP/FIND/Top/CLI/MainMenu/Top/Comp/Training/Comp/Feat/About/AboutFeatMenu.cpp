/*
 * AboutFeatureMenu.cpp
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */
#include <AboutFeatMenu.h>

AboutFeatureMenu::AboutFeatureMenu(FeatureSelectionMenuMain* back) {
  this->subMenus.push_back((MenuBase*)back);
}

std::string AboutFeatureMenu::getName() const {
  return "About/help";
}

void AboutFeatureMenu::doTask() {
  std::cout << "When both getting sample data for training and carrying out document analysis on a new page, the first operation carried out by this framework\
is to extract a set of features from each blob (set of connected pixels) in the provided image (or images when training on multiple \
pages). Thus the feature extractor will output a list of normalized features (typically between 0 and 1) for each blob which are fed into the detector\
for classification. Once the detector has finished classifying each blob on the page using the features, the results are passed into the segmentor\
which merges the blobs into complete math expressions. These math expressions should then be ready to be fed into a math-specific OCR engine if all\
goes well.\n\
\n\
For this work, the features implemented can be categorized as either “spatial” or “recognition-based”. While spatial features describe a character's spatial \
relationship with regard to its surrounding characters, recognition-based features are any features that can be gleaned from OCR results.\n\n";
}
