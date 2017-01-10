/*
 * SentenceNGramsFeatureExtractor.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef SENTENCENGRAMSFEATUREEXTRACTOR_H_
#define SENTENCENGRAMSFEATUREEXTRACTOR_H_

#include <BlobFeatExt.h>
#include <NGDesc.h>
#include <FinderInfo.h>
#include <BlobDataGrid.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>
#include <NGram.h>
#include <SentenceData.h>
#include <NGramRanker.h>
#include <StopwordHelper.h>
#include <NGDesc.h>

#include <vector>
#include <string>

class SentenceNGramsFeatureExtractor : public BlobFeatureExtractor {

 public:

  SentenceNGramsFeatureExtractor(
      SentenceNGramsFeatureExtractorDescription* description,
      FinderInfo* const finderInfo);

  ~SentenceNGramsFeatureExtractor();

  void doTrainerInitialization();

  void doFinderInitialization();

  void doPreprocessing(BlobDataGrid* const blobDataGrid);

  std::vector<DoubleFeature*> extractFeatures(BlobData* const blob);

  BlobFeatureExtractorDescription* getFeatureExtractorDescription();

  std::vector<FeatureExtractorFlagDescription*> getEnabledFlagDescriptions();

  void enableUnigramFlag();
  void enableBigramFlag();
  void enableTrigramFlag();

 private:

  void dbgDisplayNGrams(const RankedNGramVecs& ngrams);

  double getNGFeature(
      TesseractSentenceData* const sentence,
      const int gram);

  double findNGProfileMatch(
      NGramFrequency* const ngramfreq,
      RankedNGramVec profile,
      const int gram);

  double scaleNGramFeature(
      double ng_feat,
      RankedNGramVec profile);

  TesseractSentenceData* getBlobSentence(
      BlobData* const blobData);

  FinderInfo* finderInfo;
  NGramRanker* ngramRanker;
  StopwordFileReader* stopwordHelper;
  RankedNGramVecs mathNGramProfile;
  std::string ngramdir;
  SentenceNGramsFeatureExtractorDescription* description;
  std::vector<FeatureExtractorFlagDescription*> enabledFlagDescriptions;

  bool isUnigramFlagEnabled;
  bool isBigramFlagEnabled;
  bool isTrigramFlagEnabled;
};


#endif /* SENTENCENGRAMSFEATUREEXTRACTOR_H_ */
