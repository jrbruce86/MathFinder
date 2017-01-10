/*
 * EvaluatorMenu.h
 *
 *  Created on: Dec 14, 2016
 *      Author: jake
 */

#ifndef EVALUATORMENU_H_
#define EVALUATORMENU_H_

#include <MenuBase.h>
#include <MainMenu.h>

#include <string>

class EvaluatorMenu : public virtual MenuBase {

 public:

  EvaluatorMenu(MainMenu* const back);

  std::string getName() const;

  void doTask();
};


#endif /* EVALUATORMENU_H_ */
