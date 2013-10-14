/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		MEDS.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:11:16 PM
 * Description: TODO
 * 
 * This file is part of Project Isagoge.
 * 
 * Project Isagoge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Project Isagoge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Project Isagoge.  If not, see <http://www.gnu.org/licenses/>.  
 ***************************************************************************/

#include "MEDS.h"
#include "tesseractclass.h"
#include "bbgrid.h"

namespace tesseract {

MEDS::MEDS() : tess(NULL), blobinfogrid(NULL), img(NULL) {}

int MEDS::FindEquationParts(ColPartitionGrid* part_grid,
    ColPartitionSet** best_columns) {

  // I'll extract features from my own custom grid which holds both
  // information that can be gleaned from language recognition as
  // well as everything which couldn't (will hold all of the blobs
  // and if they were recognized then holds the word and sentence
  // it belongs to as well)
  blobinfogrid = new BlobInfoGrid(part_grid->gridsize(), part_grid->bleft(),
      part_grid->tright());
  blobinfogrid->setColPart(part_grid, best_columns);
  blobinfogrid->setTessAndPix(tess);

  // First I'll move (shallow copy) all the blobs in the partition grid
  // over to a grid where each element is just the blob rather than
  // the partition. Once I have the recognition results for each
  // partition I'll be carrying out analysis on the individual blobs
  // throughout the entire image independently of how the partitions
  // have been segmented by Tesseract's layout analysis framework in
  // previous steps.
  blobinfogrid->partGridToBBGrid();


  // I need more information than could be gleaned from just looking at
  // the individual blobs alone and their recognition results. Using
  // Tesseract's API it is possible to achieve high accuracy on normal
  // text regions. I can use sentences extracted from Tesseract's output
  // in order to assign each sentence an N-Gram feature. Thus each blob
  // which is considered part of such a sentence will be assigned a
  // probability of being part of a sentence which contains embedded
  // mathematical expressions.. Further, by simply running the OCR engine
  // on each individual blob various characters are missed altogether.
  // For instance the "=" sign is seen as two horizontal bars, the
  // horizontal bars are often misrecognized as "j". Likewise periods
  // and commas are often misrecognized. Characters like "l", "I", and "i"
  // are mistaken for "1"'s. The dot on top of the i is not, at this stage,
  // known to be part of the "i". By taking advantage of information gleaned
  // from Tesseract's word recognition these issues can be avoided
  // altogether for regions which have relatively normal layout structure.

  // Now I use a Tesseract API assigned to the language being used in
  // order to recognize all the text in each ColPartition (colpartitions
  // gives the page's current segmentatmion from all processing carried out
  // by Tesseract's layout analysis framework up to this point). As I
  // recognize whatever is in these partitions I move all the information
  // that can be gleaned from the recognition into my grid so features may
  // be extracted from these results later.
  blobinfogrid->recognizeColParts();

  // Next step is to iterate through the BlobGrid, inserting all the BLOBNBOXEs
  // into their appropriate BLOBINFO object and/or creating a new BLOBINFO object
  // for blobs which may not have been recognized at all.
  blobinfogrid->insertRemainingBlobs();




  exit(EXIT_FAILURE);

  // Once the blobinfo grid has been established, it becomes possible to then run
  // each individual blobinfo element through feature detection and classification.
  // After the feature detection/classification step, merging will be carried out
  // in order to ensure proper segmentation.

  // ^^^ For creating the math segmentations I will be using my own custom data structure
  //     called the MathSegment, so I'll have a list of these, then I'll convert those into
  //     ColPartitions later. The MathSegment structure is designed to facilitate merging of
  //     detected mathematical blobs into proper segments for subsequent mathematical
  //     recognition. Each MathSegment initially consists of one BLOBINFO object, the
  //     segments are then iteratively merged with their nearest neighbors on the blobinfogrid
  //     based on some heuristics. This procedure is carried out for each MathSegment.








  // Once the final results have been obtained, the issue becomes that of converting
  // all of my custom data structures back into structures that will be useful for
  // Tesseract. This will involve simply taking the initial ColPartsGrid and x

  // going to want to call setowner on the blobnbox to change the owner to the new
   //  colpartition... set_owner.... the inline expressions need to be placed in their
   // own separate ColPartition which will have the polyblocktype of INLINE_EXPRESSION
   // and removed from the ColPartition to which it was previously assumed to belong.

  // The InsertPartAfterAbsorb() method from Joe Liu's work is what I
  // will use as a starting point for this purpose.. The first step will be to convert
  // the MathSegmentation list entries into new ColPartitions. This will involve
  // moving all of the BLOBNBOXES inside the mathsegment to the associated ColPartition
  // the ClaimBoxes() method should be useful here, you need to remember to make the
  // original partition disown the boxes first by using DisownBoxes() otherwise an
  // exception will be raised. There may also be a lot of settings for the
  // ColPartition to which the box originally belonged that should be kept the same
  // in the new ColPartition.. This I'll play by ear
  // then I should be able to insert these
  // partitions back into the grid using Joe Liu's technique or something similar.





/*
  BLOBNBOX* bb = bbgridsearch.NextFullSearch();
  BOX* bbbox = getBlobBoxImCoords(bb);
  cout << "original blob coords in image space:\n";
  dispBoxCoords(bbbox);
  dispRegion(bbbox);
  TBOX cbox = bb->cblob()->bounding_box();
  cout << "original blob coords in cblob grid space:\n";
  cout << "(l,t,r,b): (" << cbox.left() << ", " << cbox.top() \
       << ", " << cbox.right() << ", " << cbox.bottom() << ")\n";
*/


  //waitForInput();

  // need to come up with better ideas. look at text. study it for patterns. what are recognizable features??

   //maybe a good idea to start from feature recognition then come back to precomputing if necessary or it seems like it will improve results. also review term paper and gerrain's work.



  //////1. how many horizontally adjacent characters are also within some vertical threshold -
  //////////////good for detecting





    // 1. average vertical/horizontal distance measure:
    //iterate through the bloblist
      // calculate the distance to the vertically closest blob below within some horizontal
         // threshold with regard to the sizes of the two blobs (disgard if can't be found
             // or not within threshold)
      // for each blob not disregarded increment a count
    // calculate the vertical blob distance average from the sum and count

    ////////average horizontal distance measure:
    //iterate through the bloblist
      // calculate the distance to the horizontally closest blob within some vertical threshold
        // with regard to the sizes of the two blobs (disregard if can't be found or not within threshold)
      // for each blob not disregarded increment a count


    // 2

  return 0;
}

// I'll keep this as the default implementation provided with Tesseract (for now)
int MEDS::LabelSpecialText(TO_BLOCK* to_block) {
  if (to_block == NULL) {
    tprintf("Warning: input to_block is NULL!\n");
    return -1;
  }

  GenericVector<BLOBNBOX_LIST*> blob_lists;
  blob_lists.push_back(&(to_block->blobs));
  blob_lists.push_back(&(to_block->large_blobs));
  for (int i = 0; i < blob_lists.size(); ++i) {
    BLOBNBOX_IT bbox_it(blob_lists[i]);
    for (bbox_it.mark_cycle_pt (); !bbox_it.cycled_list();
         bbox_it.forward()) {
      bbox_it.data()->set_special_text_type(BSTT_NONE);
    }
  }
  return 0;
}

// this gets called when the equation detector is being set
void MEDS::SetLangTesseract(Tesseract* lang_tesseract) {
  tess = lang_tesseract;
}



} // end namespace tesseract




