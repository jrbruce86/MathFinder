/*
 * MathExpressionFeatureExtractorFactory.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: jake
 */

#include <FeatExtFac.h>

#include <FeatExt.h>
#include <BlobFeatExt.h>
#include <GeometryCat.h>
#include <RecCat.h>
#include <InfoFileParser.h>

#include <string>
#include <vector>

MathExpressionFeatureExtractor* MathExpressionFeatureExtractorFactory
::createMathExpressionFeatureExtractor(FinderInfo* const finderInfo,
    GeometryBasedExtractorCategory* const spatialCategory,
    RecognitionBasedExtractorCategory* const recognitionCategory) {
  // Initialize the feature extractor factories from the finder info
  std::vector<std::string> featureExtractorUniqueNames = finderInfo->getFeatureExtractorUniqueNames();
  std::vector<BlobFeatureExtractorFactory*> featureExtractorFactories;
  for(int i = 0; i < featureExtractorUniqueNames.size(); ++i) {
    std::string uniqueName = stripFeatureFlags(featureExtractorUniqueNames[i]);
    std::vector<std::string> flags = getFeatureFlags(featureExtractorUniqueNames[i]);
    BlobFeatureExtractorFactory* matchingFactory = NULL;
    matchingFactory = spatialCategory->getFactoryFromNameAndFlags(uniqueName, flags);
    if(matchingFactory != NULL) {
      featureExtractorFactories.push_back(matchingFactory);
      continue;
    }
    matchingFactory = recognitionCategory->getFactoryFromNameAndFlags(uniqueName, flags);
    if(matchingFactory != NULL) {
      featureExtractorFactories.push_back(matchingFactory);
      continue;
    }
    std::cout << "ERROR: Unknown feature extractor read in from " << finderInfo->getFinderTrainingPaths()->getInfoFilePath()
        << " while loading Math Finder.\n";
    assert(false);
  }
  return createMathExpressionFeatureExtractor(finderInfo,
      featureExtractorFactories);
}

MathExpressionFeatureExtractor* MathExpressionFeatureExtractorFactory
::createMathExpressionFeatureExtractor(FinderInfo* const finderInfo,
    std::vector<BlobFeatureExtractorFactory*> featureFactories) {

  std::vector<BlobFeatureExtractor*> featureExtractors;
  for(int i = 0; i < featureFactories.size(); ++i) {
    featureExtractors.push_back(featureFactories[i]->create(finderInfo));
  }

  return new MathExpressionFeatureExtractor(finderInfo, featureExtractors);
}


std::string MathExpressionFeatureExtractorFactory::stripFeatureFlags(
    const std::string& uniqueFeatureName) {
  return uniqueFeatureName.substr(
      0,
      uniqueFeatureName.find(TrainingInfoFileParser::FlagDelimiter()));
}

std::vector<std::string> MathExpressionFeatureExtractorFactory::getFeatureFlags(const std::string& uniqueFeatureName) {

  // Check if there are flags or not
  std::size_t flagIndex = uniqueFeatureName.find(TrainingInfoFileParser::FlagDelimiter());
  if(flagIndex == std::string::npos) {
    return std::vector<std::string>(); // no flags, return empty vector
  }

  // Flags found, parse them from the string
  std::vector<std::string> flags;
  do {
    std::size_t prevFlagIndex = flagIndex;
    flagIndex = uniqueFeatureName.find(TrainingInfoFileParser::FlagDelimiter(), prevFlagIndex + 1);
    const int flagLength = TrainingInfoFileParser::FlagDelimiter().size();
    flags.push_back(
        uniqueFeatureName.substr(
            prevFlagIndex + TrainingInfoFileParser::FlagDelimiter().size(),
            (flagIndex == std::string::npos) ?
                std::string::npos // copy to the end
                  : flagIndex - prevFlagIndex - flagLength)); // copy to the start of the next flag
  } while(flagIndex != std::string::npos);

  return flags;
}
