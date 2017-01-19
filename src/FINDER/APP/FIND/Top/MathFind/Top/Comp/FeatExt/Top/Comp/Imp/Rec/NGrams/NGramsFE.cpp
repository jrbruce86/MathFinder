/*
 * SentenceNGramsFeatureExtractor.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#include <NGramsFE.h>

#include <NGDesc.h>
#include <FinderInfo.h>
#include <NGram.h>
#include <NGramRanker.h>
#include <Utils.h>
#include <NGProfile.h>
#include <BlobDataGrid.h>
#include <SentenceData.h>
#include <M_Utils.h>
#include <DoubleFeature.h>
#include <BlobData.h>
#include <BlockData.h>
#include <WordData.h>
#include <BlobFeatExtDesc.h>
#include <FeatExtFlagDesc.h>

#include <baseapi.h>

#include <iostream>
#include <stddef.h>
#include <assert.h>
#include <vector>

#define DBG_DISPLAY_NG_PROFILE
#define DBG_SHOW_NGRAMS
#define DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE

SentenceNGramsFeatureExtractor::SentenceNGramsFeatureExtractor(
     SentenceNGramsFeatureExtractorDescription* const description,
     FinderInfo* const finderInfo)
: isUnigramFlagEnabled(false),
  isBigramFlagEnabled(false),
  isTrigramFlagEnabled(false) {
  this->finderInfo = finderInfo;
  this->description = description;
  this->stopwordHelper = description->getCategory()->getStopwordHelper();
  this->ngramRanker = new NGramRanker(stopwordHelper);
  this->ngramdir = Utils::checkTrailingSlash(finderInfo->getFinderTrainingPaths()->getFeatureExtDirPath()) + "N-Grams/";
}

SentenceNGramsFeatureExtractor::~SentenceNGramsFeatureExtractor() {
  delete ngramRanker;

  // Delete the ngram profile if non-empty
  NGramRanker::destroyNGramVecs(mathNGramProfile);
}

void SentenceNGramsFeatureExtractor
::doTrainerInitialization() {
  mathNGramProfile =
      NGramProfileGenerator(finderInfo, ngramRanker, ngramdir)
      .generateMathNGrams();
#ifdef DBG_DISPLAY_NG_PROFILE
  std::cout << "-----------\nDisplaying the n-gram profile\n------------\n";
  for(int i = 0; i < mathNGramProfile.size(); ++i) {
    const int gram = i + 1;
    std::cout << gram << "-grams:\n";
    for(int j = 0; j < mathNGramProfile[i].size(); ++j) {
      NGramFrequency* ngFreq = mathNGramProfile[i][j];
      std::cout << gram << "-gram: " << *(ngFreq->ngram)
          << ", frequency: " << ngFreq->frequency << std::endl;
    }
  }
#endif
}

void SentenceNGramsFeatureExtractor::doFinderInitialization() {
  doTrainerInitialization();
}

void SentenceNGramsFeatureExtractor::doPreprocessing(
    BlobDataGrid* const blobDataGrid) {

  // Determine the N-Gram features for each sentence
  // -- first get the ranked ngram vectors for each sentence
  std::vector<TesseractSentenceData*> page_sentences = blobDataGrid->getAllRecognizedSentences();
  for(int i = 0; i < page_sentences.size(); ++i) {
    TesseractSentenceData* cursentence = page_sentences[i];
    if(cursentence->sentence_txt == NULL) {
      std::cout << "in null sentence at sentence " << i+1 << " of " << page_sentences.size() << std::endl;
    }
    assert(cursentence->sentence_txt != NULL); // shouldn't have been added in the first place if empty
    RankedNGramVecs* sentence_ngrams = new RankedNGramVecs;
    *sentence_ngrams = ngramRanker->generateSentenceNGrams(cursentence, ngramdir);
    cursentence->setNGramCounts(sentence_ngrams); // store the n-grams in the sentence
#ifdef DBG_SHOW_NGRAMS
    std::cout << "Displaying the N-Grams found for the following sentence:\n"
        << cursentence->sentence_txt << std::endl;
    dbgDisplayNGrams(*(cursentence->ngrams));
    M_Utils m;
    m.waitForInput();
#endif
  }

  // -- now use the ranked ngram vectors and compare them to the n-gram profile
  //    to get the actual feature values.
  for(int i = 0; i < page_sentences.size(); ++i) {
    TesseractSentenceData* cursentence = page_sentences[i];
    GenericVector<double> ng_features;
    for(int j = 0; j < 3; ++j)
      ng_features.push_back(getNGFeature(cursentence, j+1));
    cursentence->setNGramFeatures(ng_features);
  }
}

std::vector<DoubleFeature*> SentenceNGramsFeatureExtractor
::extractFeatures(BlobData* const blob) {
  double unigram = (double)0, bigram = (double)0, trigram = (double)0;
  TesseractSentenceData* blob_sentence = getBlobSentence(blob);
  if(blob_sentence != NULL) {
    assert(blob_sentence->getNGramFeatures().size() != 0);
    GenericVector<double> sentence_ngram_features = blob_sentence->getNGramFeatures();
    unigram = sentence_ngram_features[0];
    bigram = sentence_ngram_features[1];
    trigram = sentence_ngram_features[2];
  }

  std::vector<DoubleFeature*> fv;

  if(isUnigramFlagEnabled) {
    fv.push_back(new DoubleFeature(description, unigram, description->getUnigramFlag()));
  }
  if(isBigramFlagEnabled) {
    fv.push_back(new DoubleFeature(description, bigram, description->getBigramFlag()));
  }
  if(isTrigramFlagEnabled) {
    fv.push_back(new DoubleFeature(description, trigram, description->getTrigramFlag()));
  }

  return fv;
}

TesseractSentenceData* SentenceNGramsFeatureExtractor::getBlobSentence(
    BlobData* const blobData) {
  TesseractBlockData* block  = blobData->getParentBlock();
  if(block == NULL) {
    return NULL;
  }
  TesseractWordData* word = blobData->getParentWord();
  if(word == NULL) {
    assert(false); // shouldn't be possible...
    return NULL;
  }
  const int sentenceIndex = word->getSentenceIndex();
  if(sentenceIndex < 0) {
    return NULL;
  }
  if(sentenceIndex >= block->getRecognizedSentences().size()) {
    std::cout << "ERROR: a tesseract word was assigned to a non-existent sentence.\n";
    assert(false);
  }
  return block->getRecognizedSentences()[sentenceIndex];
}

BlobFeatureExtractorDescription* SentenceNGramsFeatureExtractor
::getFeatureExtractorDescription() {
  return description;
}

double SentenceNGramsFeatureExtractor::getNGFeature(
    TesseractSentenceData* const sentence,
    const int gram) {
  assert(gram > 0 && gram < 4); // only uni, bi, and tri-grams supported
  double ng_feat = 0;
  const int gramindex = gram - 1; // zero-based index
  RankedNGramVecs ngramsvec = *sentence->ngrams;
  RankedNGramVec ngrams = ngramsvec[gramindex];
  for(int i = 0; i < ngrams.length(); ++i) {
    ng_feat += findNGProfileMatch(ngrams[i], mathNGramProfile[gramindex], gram);
  }
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  std::cout << "Sentence:\n" << sentence->sentence_txt << std::endl;
  std::cout << "The unscaled " << gram << "-gram feature for the above sentence:\n";
  std::cout << ng_feat << std::endl;
#endif
  ng_feat = scaleNGramFeature(ng_feat, mathNGramProfile[gramindex]);
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  std::cout << "The scaled " << gram << "-gram feature: " << ng_feat << std::endl;
#endif
  ng_feat = M_Utils::expNormalize(ng_feat);
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
  std::cout << "The normalized " << gram << "-gram feature: " << ng_feat << std::endl;
  M_Utils::waitForInput();
#endif
  return ng_feat;
}

double SentenceNGramsFeatureExtractor::findNGProfileMatch(
    NGramFrequency* const ngramfreq,
    RankedNGramVec profile,
    const int gram) {
  NGram* ngram = ngramfreq->ngram;
  double ngram_count = ngramfreq->frequency;
  assert(ngram->length() == gram);
  for(int i = 0; i < profile.length(); ++i) {
    NGram* p_ngram = profile[i]->ngram;
    double p_ngram_count = profile[i]->frequency;
    assert(p_ngram->length() == gram);
    if(*p_ngram == *ngram) {
#ifdef DBG_SHOW_EACH_SENTENCE_NGRAM_FEATURE
      std::cout << "Matching profile ngram: " << *p_ngram << ", occurs " << p_ngram_count
          << " times on profile and " << ngram_count << " times in sentence\n";
#endif
      return ngram_count * p_ngram_count;
    }
  }
  return (double)0;
}

double SentenceNGramsFeatureExtractor::scaleNGramFeature(
    double ng_feat,
    RankedNGramVec profile) {
  double upperbound = (double)5;
  double scaled_feat = ng_feat;
  double top_profile_freq = profile[0]->frequency;
  if(top_profile_freq > upperbound)
    scaled_feat /= (double)10;
  if(scaled_feat > upperbound)
    scaled_feat = upperbound;
  return scaled_feat;
}

void SentenceNGramsFeatureExtractor::dbgDisplayNGrams(const RankedNGramVecs& ngrams) {
  for(int i = 0; i < ngrams.length(); i++) {
    std::cout << i + 1 << "-grams:\n";
    for(int j = 0; j < ngrams[i].length(); j++) {
      std::cout << *(ngrams[i][j]->ngram) << " : "
          << ngrams[i][j]->frequency << std::endl;
    }
  }
}

void SentenceNGramsFeatureExtractor::enableUnigramFlag() {
  enabledFlagDescriptions.push_back(description->getUnigramFlag());
  isUnigramFlagEnabled = true;
}

void SentenceNGramsFeatureExtractor::enableBigramFlag() {
  enabledFlagDescriptions.push_back(description->getBigramFlag());
  isBigramFlagEnabled = true;
}

void SentenceNGramsFeatureExtractor::enableTrigramFlag() {
  enabledFlagDescriptions.push_back(description->getTrigramFlag());
  isTrigramFlagEnabled = true;
}

std::vector<FeatureExtractorFlagDescription*> SentenceNGramsFeatureExtractor
::getEnabledFlagDescriptions() {
  return enabledFlagDescriptions;
}
