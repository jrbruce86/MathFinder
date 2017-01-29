/*
 * MathExpressionSegmentor.h
 *
 *  Created on: Oct 30, 2016
 *      Author: jake
 */

#ifndef MATHEXPRESSIONSEGMENTOR_H_
#define MATHEXPRESSIONSEGMENTOR_H_

#include <MFinderResults.h>
#include <BlobDataGrid.h>

class MathExpressionSegmentor {
 public:
  MathExpressionSegmentor(){};

  /**
   * Runs the segmentation algorithm. The results should get stored on
   * the grid.
   */
  virtual void runSegmentation(BlobDataGrid* const grid)=0;

  virtual ~MathExpressionSegmentor(){};
};


#endif /* MATHEXPRESSIONSEGMENTOR_H_ */
