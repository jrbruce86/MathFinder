///////////////////////////////////////////////////////////////////////
// File:        equationdetect.h
// Description: The equation detection class that inherits equationdetectbase.
// Author:      Zongyi (Joe) Liu (joeliu@google.com)
// Created:     Fri Aug 31 11:13:01 PST 2011
//
// (C) Copyright 2011, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCMAIN_EQUATIONDETECT_H__
#define TESSERACT_CCMAIN_EQUATIONDETECT_H__

#include "blobbox.h"
#include "equationdetectbase.h"
#include "genericvector.h"
#include "unichar.h"

// added by jake
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
enum Color {RED, BLUE};
#define fgthresh 100   // this may need to change depending on the image..
typedef l_int32 rgbtype;
#define DATASET_SIZE 15
// end added by jake

class BLOBNBOX;
class BLOB_CHOICE;
class BLOB_CHOICE_LIST;
class TO_BLOCK_LIST;
class TBOX;
class UNICHARSET;

namespace tesseract {

class Tesseract;
class ColPartition;
class ColPartitionGrid;
class ColPartitionSet;

class EquationDetect : public EquationDetectBase {
 public:
  EquationDetect(const char* equ_datapath,
                 const char* equ_language);
  ~EquationDetect();

  enum IndentType {
    NO_INDENT,
    LEFT_INDENT,
    RIGHT_INDENT,
    BOTH_INDENT,
    INDENT_TYPE_COUNT
  };

  // Reset the lang_tesseract_ pointer. This function should be called before we
  // do any detector work.
  void SetLangTesseract(Tesseract* lang_tesseract);

  // Iterate over the blobs inside to_block, and set the blobs that we want to
  // process to BSTT_NONE. (By default, they should be BSTT_SKIP). The function
  // returns 0 upon success.
  int LabelSpecialText(TO_BLOCK* to_block);

  // Find possible equation partitions from part_grid. Should be called
  // after the special_text_type of blobs are set.
  // It returns 0 upon success.
  int FindEquationParts(ColPartitionGrid* part_grid,
                        ColPartitionSet** best_columns);

  // Reset the resolution of the processing image. TEST only function.
  void SetResolution(const int resolution);

  // Debug function: Render a bounding box on pix based on the value of its
  // special_text_type, specifically:
  // BSTT_MATH: red box
  // BSTT_DIGIT: cyan box
  // BSTT_ITALIC: green box
  // BSTT_UNCLEAR: blue box
  // All others: yellow box
  static void RenderSpecialText(Pix* pix, BLOBNBOX* blob);


 protected:
  // Identify the special text type for one blob, and update its field. When
  // height_th is set (> 0), we will label the blob as BSTT_NONE if its height
  // is less than height_th.
  void IdentifySpecialText(BLOBNBOX *blob, const int height_th);

  // Estimate the type for one unichar.
  BlobSpecialTextType EstimateTypeForUnichar(
      const UNICHARSET& unicharset, const UNICHAR_ID id) const;

  // Compute special text type for each blobs in part_grid_.
  void IdentifySpecialText();

  // Identify blobs that we want to skip during special blob type
  // classification.
  void IdentifyBlobsToSkip(ColPartition* part);

  // The ColPartitions in part_grid_ maybe over-segmented, particularly in the
  // block equation regions. So we like to identify these partitions and merge
  // them before we do the searching.
  void MergePartsByLocation();

  // Staring from the seed center, we do radius search. And for partitions that
  // have large overlaps with seed, we remove them from part_grid_ and add into
  // parts_overlap. Note: this function may update the part_grid_, so if the
  // caller is also running ColPartitionGridSearch, use the RepositionIterator
  // to continue.
  void SearchByOverlap(ColPartition* seed,
                       GenericVector<ColPartition*>* parts_overlap);

  // Insert part back into part_grid_, after it absorbs some other parts.
  void InsertPartAfterAbsorb(ColPartition* part);

  // Identify the colparitions in part_grid_, label them as PT_EQUATION, and
  // save them into cp_seeds_.
  void IdentifySeedParts();

  // Check the blobs count for a seed region candidate.
  bool CheckSeedBlobsCount(ColPartition* part);

  // Compute the foreground pixel density for a tbox area.
  float ComputeForegroundDensity(const TBOX& tbox);

  // Check if part from seed2 label: with low math density and left indented. We
  // are using two checks:
  // 1. If its left is aligned with any coordinates in indented_texts_left,
  // which we assume have been sorted.
  // 2. If its foreground density is over foreground_density_th.
  bool CheckForSeed2(
      const GenericVector<int>& indented_texts_left,
      const float foreground_density_th,
      ColPartition* part);

