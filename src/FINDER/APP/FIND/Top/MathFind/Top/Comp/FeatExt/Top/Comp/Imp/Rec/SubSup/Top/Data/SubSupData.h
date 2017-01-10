/*
 * SubOrSuperscriptsData.h
 *
 *  Created on: Dec 4, 2016
 *      Author: jake
 */

#ifndef SUBORSUPERSCRIPTSDATA_H_
#define SUBORSUPERSCRIPTSDATA_H_

class SubOrSuperscriptsData : public BlobFeatureExtractionData {

 public:

  bool hasSuperscript;
  bool isSuperscript;
  bool hasSubscript;
  bool isSubscript;
};

#endif /* SUBORSUPERSCRIPTSDATA_H_ */
