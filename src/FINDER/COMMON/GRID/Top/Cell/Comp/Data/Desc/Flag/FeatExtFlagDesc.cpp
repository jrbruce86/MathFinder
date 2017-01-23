/*
 * FeatureExtractorFlagDescription.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: jake
 */

#include <FeatExtFlagDesc.h>

std::ostream& operator<<(
    std::ostream& stream, FeatureExtractorFlagDescription& f) {
  stream << f.getName() + ":\n" + f.getDescriptionText();
  return stream;
}

FeatureExtractorFlagDescription::
~FeatureExtractorFlagDescription() {};