  // Count the number of values in sorted_vec that is close to val, used to
  // check if a partition is aligned with text partitions.
  int CountAlignment(
      const GenericVector<int>& sorted_vec, const int val) const;

  // Check for a seed candidate using the foreground pixel density. And we
  // return true if the density is below a certain threshold, because characters
  // in equation regions usually are apart with more white spaces.
  bool CheckSeedFgDensity(const float density_th, ColPartition* part);

  // A light version of SplitCPHor: instead of really doing the part split, we
  // simply compute the union bounding box of each splitted part.
  void SplitCPHorLite(ColPartition* part, GenericVector<TBOX>* splitted_boxes);

  // Split the part (horizontally), and save the splitted result into
  // parts_splitted. Note that it is caller's responsibility to release the
  // memory owns by parts_splitted. On the other hand, the part is unchanged
  // during this process and still owns the blobs, so do NOT call DeleteBoxes
  // when freeing the colpartitions in parts_splitted.
  void SplitCPHor(ColPartition* part,
                  GenericVector<ColPartition*>* parts_splitted);

  // Check the density for a seed candidate (part) using its math density and
  // italic density, returns true if the check passed.
  bool CheckSeedDensity(const float math_density_high,
                        const float math_density_low,
                        const ColPartition* part) const;

  // Check if part is indented.
  IndentType IsIndented(ColPartition* part);

  // Identify inline partitions from cp_seeds_, and re-label them.
  void IdentifyInlineParts();

  // Comute the super bounding box for all colpartitions inside part_grid_.
  void ComputeCPsSuperBBox();

  // Identify inline partitions from cp_seeds_ using the horizontal search.
  void IdentifyInlinePartsHorizontal();

  // Estimate the line spacing between two text partitions. Returns -1 if not
  // enough data.
  int EstimateTextPartLineSpacing();

  // Identify inline partitions from cp_seeds_ using vertical search.
  void IdentifyInlinePartsVertical(const bool top_to_bottom,
                                   const int textPartsLineSpacing);

  // Check if part is an inline equation zone. This should be called after we
  // identified the seed regions.
  bool IsInline(const bool search_bottom,
                const int textPartsLineSpacing,
                ColPartition* part);

  // For a given seed partition, we search the part_grid_ and see if there is
  // any partition can be merged with it. It returns true if the seed has been
  // expanded.
  bool ExpandSeed(ColPartition* seed);

  // Starting from the seed position, we search the part_grid_
  // horizontally/vertically, find all parititions that can be
  // merged with seed, remove them from part_grid_, and put them  into
  // parts_to_merge.
  void ExpandSeedHorizontal(const bool search_left,
                            ColPartition* seed,
                            GenericVector<ColPartition*>* parts_to_merge);
  void ExpandSeedVertical(const bool search_bottom,
                          ColPartition* seed,
                          GenericVector<ColPartition*>* parts_to_merge);

  // Check if a part_box is the small neighbor of seed_box.
  bool IsNearSmallNeighbor(const TBOX& seed_box,
                           const TBOX& part_box) const;

  // Perform the density check for part, which we assume is nearing a seed
  // partition. It returns true if the check passed.
  bool CheckSeedNeighborDensity(const ColPartition* part) const;

  // After identify the math blocks, we do one more scanning on all text
  // partitions, and check if any of them is the satellite of:
  // math blocks: here a p is the satellite of q if:
  // 1. q is the nearest vertical neighbor of p, and
  // 2. y_gap(p, q) is less than a threshold, and
  // 3. x_overlap(p, q) is over a threshold.
  // Note that p can be the satellites of two blocks: its top neighbor and
  // bottom neighbor.
  void ProcessMathBlockSatelliteParts();

  // Check if part is the satellite of one/two math blocks. If it is, we return
  // true, and save the blocks into math_blocks.
  bool IsMathBlockSatellite(
      ColPartition* part, GenericVector<ColPartition*>* math_blocks);

  // Search the nearest neighbor of part in one vertical direction as defined in
  // search_bottom. It returns the neighbor found that major x overlap with it,
  // or NULL when not found.
  ColPartition* SearchNNVertical(const bool search_bottom,
                                 const ColPartition* part);

  // Check if the neighbor with vertical distance of y_gap is a near and math
  // block partition.
  bool IsNearMathNeighbor(const int y_gap, const ColPartition *neighbor) const;