/* :::Jake EARLIER FEATURE REC WORK:: MERGE AS MUCH AS POSSIBLE OF THE FOLLOWING:::
  Grab the features for all the data entries in the Labeled Results
  file and append it to the entries in a new file, FinalGroundTruth.dat.
  The features are as explained in the project proposal.
  The output format for FullGroundTruth.dat will be:

  [same data from LabeledResults]:<rowdistabove,rowdistbelow,rowoverlap,rowloc,rowstdv,rowh,wordconf,wordstdv,wordgap,wordblobnum>


  The 10 fields can be organized into two main categories: row-level and word-level features

  * Row Level Features *

  Rowdistabove: The distance above a row (if on top this is set to zero)
  Rowdistabove = 1 - e^(-r_above/average_dist_above)
  - where r_above is the average white space and average_dist_above is the precomputed average (from precompute.cpp)
  Rowdistbelow: The distance below a row (if on bottom this is set to zero)
  Rowdistbelow = 1 - e^(-r_below/average_dist_below)
  - where r_below is the average white space and average_dist_below is the precomputed average (from precompute.cpp)
  Rowoverlap: 0 if not overlapping, 1 otherwise. Overlap detected when either rowdistabove or rowdistbelow is less than zero
  RowLoc: 0 if bottom, 1 if top, .5 if anywhere else
  RowStDev: Standard deviation of the bottom y coords of words w/in a row
  RowStDev = 1 - e^-stdev_y
  - where stdev_y is the standard deviation calculated
  RowHeight: Pixel height of a row
  RowHeight = 1 - e^(-h/hu)
  - where h is the row height in pixels and hu is the precomputed average height
  of normal text (precompute.cpp)
  (TODO)
  RowOperators: Count of the instances of =, +, -, /, (, ), [, ], {, }, <, or > in a row
  RowOperators = 1 - e^-count
  - where sum is the number of operators found in a row
  (END TODO)

  * Word Level Features *

  WordConf: Tesseract's OCR confidence for each word
  WordConf = 1 - e^(-c/thresh)
  - where c is the confidence rating from tesseract and thresh is
  the precomputed average confidence of normal text (precompute.cpp).
  WordStDev: Standard deviation of bottom coordinates of bounding boxes within a word
  WordStDev = 1 - e^-stdev_y
  - where stdev_y is the standard deviation calculated
  WordGaps: Average inter-character gaps between bounding boxes within a word
  WordGaps = 1 - e^(-g/gu)
  - where g is the average calculated and gu is the precomputed average
  for normal text words (precompute.cpp) (is zero if no gaps or only one blob)

/* Common Variables
double ru, hu, thresh, gu; // precomputed averages extracted from a file
QString curData;
// an entry in the results file is organized as follows:
// filename:label:wordboundingbox:recognitionresult:confidence:rowboundingbox
// the following variables will give the indexes just after each colon of an entry:
int colon1, colon2, colon3, colon4, colon5;
int currowbottomy, currowtopy; // top and bottom y coordinates for a row
int from, to; // scratch
int curfilenum; // the filenumber updated for each word
int filenum = -1; // the file number updated only for each new file (used to check for new file)
double buff;
QStringList outputList; // the updated entries that will be written to the output file
void getIndexes();
void getRowCoords();
bool init = true; // just so we know when we are on the first file

/* Variables for distance btwn rows as well as other row-related features
int prevbottom = -1, prevtop = -1; // the bottom and top of a previous row
typedef enum {TOP, BOTTOM, MIDDLE} loc; // describes whether row is on bottom, top or somewhere between
typedef struct {
  double dist_above;
  loc loc_;
  double dist_below;
  bool vert_overlap;
} RowDistFeatures;
QList<RowDistFeatures> wsList; // the list of white space measurements in the order as given by LabeledResults
// the whitespace above the previous entry, below the previous entry, and average of the two
double whitespaceabove = -1, whitespacebelow, whitespace;
bool prevfirstrow = false; // so we know when we are on the second row
double dist_above, dist_below;

/* Variables for RowStDev Calculation
// the list of bottom Y coordinates for words in a row
QList<int> wordBottomYList;
QList<double> RowStDevList;
int curwordbottomy;
int count_;
double mean = 0; // mean
double stdev = 0; // standard deviation
void computeRowStDev();

/* Variables for computing WordStDev and WordGaps
int left_, right_, top_, bottom_;
void getWordBox();
void openNewImage();
IplImage *img = 0; // the image file
IplImage *rgbIn; // an rgb version (required to use cvblob)
IplImage *labelim; // the labeled image (required by cvblob)
CvBlobs blobs; // holds the blobs within a word
uchar* data; // representation of the image used for inverting it's colors
QString filename;
bool cmpX(const pair<CvLabel, CvBlob*> &p1, const pair<CvLabel, CvBlob*> &p2) {
  return p1.second->centroid.x < p2.second->centroid.x;
}
vector< pair <CvLabel, CvBlob*> > blobList;
// for calculating inter-character gaps
int prevblobright = -1;
double gap;
int gapcount = 0;

int main() {
  QFile labelFile("LabeledResults");
  if(!labelFile.open(QIODevice::ReadOnly)) {
    qDebug() << "ERROR: Could not open LabeledResults";
    return -1;
  }
  QFile averages("Precomputed_Averages");
  if(!averages.open(QIODevice::ReadOnly)) {
    qDebug() << "ERROR: Could not open Precomputed_Averages";
    return -1;
  }
  QFile outFile("FullGroundTruth.dat");
  if(!outFile.open(QIODevice::WriteOnly)) {
    qDebug() << "ERROR: Could not open FullGroundTruth.dat";
    return -1;
  }

  QTextStream labelStream(&labelFile);
  QTextStream aveStream(&averages);
  QTextStream outStream(&outFile);

  // get the precomputed averages from the Precomputed_Averages file
  curData = aveStream.readLine();
  ru = (curData.right(curData.size() - curData.indexOf(":") - 1)).toDouble();
  curData = aveStream.readLine();
  hu = (curData.right(curData.size() - curData.indexOf(":") - 1)).toDouble();
  curData = aveStream.readLine();
  thresh = (curData.right(curData.size() - curData.indexOf(":") - 1)).toDouble();
  curData = aveStream.readLine();
  gu = (curData.right(curData.size() - curData.indexOf(":") - 1)).toDouble();

  /* Distance between rows
  RowDistFeatures features; // the distance above followed by the distance below (above, below)
  features.vert_overlap = false;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();

    getIndexes();
    getRowCoords(); // updates currowbottomy and currowtopy
    // if its a new file then just need the white space below and
    // add existing whitespaceabove to list for end of previous file
    // (if not on first)
    if(filenum != curfilenum) {
      if(!init) { // record info for the last row of a file (the bottom of the prev file)
        features.loc_ = BOTTOM;
        features.dist_above = whitespaceabove;
  features.dist_below = 0;
        wsList.append(features);
      }
      init = false;
      filenum = curfilenum;
      prevbottom = currowbottomy;
      prevtop = currowtopy;
      prevfirstrow = true;
    }
    // if on a new row and its the second row (this is actually writing the feature for the first row
    // since we need the distance above and below each row, when we process a row we are recording data
    // for the one preceeding the one we're on
    else if(((prevbottom != currowbottomy) || (prevtop != currowtopy)) && prevfirstrow) {
      prevfirstrow = false;
      whitespaceabove = (double)(currowtopy - prevbottom);
      if(whitespaceabove < 0) {
        whitespaceabove = 0;
        features.vert_overlap = true;
      }
      else
        features.vert_overlap = false;
      features.dist_above = 0;
      features.dist_below = whitespaceabove;
      features.loc_ = TOP; // seperate feature to indicate we're on the first row
      wsList.append(features); // add distance to list, this is for the distance below the 1st row
      prevbottom = currowbottomy;
      prevtop = currowtopy;
    }
    // if on new row and its not the first or second one
    else if(((prevbottom != currowbottomy) || (prevtop != currowtopy)) && !prevfirstrow) {
      whitespacebelow = (double)(currowtopy - prevbottom);
      if(whitespacebelow < 0)
        whitespacebelow = 0;
      if(whitespaceabove == 0 || whitespacebelow == 0)
        features.vert_overlap = true;
      else
        features.vert_overlap = false;
      features.loc_ = MIDDLE;
      features.dist_above = whitespaceabove;
      features.dist_below = whitespacebelow;
      wsList.append(features);
      prevbottom = currowbottomy;
      prevtop = currowtopy;
      whitespaceabove = whitespacebelow;
    }
    //qDebug() << features.dist_above << features.dist_below;
  }
  features.loc_ = BOTTOM;
  features.dist_above = whitespaceabove;
  features.dist_below = 0;
  wsList.append(features); // for the distance above the last row of the last file;

  // append each data with its RowWhiteSpace measurement then
  // append the result to a QStringList
  RowDistFeatures buff_;
  labelStream.seek(0);
  int rownum = -1;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();
    getIndexes();
    getRowCoords();
    if((curfilenum != filenum) || (currowtopy != prevtop) || (currowbottomy != prevbottom)) {
      filenum = curfilenum;
      prevbottom = currowbottomy;
      prevtop = currowtopy;
      rownum++;
    }
    buff_ = wsList[rownum];
    //  qDebug() << buff_.dist_above << buff_.dist_below;
    dist_above = 1 - exp(-buff_.dist_above/(ru/(double)2));
    dist_below = 1 - exp(-buff_.dist_below/(ru/(double)2));
    //    buff = 1 - exp(-buff/ru);
    curData.append(":");
    curData.append(QString::number(dist_above));
    curData.append(":");
    curData.append(QString::number(dist_below));
    curData.append(":");
    if(buff_.vert_overlap)
      curData.append(QString::number(1));
    else
      curData.append(QString::number(0));
    curData.append(":");
    if (buff_.loc_ == TOP)
      curData.append(QString::number(1));
    else if (buff_.loc_ == MIDDLE)
      curData.append(QString::number(.5));
    else if (buff_.loc_ == BOTTOM)
      curData.append(QString::number(0));
    else {
      qDebug() << "ERROR: Only TOP, BOTTOM, and MIDDLE are used for the location feature\n";
      return -1;
    }
    outputList.append(curData);
  }

  /*RowStDev
  init = true;
  labelStream.seek(0);
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();
    getIndexes();
    getRowCoords();

    // get the bottom y coordinate for the current word
    from = curData.indexOf(",", curData.indexOf(",", colon2) + 1) + 1;
    to = curData.indexOf(")", from);
    curwordbottomy = (curData.mid(from, to - from)).toInt();

    if((curfilenum != filenum) || (currowtopy != prevtop) ||
       (currowbottomy != prevbottom)) {
      filenum = curfilenum;
      prevbottom = currowbottomy;
      prevtop = currowtopy;
      if(!init)
        computeRowStDev();
      init = false;
    }
    wordBottomYList.append(curwordbottomy);
  }
  computeRowStDev();
  // append the information to the list which will go to the file
  rownum = -1;
  int index = 0;
  labelStream.seek(0);
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();
    getIndexes();
    getRowCoords();
    if((curfilenum != filenum) || (currowtopy != prevtop) || (currowbottomy != prevbottom)) {
      filenum = curfilenum;
      prevbottom = currowbottomy;
      prevtop = currowtopy;
      rownum++;
    }
    buff = RowStDevList[rownum];
    buff = 1 - exp(-(buff/(double)2));
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));
    index++;
  }

  /* Row Height
  labelStream.seek(0);
  index = 0;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    getIndexes();
    getRowCoords();
    buff = (double)(currowbottomy - currowtopy);
    buff = 1 - exp(-buff/hu);
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));
    index++;
  }

  /* Word Conf
  labelStream.seek(0);
  index = 0;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    getIndexes();
    buff = (curData.mid(colon4, colon5 - colon4 - 1)).toDouble();
    buff = 1 - exp(-buff/thresh);
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));
    index++;
  }

  /* Standard deviation within a word
  labelStream.seek(0);
  filenum = -1;
  init = true;
  index = 0;
  double meanarea = 0;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();
    getIndexes();
    getWordBox();
    if(filenum != curfilenum)  // need to open new image
      openNewImage();

    // make sure the bounding box isn't flawed (sometimes tesseract throws out junk)
    CvRect rect = cvRect(left_, top_, right_ - left_, bottom_ - top_);
    if(!(rect.width >= 0 && rect.height >= 0 && rect.x < img->width
         && rect.y < img->height && rect.x + rect.width >= (int)(rect.width > 0)
         && rect.y + rect.height >= (int)(rect.height > 0)))
      continue;

    // set the region of interest to the word's bounding box
    cvSetImageROI(img, cvRect(left_, top_, right_ - left_, bottom_ - top_));

    labelim = cvCreateImage(cvGetSize(img), IPL_DEPTH_LABEL, 1);
    cvZero(labelim);

    // get the blobs
    cvLabel(img, labelim, blobs);

    // sort the blobs from left to right
    blobList.erase(blobList.begin(), blobList.end());
    copy(blobs.begin(), blobs.end(), back_inserter(blobList));
    sort(blobList.begin(), blobList.end(), cmpX);

    // get rid of blobs that are too tiny (ie the dot on an i)
    count_ = 0;
    meanarea = 0;
    for(int i = 0; i < (int)blobList.size(); i++) {
      meanarea += (double)blobList[i].second->area;
      count_++;
    }
    meanarea = meanarea / (double)count_;
    for(int i = 0; i < (int)blobList.size(); i++) { // get rid of blobs < 1/4 the mean area
      if((double)blobList[i].second->area < (meanarea / (double)4)) {
        /* if((curfilenum == 10) && (left_ == 363) && (top_ == 698)
           && (right_ == 605) && (bottom_ == 799)) {
           // for debugging, output all the blobs that are being erased since
           // their area is too small. hard code numbers to the region of interest
           qDebug() << "erasing a blob! blob's bottom right is at ("
           << blobList[i].second->maxx + left_ << "," << blobList[i].second->maxy + top_ << ")"
           << "its area is " << blobList[i].second->area << " and mean area is " << meanarea;
           }
        blobList.erase(blobList.begin() + i);
        i--;
      }
    }

    // compute the mean
    count_ = 0;
    for(int i = 0; i < (int)blobList.size(); i++) {
      /* if((curfilenum == 10) && (left_ == 363) && (top_ == 698)
         && (right_ == 605) && (bottom_ == 799)) {
         // for debugging, show the blobs and their areas after
         // getting rid of ones whose area is too small (make sure the area is high enough)
         qDebug() << "blob" << i << ":"
         << blobList[i].second->maxx + left_ << "," << blobList[i].second->maxy + top_ << ")"
         << " its area is " << blobList[i].second->area;
         }
      mean += (double)blobList[i].second->maxy;
      count_++;
    }
    mean = mean / (double)count_;
    // compute the standard deviation
    for(int i = 0; i < (int)blobList.size(); i++) {
      stdev += pow((double)blobList[i].second->maxy - mean, 2);
    }
    if(count_ < 1)
      count_ = 1;
    stdev = sqrt(stdev / (double)count_);
    buff = 1 - exp(-stdev/5);
    // if(blobList.size() == 1)
    // buff = .5;
    /* if((curfilenum == 10) && (left_ == 363) && (top_ == 698)
       && (right_ == 605) && (bottom_ == 799)) {
       // debugging to show the results for the hard-coded region of interest
       qDebug() << "standard deviation computed as " << stdev << " and mean is "
       << mean << ", n = " << count_ << ". final output is " << buff;
       }
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));
    index++;
    mean = stdev = 0;
    /*
    // debug code for displaying blobs at hard-coded region of interest
    if((curfilenum == 10) && (left_ == 363) && (top_ == 698)
    && (right_ == 605) && (bottom_ == 799)) {
    IplImage *imgOut = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3); cvZero(imgOut);
    cvRenderBlobs(labelim, blobs, rgbIn, imgOut);
    cvShowImage("test", imgOut);
    cvWaitKey(0);
    cvDestroyWindow("test");
    }
  }

  /* Inter-character gaps calculation
  labelStream.seek(0);
  filenum = -1;
  init = true;
  index = 0;
  while(!labelStream.atEnd()) {
    curData = labelStream.readLine();
    curfilenum = (curData.left(curData.indexOf("."))).toInt();
    getIndexes();
    getWordBox();
    if(filenum != curfilenum) // need to open new image
      openNewImage();

    // set the region of interest to the word's bounding box
    cvSetImageROI(img, cvRect(left_, top_, right_ - left_, bottom_ - top_));

    labelim = cvCreateImage(cvGetSize(img), IPL_DEPTH_LABEL, 1);
    cvZero(labelim);

    // get the blobs
    cvLabel(img, labelim, blobs);

    // sort the blobs from left to right
    blobList.erase(blobList.begin(), blobList.end());
    copy(blobs.begin(), blobs.end(), back_inserter(blobList));
    sort(blobList.begin(), blobList.end(), cmpX);

    // find the inter-character gaps
    for(unsigned int i = 0; i < blobList.size(); i++) {
      if(prevblobright != -1) {
        if(prevblobright < (int)blobList[i].second->minx) {
          gap += ((int)blobList[i].second->minx - prevblobright);
             /* if((curfilenum == 10) && (left_ == 809) && (top_ == 714)
             && (right_ == 1088) && (bottom_ == 795)) {
             // debug what the gaps are for hard-coded region of interest
             qDebug() << "gap: " << ((int)blobList[i].second->minx - prevblobright)
             << ", from " << prevblobright + left_ << " to " << blobList[i].second->minx + left_;
             }
          gapcount++;
        }
      }
      prevblobright = blobList[i].second->maxx;
    }
    prevblobright = -1;
    if(gapcount > 0)
      gap = gap / (double)gapcount;
    else
      gap = 0;

    buff = 1 - exp(-gap/gu); // feature showing the average gap between words
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));

    //qDebug() << blobList.size();
    buff = 1 - exp(-(double)blobList.size()); // add a feature showing the number of blobs within the word
    outputList[index].append(":");
    outputList[index].append(QString::number(buff));

    index++;
    gap = 0;
    gapcount = 0;
    /*
    // debug code for displaying blobs at hard-coded region of interest
    if((curfilenum == 10) && (left_ == 809) && (top_ == 714)
    && (right_ == 1088) && (bottom_ == 795)) {
    IplImage *imgOut = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3); cvZero(imgOut);
    cvRenderBlobs(labelim, blobs, rgbIn, imgOut);
    cvShowImage("test", imgOut);
    cvWaitKey(0);
    cvDestroyWindow("test");
    }

  }
  /* // debug code for displaying blobs
     IplImage *imgOut = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3); cvZero(imgOut);
     cvRenderBlobs(labelim, blobs, rgbIn, imgOut);
     cvShowImage("test", imgOut);
     cvWaitKey(0);
     cvDestroyWindow("test");


  for(int i = 0; i < outputList.size(); i++)
    outStream << outputList[i] << "\n";

  return 0;
}

void getIndexes() {
  // get the important indexes
  colon1 = curData.indexOf(":") + 1;
  colon2 = curData.indexOf(":", colon1) + 1;
  colon3 = curData.indexOf(":", colon2) + 1;
  colon4 = curData.indexOf(":-", colon3) + 1;
  colon5 = curData.indexOf(":", colon4) + 1;
}

void getRowCoords() {
  // get the top and bottom y coordinates for the current row
  from = curData.indexOf(",", colon5) + 1;
  to = curData.indexOf(")", colon5);
  currowtopy = (curData.mid(from, to - from)).toInt();
  from = curData.indexOf(",", to) + 1;
  to = curData.indexOf(")", from);
  currowbottomy = (curData.mid(from, to - from)).toInt();
}

void computeRowStDev() {
  // compute the mean
  count_ = 0;
  for(int i = 0; i < wordBottomYList.size(); i++) {
    mean += (double)wordBottomYList[i];
    count_++;
  }
  mean = mean / (double)count_;
  // compute the standard deviation
  for(int i = 0; i < wordBottomYList.size(); i++)
    stdev += pow((double)wordBottomYList[i] - mean, 2);
  stdev = sqrt(stdev / (double)count_);
  RowStDevList.append(stdev);
  while(!wordBottomYList.isEmpty()) // start a new list for new row/file
    wordBottomYList.removeFirst();
  mean = stdev = 0;
}

void getWordBox() {
  to = curData.indexOf(",", colon2);
  left_ = (curData.mid(colon2 + 1, to - colon2 - 1)).toInt();
  from = to;
  to = curData.indexOf(")", from);
  top_ = (curData.mid(from + 1, to - from - 1)).toInt();
  from = to + 6;
  to = curData.indexOf(",", from);
  right_ = (curData.mid(from, to - from)).toInt();
  from = to;
  to = curData.indexOf(")", from);
  bottom_ = (curData.mid(from + 1, to - from - 1)).toInt();
}

void openNewImage() {
  if(!init) {
    cvReleaseImage(&img);
    cvReleaseImage(&rgbIn);
    cvReleaseImage(&labelim);
  }
  init = false;
  filenum = curfilenum;
  filename = QString::number(curfilenum);
  filename.append(".png");
  // read in and invert the colors in the image
  img = cvLoadImage(filename.toStdString().c_str(), CV_LOAD_IMAGE_GRAYSCALE);
  data = (uchar *)img->imageData;
  for(int i = 0; i < img->height; i++)
    for(int j = 0;j < img->width; j++)
      for(int k = 0; k < img->nChannels; k++)
        data[(i * img->widthStep) + (j * img->nChannels) + k] =
          255 - data[(i*img->widthStep) + (j*img->nChannels) + k];
  rgbIn = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
  cvCvtColor(img, rgbIn, CV_GRAY2BGR);
}


  */



