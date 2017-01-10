/*
 * TrainingSampleExtractor.h
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#ifndef TRAININGSAMPLEEXTRACTOR_H_
#define TRAININGSAMPLEEXTRACTOR_H_

#include <FinderInfo.h>
#include <Sample.h>
#include <BlobDataGrid.h>
#include <BlobData.h>
#include <FeatExt.h>

#include <vector>

class TrainingSampleExtractor {

 public:

  TrainingSampleExtractor(FinderInfo* const finderInfo,
      MathExpressionFeatureExtractor* const featureExtractor);

  std::vector<std::vector<BLSample*> > getSamples();



  static void destroySamples(std::vector<std::vector<BLSample*> >& samples);

 private:

  void getNewSamples(bool writeToFile=true);

  std::vector<BLSample*> getGridSamples(BlobDataGrid* const grid, int image_index);

  GroundTruthEntry* getBlobGTEntry(BlobData* const blob, const int image_index, Pix* const img);

  void sampleReadVerify();

  // only one of the following is used unless sampleReadVerify is being used to test
   // that the samples extracted are the same as the ones read from the file. if training
   // is carried out, however, only one is used and whichever one is used will be owned
   // by this class.
   std::vector<std::vector<BLSample*> > samples_extracted;
   std::vector<std::vector<BLSample*> > samples_read; // should be the same as samples_extracted
                                                       // can be verified by sampleReadVerify
   FinderInfo* finderInfo;
   MathExpressionFeatureExtractor* featureExtractor;
};


#endif /* TRAININGSAMPLEEXTRACTOR_H_ */
