/*
 * SampleFileParser.h
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#ifndef SAMPLEFILEPARSER_H_
#define SAMPLEFILEPARSER_H_

#include <Sample.h>
#include <BlobFeatExt.h>

#include <string>
#include <vector>
#include <iostream>

namespace SampleFileParser {

// Serialization methods
void writeSamples(const std::string& sample_path,
    const std::vector<std::vector<BLSample*> >& samples);
void writeSample(BLSample* const sample, std::ofstream& fs);

// Deserialization methods
std::vector<std::vector<BLSample*> > readOldSamples(const std::string& sample_path,
  const std::vector<BlobFeatureExtractor*>& blobFeatureExtractors);
BLSample* readSample(const std::string& line,
    const std::vector<BlobFeatureExtractor*>& blobFeatureExtractors);

// Other
void error();
}

#endif /* SAMPLEFILEPARSER_H_ */