/*
// Assume all blobs in image are stored in __________
// Compute the following for the entire page:
// 1. Average vertical/horizontal distance in pixels between vertically adjacent nearest neighbor
// 2. Average character height
// 3. Average language confidence level for all characters
void MEDS::PreComputeThresholds() {
      // variables for precomputing average of displayed feature #1 for normal text
      // as described at top of this file
      bool prevNormal = false;
      int filenum = -1;
      int prevrowbottomy = -1;
      int rowbottomy = -1;
      double wstotal = 0;
      int wscount = 0;

      // variables for precomputing average of displayed feature #2 for normal text
      // as described at top of this file
      double totalrowheight = 0;
      int rowheightcount = 0;

      // variables for precomputing average of embedded exp. feature #1 for normal text
      // as described at top of this file
      double conf = -1;
      double totalconfidence = 0;
      int confidencecount = 0;

      // variables for precomputing average of embedded exp. feature #2 for normal text
      // as described at top of this file (word level as opposed to row level)
      int filenum_ = -1;
      QString filename;
      IplImage *img = 0; // the image file
      IplImage *rgbIn; // an rgb version (required to use cvblob)
      IplImage *labelim; // the labeled image (required by cvblob)
      CvBlobs blobs; // holds the blobs within a word
      uchar* data; // representation of the image used for inverting it's colors
      int left, right, top, bottom;
      bool init = true;
      // for calculating inter-character gaps
      int prevblobright = -1;
      int gap;
      double gaptotal = 0;
      int gapcount = 0;

      vector< pair<CvLabel, CvBlob*> > blobList;





        /* Computing the average vertical distance in pixels between
           two rows of normal/embedded text
        if((filenum != curfilenum) || (rowbottomy != currowbottomy)) { // new file or row
          rowbottomy = currowbottomy;
          if(filenum != curfilenum) {
      filenum = curfilenum;
      if(curLabel == "Normal Text" || curLabel == "Embedded Expression") {
        prevNormal = true;
        prevrowbottomy = currowbottomy;
      }
      else
        prevNormal = false;
          }
          else if(curLabel == "Normal Text" || curLabel == "Embedded Expression") {
      if(prevNormal) {
        if(prevrowbottomy == -1) {
          qDebug() << "BUG: a white space distance being computed without a bottom y";
          return -1;
        }
        if((currowtopy - prevrowbottomy) >= 0) {
          wstotal += (currowtopy-prevrowbottomy);
          wscount++;
        }
      }
      prevNormal = true;
      prevrowbottomy = currowbottomy;
          }
          else
      prevNormal = false;

          /* Computing the average height for a row of normal/embedded
       text.
          if(curLabel == "Normal Text" || curLabel == "Embedded Expression") {
      if((currowbottomy-currowtopy) >= 0) {
        if((currowbottomy - currowtopy) <= 100) {
          totalrowheight += (currowbottomy - currowtopy);
          rowheightcount++;
        }
      }
          }
        }

        /* Computing the average confidence level for all normal
           text.
        if(curLabel == "Normal Text") {
          // get the confidence for the current word
          from = colon4;
          to = colon5 - 1;
          conf = (curData.mid(from, to - from)).toDouble();
          if(conf > (double)-100) {
      totalconfidence += conf;
      confidencecount++;
          }
        }

        /* Computing the average inter-character gaps within each
           word of normal text.
        if((filenum_ != curfilenum) || init) {
          if(!init) {
      cvReleaseImage(&img);
      cvReleaseImage(&rgbIn);
      cvReleaseImage(&labelim);
          }
          filenum_ = curfilenum;
          init = false;
          filename = QString::number(curfilenum);
          filename.append(".png");
          // read in and invert the colors in the image
          img = cvLoadImage(filename.toStdString().c_str(), CV_LOAD_IMAGE_GRAYSCALE);
          data = (uchar *)img->imageData;
          for(int i = 0; i < img->height; i++)
      for(int j = 0;j < img->width; j++)
        for(int k = 0; k < img->nChannels; k++)
          data[(i * img->widthStep) + (j * img->nChannels) + k] =
            255 - data[(i*img->widthStep) + (j*img->nChannels) + k];
          rgbIn = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
          cvCvtColor(img, rgbIn, CV_GRAY2BGR);
        }
        if((curLabel == "Normal Text") ){ // && (conf > (double)(-.1))) {
          // grab the left, top, right, and bottom coordinates of the word
          from = colon2 + 1;
          to = curData.indexOf(",", from);
          left = (curData.mid(from, to - from)).toInt();
          from = to + 1;
          to = curData.indexOf(")", from);
          top = (curData.mid(from, to - from)).toInt();
          from = to + 6;
          to = curData.indexOf(",", from);
          right = (curData.mid(from, to - from)).toInt();
          from = to + 1;
          to = curData.indexOf(")", from);
          bottom = (curData.mid(from, to - from)).toInt();

          // make sure word's rectangle isn't junk (sometimes tesseract outputs junk)
          CvRect rect = cvRect(left, top, right - left, bottom - top);
          if(!(rect.width >= 0 && rect.height >= 0 && rect.x < img->width
         && rect.y < img->height && rect.x + rect.width >= (int)(rect.width > 0)
         && rect.y + rect.height >= (int)(rect.height > 0)))
      continue;

          // set the region of interest to the word's bounding box
          cvSetImageROI(img, cvRect(left, top, right - left, bottom - top));

          labelim = cvCreateImage(cvGetSize(img), IPL_DEPTH_LABEL, 1);
          cvZero(labelim);

          // get the blobs
          cvLabel(img, labelim, blobs);

          // sort the blobs from left to right
          blobList.erase(blobList.begin(), blobList.end());
          copy(blobs.begin(), blobs.end(), back_inserter(blobList));
          sort(blobList.begin(), blobList.end(), cmpX);

          // find the inter-character gaps
          for (unsigned int i=0; i<blobList.size(); i++) {
      if(prevblobright != -1) {
        if(prevblobright < (int)blobList[i].second->minx) {
          gap = ((int)blobList[i].second->minx - prevblobright);
          gaptotal += gap;
          gapcount++;
        }
      }
      prevblobright = blobList[i].second->maxx;
          }
          prevblobright = -1;
        }
      }

      double average_ws = wstotal / (double)wscount;
      double average_height = totalrowheight / (double)rowheightcount;
      double average_conf = totalconfidence / (double)confidencecount;
      double average_gap = gaptotal / (double)gapcount;

      qDebug() << "Average white space between normal/embedded text: " << average_ws;
      qDebug() << "Average row height for normal/embedded text: " << average_height;
      qDebug() << "Average normal text confidence rating: " << average_conf;
      qDebug() << "Average normal text inter-character gap: " << average_gap;

      outputStream << "Average vertical pixels between normal/embedded text rows:"
             << QString::number(average_ws) << "\n";

      outputStream << "Average height of normal/embedded text rows:"
             << QString::number(average_height) << "\n";

      outputStream << "Average confidence rating of normal text:"
             << QString::number(average_conf) << "\n";

      outputStream << "Average horizontal inter-character gap for normal text:"
             << QString::number(average_gap) << "\n";

      return 0;
    }





  bool cmpX(const pair<CvLabel, CvBlob*> &p1, const pair<CvLabel, CvBlob*> &p2) {
    return p1.second->centroid.x < p2.second->centroid.x;
  }
}*/


