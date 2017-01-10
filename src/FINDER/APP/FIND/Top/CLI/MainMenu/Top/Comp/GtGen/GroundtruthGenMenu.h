/*
 * GroundtruthGenMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef GROUNDTRUTHGENMENU_H_
#define GROUNDTRUTHGENMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>

#include <string>

class GroundtruthGenMenu : public virtual MenuBase {

 public:

  GroundtruthGenMenu(MainMenu* const back);

  std::string getName() const;

  void doTask();
};



#endif /* GROUNDTRUTHGENMENU_H_ */
