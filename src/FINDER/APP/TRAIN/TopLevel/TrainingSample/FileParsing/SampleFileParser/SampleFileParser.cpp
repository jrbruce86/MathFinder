/*
 * SampleFileParser.cpp
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#include <SampleFileParser.h>

#include <Sample.h>
#include <Utils.h>
#include <DoubleFeature.h>
#include <FeatExtFlagDesc.h>

#include <allheaders.h>

#include <vector>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <stddef.h>

void SampleFileParser::writeSamples(const std::string& sample_path,
    const std::vector<std::vector<BLSample*> >& samples) {
  std::cout << "Writing the samples to " << sample_path << std::endl;
  int samplecount = 0;
  std::ofstream s(sample_path.c_str());
  if(!s.is_open()) {
    std::cout << "ERROR: Couldn't open " << sample_path << " for writing\n";
    assert(false);
  }

  for(int i = 0; i < samples.size(); ++i) {
    for(int j = 0; j < samples[i].size(); ++j) {
      writeSample(samples[i][j], s);
      ++samplecount;
    }
  }
  std::cout << "Total of " << samplecount << " samples written.\n";
}

// writes the sample in the following space-delimited format:
// label featurevec imgindex blobbox groundtruth_entry
// label: 0 or 1
// featurevec: comma delimited list of doubles
// imgname: gives the name of the image in which the blob resides (0.png, 1.png, 2.jpg, etc.)
// imgindex: gives the index of the image in which the blob resides (should be an integer matching the name)
// blobbox: comma delimited coordinates as follows x,y,w,h
// groundtruth_entry: info on the matching groundtruth entry if label is 1
//                    otherwise the following comma delimited list is all 0's
//                    type,x,y,w,h <- type is E/D/L (embedded displayed or labeled)
//                     and x,y,w,h is the bounding box of the entry
// Thus the entire space-delimited format is as follows:
// label f1,f2,etc., imgname x1,y1,w1,h1 entrytype,x2,y2,w2,h2
// f is the feature, x1-h1 are the coords for the blob, and x2-h2 are the coords
// for the groundtruth entry if applicable (otherwise all 0's)
void SampleFileParser::writeSample(BLSample* const sample, std::ofstream& fs) {
  if(sample->label)
    fs << 1 << " ";
  else
    fs << 0 << " ";
  int numfeat = (sample->features).size();
  for(int i = 0; i < numfeat; ++i) {
    fs << std::setprecision(20) << sample->features[i]->getFeature();
    fs << (((i + 1) < numfeat) ? "," : " ");
  }
  fs << sample->imageName << " ";
  fs << sample->imageIndex << " ";
  BOX* box = sample->blobbox;
  fs << box->x << "," << box->y << "," << box->w << "," << box->h << " ";
  if(sample->entry) {
    GroundTruthEntry* entry = sample->entry;
    GT_Entry::GTEntryType type = entry->entry;
    if(type == GT_Entry::DISPLAYED)
      fs << "D,";
    else if(type == GT_Entry::EMBEDDED)
      fs << "E,";
    else if(type == GT_Entry::LABEL)
      fs << "L,";
    else {
      std::cout << "ERROR: Unexpected groundtruth entry type\n";
      assert(false);
    }
    BOX* gtbox = entry->rect;
    fs << gtbox->x << "," << gtbox->y << "," << gtbox->w
       << "," << gtbox->h << "\n";
  }
  else
    fs << "0,0,0,0,0\n";
}

std::vector<std::vector<BLSample*> > SampleFileParser::readOldSamples(const std::string& sample_path,
    const std::vector<BlobFeatureExtractor*>& blobFeatureExtractors) {
  std::vector<std::vector<BLSample*> > samples_read = std::vector<std::vector<BLSample*> >();
  std::ifstream s(sample_path.c_str());
  if(!s.is_open()) {
    std::cout << "ERROR: Couldn't open sample file for reading at " << sample_path << std::endl;
    assert(false);
  }
  int nullcount = 0;
  int curimg = -1;
  std::string line;
  while(getline(s, line)) {
    if(line.empty()) {
      if(nullcount++ > 1) {
        std::cout << "ERROR: Invalid sample file at " << sample_path << std::endl;
        assert(false);
      }
      continue;
    }
    BLSample* const sample = readSample((std::string)line, blobFeatureExtractors);
    assert(sample->imageIndex > -1);
    if(sample->imageIndex != curimg) {
      assert(sample->imageIndex == (curimg + 1)); // should start at 0 and go up by 1 for each image (the images should be ordered by name)
      ++curimg;
      samples_read.push_back(std::vector<BLSample*>()); // add a new vector for this image
    }
    samples_read.back().push_back(sample); // add the sample to the list of samples for the current image
  }
  return samples_read;
}

// reads the sample in the following space-delimited format:
// label featurevec imgindex blobbox groundtruth_entry
// label: 0 or 1
// featurevec: comma delimited list of doubles
// imageName: gives the name of the image for this blob
// imgIndex: gives the image number for the blob
// blobbox: comma delimited coordinates as follows x,y,w,h
// groundtruth_entry: info on the matching groundtruth entry if label is 1
//                    otherwise the following comma delimited list is all 0's
//                    type,x,y,w,h <- type is E/D/L (embedded displayed or labeled)
//                     and x,y,w,h is the bounding box of the entry
// Thus the entire space-delimited format is as follows:
// label f1,f2,etc., imgindex x1,y1,w1,h1 entrytype x2,y2,w2,h2
// f is the feature, x1-h1 are the coords for the blob, and x2-h2 are the coords
// for the groundtruth entry if applicable (otherwise all 0's)
BLSample* SampleFileParser::readSample(const std::string& line,
   const std::vector<BlobFeatureExtractor*>& blobFeatureExtractors) {
  std::vector<std::string> spacesplit = Utils::stringSplit(line, ' ');
  // get the label
  int labelint = atoi(spacesplit[0].c_str());
  assert(labelint == 0 || labelint == 1);
  bool label = (labelint == 1) ? true : false;

  // get the feature vec
  std::string fvecstring = spacesplit[1];
  std::vector<std::string> featureStrVec = Utils::stringSplit(fvecstring, ',');
  std::vector<DoubleFeature*> featureVec;
  int index = 0;
  for(int i = 0; i < blobFeatureExtractors.size(); ++i) {
    BlobFeatureExtractor* const featExt = blobFeatureExtractors[i];
    assert(index < featureStrVec.size()); // sanity
    std::vector<FeatureExtractorFlagDescription*> enabledFlags =
        featExt->getEnabledFlagDescriptions();
    // no enabled flags, just read in as a feature
    if(enabledFlags.empty()) {
      featureVec.push_back(
          new DoubleFeature(
              featExt->getFeatureExtractorDescription(),
              atof(featureStrVec[index++].c_str())));
      continue;
    }
    // otherwise read in as flags
    for(int j = 0; j < enabledFlags.size(); ++j) {
      assert(index < featureStrVec.size()); // sanity
      featureVec.push_back(
          new DoubleFeature(
              featExt->getFeatureExtractorDescription(),
              atof(featureStrVec[index++].c_str()),
              enabledFlags[j]));
    }
  }
  assert(featureVec.size() == featureStrVec.size()); // sanity check
  assert(index == featureStrVec.size()); // sanity check

  // get the image name
  std::string imageName = spacesplit[2];

  // get the image index
  const int imageIndex = atoi(spacesplit[3].c_str());

  // get blobbox
  std::string blobboxstr = spacesplit[4];
  std::vector<std::string> blobstrvec = Utils::stringSplit(blobboxstr, ',');
  assert(blobstrvec.size() == 4);
  int x = atoi(blobstrvec[0].c_str());
  int y = atoi(blobstrvec[1].c_str());
  int w = atoi(blobstrvec[2].c_str());
  int h = atoi(blobstrvec[3].c_str());
  BOX* blobbox = boxCreate(x, y, w, h);

  //get the groundtruth type if applicable
  std::string gtentrystr = spacesplit[5];
  GroundTruthEntry* gtentry;
  std::vector<std::string> gtentrystrvec =  Utils::stringSplit(gtentrystr, ',');
  assert(gtentrystrvec.size() == 5);
  const char* gttype = gtentrystrvec[0].c_str();
  assert(strlen(gttype) == 1);
  char typchar = gttype[0];
  assert(typchar == 'D' || typchar == 'E' || typchar == 'L' || typchar == '0');
  if(typchar == '0')
    gtentry = NULL;
  else if(typchar == 'D' || typchar == 'E' || typchar == 'L') {
    gtentry = new GroundTruthEntry;
    if(typchar == 'D')
      gtentry->entry = GT_Entry::DISPLAYED;
    else if(typchar == 'E')
      gtentry->entry = GT_Entry::EMBEDDED;
    else if(typchar == 'L')
      gtentry->entry = GT_Entry::LABEL;
    else
      error();
    // get the groundtruth entry's box
    int gtx = atoi(gtentrystrvec[1].c_str());
    int gty = atoi(gtentrystrvec[2].c_str());
    int gtw = atoi(gtentrystrvec[3].c_str());
    int gth = atoi(gtentrystrvec[4].c_str());
    BOX* gtrect = boxCreate(gtx, gty, gtw, gth);
    gtentry->rect = gtrect;
    gtentry->image_index = imageIndex;
  }
  else
    error();

  // now everything's parsed and loaded, set up the sample
  BLSample* sample = new BLSample;
  sample->label = label;
  sample->features = featureVec;
  sample->imageName = imageName;
  sample->imageIndex = imageIndex;
  sample->blobbox = blobbox;
  sample->entry = gtentry;
  return sample;
}




void SampleFileParser::error() {
  std::cout << "ERROR: Invalid sample file!\n";
  assert(false);
}
