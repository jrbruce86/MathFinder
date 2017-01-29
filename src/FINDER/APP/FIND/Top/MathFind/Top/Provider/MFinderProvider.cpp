/*
 * MathExpressionFeatureExtractorFactoryProvider.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: jake
 */

#include <MFinderProvider.h>

#include <MathExpressionFinder.h>
#include <FinderInfo.h>
#include <GeometryCat.h>
#include <RecCat.h>
#include <BlobFeatExtFac.h>
#include <BlobDataGridFactory.h>
#include <FeatExtFac.h>
#include <DetFac.h>
#include <SegFac.h>
#include <InfoFileParser.h>

#include <vector>
#include <assert.h>
#include <string>

MathExpressionFinder* MathExpressionFinderProvider
::createMathExpressionFinder(GeometryBasedExtractorCategory* const spatialCategory,
    RecognitionBasedExtractorCategory* const recognitionCategory,
    FinderInfo* const finderInfo) {

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

  MathExpressionFeatureExtractor* const mathExpressionFeatureExtractor =
      MathExpressionFeatureExtractorFactory().createMathExpressionFeatureExtractor(
          finderInfo, featureExtractorFactories);

  return new MathExpressionFinder(
      mathExpressionFeatureExtractor,
      MathExpressionDetectorFactory().createMathExpressionDetector(finderInfo),
      MathExpressionSegmentorFactory().createMathExpressionSegmentor(finderInfo, mathExpressionFeatureExtractor));
}

std::string MathExpressionFinderProvider::stripFeatureFlags(
    const std::string& uniqueFeatureName) {
  return uniqueFeatureName.substr(
      0,
      uniqueFeatureName.find(TrainingInfoFileParser::FlagDelimiter()));
}

std::vector<std::string> MathExpressionFinderProvider::getFeatureFlags(const std::string& uniqueFeatureName) {

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
