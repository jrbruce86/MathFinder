/*
 * FeatureExtractorFlagDescription.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef FEATUREEXTRACTORFLAGDESCRIPTION_H_
#define FEATUREEXTRACTORFLAGDESCRIPTION_H_

#include <string>

/**
 * Describes flags for a feature detector which can be either
 * enabled or disabled to alter behavior.
 */
class FeatureExtractorFlagDescription {

 public:

  /**
   * The name of this flag
   */
  virtual std::string getName() = 0;

  /**
   * Brief description for the intent of this flag
   */
  virtual std::string getDescriptionText() = 0;

  virtual ~FeatureExtractorFlagDescription();

  friend std::ostream& operator<<(
      std::ostream& stream, FeatureExtractorFlagDescription& f);
};


#endif /* FEATUREEXTRACTORFLAGDESCRIPTION_H_ */
