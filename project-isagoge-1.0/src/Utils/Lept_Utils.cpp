 /*****************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		Lept_Utils.cpp
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Aug 6, 2013 4:44:12 PM 
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
 ****************************************************************************/

#include "Lept_Utils.h"

#include <iostream>
using namespace std;

// now fill the connected components within each rectangle and return
// the resulting image (this amounts to simply analyzing the image
// and converting each foreground pixel in it to a given color
Pix* Lept_Utils::fillBoxesForeground(Pix* inputimg, BOXA* boxes, \
    LayoutEval::Color color) {
  l_uint32 numboxes = boxaGetCount(boxes);
  for(l_uint32 i = 0; i < numboxes; i++) {
    Box* bbox = boxaGetBox(boxes, i, L_COPY);
    fillBoxForeground(inputimg, bbox, color);
  }
  return inputimg;
}

void Lept_Utils::fillBoxForeground(Pix* inputimg, BOX* bbox, \
    LayoutEval::Color color, PIX* imread) {
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

int Lept_Utils::colorPixCount(PIX* im, LayoutEval::Color color) {
  int count = 0;
  l_uint32* startpix = pixGetData(im);
  l_uint32* curpix = startpix;
  l_int32 imwidth = pixGetWidth(im);
  for(l_uint32 i = 0; i < im->h; i++) {
    for(l_uint32 j = 0; j < im->w; j++) {
      curpix = startpix + (l_int32)((i*imwidth)+j);
      if(getPixelColor(curpix) == color)
        count++;
    }
  }
  return count;
}

int Lept_Utils::countFalsePositives(BOX* box1, vector<BOX*> box2, \
    PIX* pix1, LayoutEval::Color color, PIX* dbg) {
  l_uint32* curpixdbg;
  l_uint32* startpxdbg;
  if(dbg) {
    startpxdbg = pixGetData(dbg);
    curpixdbg = startpxdbg;
  }
  bool insidebox2 = false;
  int color_non_overlap = 0;
  l_int32 imwidth = pixGetWidth(pix1);
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(box1, &x1, &y1, &w1, &h1);

  //set img pointer (it looks at the pix1 image)
  l_uint32* curpixel;
  l_uint32* startpixel = pixGetData(pix1);
  for(l_int32 i = y1; i < y1+h1; i++) {
    for(l_int32 j = x1; j < x1+w1; j++) {
      // if we're inside one of the box2 rectangles
      // then move on
      insidebox2 = false;
      for(vector<Box*>::iterator boxit = box2.begin(); \
         boxit != box2.end(); boxit++) {
        Box* box = *boxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(box, &x2, &y2, &w2, &h2);
        if(j >= x2 && i >= y2 && j < x2+w2 && i < y2+h2) {
          insidebox2 = true;
          break;
        }
      }
      if(insidebox2)
        continue;
      // otherwise, if we are at a foreground pixel then
      // we increment the counter
      curpixel = startpixel + l_uint32((i*imwidth) + j);
      if(getPixelColor(curpixel) == color) {
        // make sure it hasn't been counted already (used dbg image for this)
        // TODO: rename 'dbg' image to something else.. this clearly serves
        //       a higher purpose than just debugging now since it allows me
        //       to avoid double counting pixels
        curpixdbg = startpxdbg + l_uint32((i*imwidth)+j);
        if(getPixelColor(curpixdbg) == LayoutEval::BLUE)
          continue; // already got this one.. don't double count!!
        color_non_overlap++;
        if(dbg) {
          setPixelRGB(dbg, curpixdbg, j, i, LayoutEval::BLUE);
        }
      }
    }
  }
  return color_non_overlap;
}

int Lept_Utils::countTruePositives(BOX* hypbox, vector<BOX*> gtboxes, \
    PIX* gtim, LayoutEval::Color color, PIX* dbg) {
  l_uint32* curpixdbg;
  l_uint32* startpxdbg;
  if(dbg) {
    startpxdbg = pixGetData(dbg);
    curpixdbg = startpxdbg;
  }
  bool insidegtboxes = false;
  int coloroverlap = 0;
  l_int32 imwidth = pixGetWidth(gtim);
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(hypbox, &x1, &y1, &w1, &h1);
  //set img pointer
  l_uint32* curpixel;
  l_uint32* startpixel = pixGetData(gtim);
  for(l_int32 i = y1; i < y1+h1; i++) {
    for(l_int32 j = x1; j < x1+w1; j++) {
      // if we're not inside any of the gtboxes then we
      // continue
      insidegtboxes = false;
      for(vector<Box*>::iterator gtboxit = gtboxes.begin(); \
          gtboxit != gtboxes.end(); gtboxit++) {
        Box* gtbox = *gtboxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(gtbox, &x2, &y2, &w2, &h2);
        if(x2 <= j && y2 <= i && x2+w2 > j && y2+h2 > i) {
          insidegtboxes = true;
          break;
        }
      }
      if(!insidegtboxes)
        continue;
      // otherwise, if we are at a foreground pixel then
      // we increment the counter
      curpixel = startpixel + l_uint32((i*imwidth) + j);
      if(getPixelColor(curpixel) == color) {
        coloroverlap++;
        if(dbg) {
          curpixdbg = startpxdbg + l_uint32((i*imwidth)+j);
          setPixelRGB(dbg, curpixdbg, j, i, LayoutEval::RED);
        }
      }
    }
  }
  return coloroverlap;
}

int Lept_Utils::countFalseNegatives(BOX* gtbox, vector<BOX*> hypboxes, \
     PIX* gtim, LayoutEval::Color color, PIX* dbg) {
  l_uint32* curpixdbg;
  l_uint32* startpxdbg;
  if(dbg) {
    startpxdbg = pixGetData(dbg);
    curpixdbg = startpxdbg;
  }
  bool in_hypbox = false;
  int falseneg = 0;
  l_int32 imwidth = pixGetWidth(gtim);
  l_int32 x1, y1, w1, h1;
  boxGetGeometry(gtbox, &x1, &y1, &w1, &h1);
  l_uint32* startpixel = pixGetData(gtim);
  l_uint32* curpix = startpixel;
  for(l_int32 i = y1; i < (y1+h1); i++) {
    for(l_int32 j = x1; j < (x1+w1); j++) {
      // if the current pixel is inside of any of the groundtruth
      // boxes then we ignore it (continue)
      in_hypbox = false;
      for(vector<BOX*>::iterator hypboxit = hypboxes.begin(); \
        hypboxit != hypboxes.end(); hypboxit++) {
        BOX* hypbox = *hypboxit;
        l_int32 x2, y2, w2, h2;
        boxGetGeometry(hypbox, &x2, &y2, &w2, &h2);
        if(j >= x2 && i >= y2 && j < (x2+w2) && i < (y2+h2)) {
          in_hypbox = true;
          break;
        }
      }
      if(in_hypbox)
        continue;
      // otherwise we've detected a false negative if there is
      // a foreground pixel here!
      curpix = startpixel + l_uint32((i*imwidth) + j);
      if(getPixelColor(curpix) == color) {
        falseneg++;
        if(dbg) {
          curpixdbg = startpxdbg + l_uint32((i*imwidth)+j);
          setPixelRGB(dbg, curpixdbg, j, i, LayoutEval::GREEN);
        }
      }
    }
  }
  return falseneg;
}

int Lept_Utils::countColorPixels(BOX* box, PIX* pix, \
    LayoutEval::Color color, bool countall_nonwhite) {
  l_int32 x, y, w, h;
  boxGetGeometry(box, &x, &y, &w, &h);
  l_int32 imwidth = pixGetWidth(pix);
  l_uint32* curpixel;
  l_uint32* startpixel = pixGetData(pix);
  rgbtype rgb[3];
  int foreground_count = 0;
  for(l_int32 i = y; i < (y+h); i++) {
    for(l_uint32 j = x; j < (x+w); j++) {
      curpixel = startpixel + ((l_uint32)i * (l_uint32)imwidth) + \
          (l_uint32)j;
      if(!countall_nonwhite) { // default behavior is to
                               // just count the color
        if(getPixelColor(curpixel) == color)
          foreground_count++;
      } else { // otherwise count any non-white color
               // (here we assume the background to be white)
               // TODO: Handle counting foreground pixels regardless
               //       of the background color
        getPixelRGB(curpixel, rgb);
        if(isNonWhite(rgb))
          foreground_count++;
      }
    }
  }
  return foreground_count;
}