  // Generate the tiff file name for output/debug file.
  void GetOutputTiffName(const char* name, STRING* image_name) const;

  // Debugger function that renders ColPartitions on the input image, where:
  // parts labeled as PT_EQUATION will be painted in red, PT_INLINE_EQUATION
  // will be painted in green, and other parts will be painted in blue.
  void PaintColParts(const STRING& outfile, bool final_output=false) const;

  // Debugger function that renders the blobs in part_grid_ over the input
  // image.
  void PaintSpecialTexts(const STRING& outfile) const;

  // Debugger function that print the math blobs density values for a
  // ColPartition object.
  void PrintSpecialBlobsDensity(const ColPartition* part) const;

  // The tesseract engine intialized from equation training data.
  Tesseract* equ_tesseract_;

  // The tesseract engine used for OCR. This pointer is passed in by the caller,
  // so do NOT destroy it in this class.
  Tesseract* lang_tesseract_;

  // The ColPartitionGrid that we are processing. This pointer is passed in from
  // the caller, so do NOT destroy it in the class.
  ColPartitionGrid* part_grid_;

  // A simple array of pointers to the best assigned column division at
  // each grid y coordinate. This pointer is passed in from the caller, so do
  // NOT destroy it in the class.
  ColPartitionSet** best_columns_;

  // The super bounding box of all cps in the part_grid_.
  TBOX* cps_super_bbox_;

  // The seed ColPartition for equation region.
  GenericVector<ColPartition*> cp_seeds_;

  // The resolution (dpi) of the processing image.
  int resolution_;

  // The number of pages we have processed.
  int page_count_;


  // added by jake
  // convert integer to string
  inline std::string intToString(int i) const {
    char buf[digit_count(i)];
    sprintf(buf, "%d", i);
    return (std::string) buf;
  }
  // returns the number of digits in a given integer decimal number
  inline int digit_count(int decnum) const {
    int numdigits = 0;
    double ddecnum = (double) decnum;
    while (floor(ddecnum) != 0) {
      ddecnum /= (double) 10;
      numdigits++;
    }
    return numdigits;
  }

  inline void drawHlBoxRegion(BOX* box, Pix* pix, Color color) const {
    fillBoxForeground(pix, box, color);
  }

  inline void fillBoxForeground(Pix* inputimg, BOX* bbox,
       Color color, PIX* imread=0) const {
    l_int32 imwidth = pixGetWidth(inputimg);
    const l_int32 x = bbox->x;
    const l_int32 y = bbox->y;
    const l_int32 w = bbox->w;
    const l_int32 h = bbox->h;
    //set img pointer
    l_uint32* curpixel;
    l_uint32* startpixel;
    if(imread) // if we're writing to a different img than we're reading
               // ..inputimg is always the one we are writing to
      startpixel = pixGetData(imread);
    else
      startpixel = pixGetData(inputimg);
    rgbtype rgb[3];
    // scan from left to right, top to bottom while
    // coloring the foreground
    for(l_uint32 k = y; k < y+h; k++) {
      for(l_uint32 l = x; l < x+w; l++) {
        curpixel = startpixel + k*imwidth + l;
        getPixelRGB(curpixel, rgb);
        if(isDark(rgb)) {
          setPixelRGB(inputimg, curpixel, l, k, color);
        }
      }
    }
  }
  inline void getPixelRGB(l_uint32* pixel, rgbtype* rgb) const {
    extractRGBValues(*pixel, &rgb[0], &rgb[1], &rgb[2]);
  }

  inline bool isDark(const rgbtype* const &rgb) const {
    rgbtype thresh = (rgbtype)fgthresh;
    if(checkRGBLessThan(rgb, thresh, thresh, thresh))
      return true;
    return false;
  }

  inline bool checkRGBLessThan(const rgbtype* const &rgb,
      rgbtype red, rgbtype green, rgbtype blue) const {
    if((*rgb < red) && (*(rgb+1) < green) && (*(rgb+2) < blue))
      return true;
    return false;
  }

  inline void setPixelRGB(Pix* pix,
      l_uint32* pixel, const l_int32& x, const l_int32& y,
      Color color) const {
    l_uint8 red=0, green=0, blue=0;
    switch (color) {
      case RED :
        red = 255; break;
      case BLUE :
        blue = 255; break;
      default : break;
    }
    composeRGBPixel(red, green, blue, pixel);
    pixSetPixel(pix, x, y, *pixel);
  }
  // end added by jake
};

}  // namespace tesseract

#endif  // TESSERACT_CCMAIN_EQUATIONDETECT_H_
