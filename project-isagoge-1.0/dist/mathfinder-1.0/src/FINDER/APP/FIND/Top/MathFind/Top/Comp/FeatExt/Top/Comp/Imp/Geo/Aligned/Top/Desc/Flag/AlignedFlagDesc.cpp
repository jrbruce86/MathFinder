/*
 * NumAlignedBlobsFeatureExtractorFlagDescriptions.cpp
 *
 *  Created on: Dec 11, 2016
 *      Author: jake
 */

#include <AlignedFlagDesc.h>

/**
 * Rightward flag
 */
std::string NumAlignedBlobsRightwardFeatureFlagDescription::getName() {
  return "NumAlignedRightwardEnabled";
}
std::string NumAlignedBlobsRightwardFeatureFlagDescription::getDescriptionText() {
  return "If enabled, will count the number of blobs that are aligned in the righward direction.";
}

/**
 * Downward flag
 */
std::string NumAlignedBlobsDownwardFeatureFlagDescription::getName() {
  return "NumAlignedDownwardEnabled";
}
std::string NumAlignedBlobsDownwardFeatureFlagDescription::getDescriptionText() {
  return "If enabled, will count the number of blobs that are aligned in the downward direction.";
}

/**
 * Upward flag
 */
std::string NumAlignedBlobsUpwardFeatureFlagDescription::getName() {
  return "NumAlignedUpwardEnabled";
}
std::string NumAlignedBlobsUpwardFeatureFlagDescription::getDescriptionText() {
  return "If enabled, will count the number of blobs that are aligned in the upward direction.";
}
