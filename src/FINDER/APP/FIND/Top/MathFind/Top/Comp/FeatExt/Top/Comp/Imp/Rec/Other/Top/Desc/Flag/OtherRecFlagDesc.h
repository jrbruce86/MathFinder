/*
 * OtherRecognitionFeatureExtractorFlagDescriptions.h
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#ifndef OTHERRECOGNITIONFEATUREEXTRACTORFLAGDESCRIPTIONS_H_
#define OTHERRECOGNITIONFEATUREEXTRACTORFLAGDESCRIPTIONS_H_

#include <FeatExtFlagDesc.h>

#include <string>

class VerticalDistanceAboveRowBaselineFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class HeightToAverageHeightRatioFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class WidthHeightRatioToAverageRatioFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class IsPartOfRecognizedMathSymbolFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class IsItalicFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class ConfidenceToAverageRatioFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class IsValidAccordingToTesseractFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class BelongsToRowWithRecognizedTextFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class BelongsToBadPageFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

class BelongsToStopWordFlagDescription
: public virtual FeatureExtractorFlagDescription {

 public:

  std::string getName();

  std::string getDescriptionText();
};

#endif /* OTHERRECOGNITIONFEATUREEXTRACTORFLAGDESCRIPTIONS_H_ */
