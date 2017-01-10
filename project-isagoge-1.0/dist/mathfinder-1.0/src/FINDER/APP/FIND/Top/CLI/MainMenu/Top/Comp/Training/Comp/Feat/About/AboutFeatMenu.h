/*
 * AboutFeatureMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef ABOUTFEATUREMENU_H_
#define ABOUTFEATUREMENU_H_

#include <MenuBase.h>

#include <string>

class FeatureSelectionMenuMain;

class AboutFeatureMenu : public virtual MenuBase {

 public:

  AboutFeatureMenu(FeatureSelectionMenuMain* back);

  std::string getName() const;

  void doTask();
};


#endif /* ABOUTFEATUREMENU_H_ */
