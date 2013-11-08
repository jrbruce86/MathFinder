/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:   GTParser.h
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:     Created Nov 3, 2013 12:31:52 PM
 * ------------------------------------------------------------------------
 * Description: Simple function to parse a line of text from a manually
 *              generated math groundtruth file for an image. The expected
 *              format is exemplified as follows:
 *              1.png embedded 2750 570 2796 630
 *              Where 1 is the image index (indexed from 1), png is the
 *              image extension, "embedded" is the type which can be
 *              one of the following: embedded, displayed, label. The
 *              last four numbers are the left, top, right, and bottom
 *              of the entry's rectangle.
 * ------------------------------------------------------------------------
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
#ifndef GTPARSER_H
#define GTPARSER_H

#include <string>
using namespace std;

#include <Sample.h>

namespace GT_Parser {
GroundTruthEntry* parseGTLine(const string& line) {
  if(line.empty())
    return NULL;
  vector<string> splitline = Basic_Utils::stringSplit(line);
  string imgname = splitline[0];
  string type = splitline[1];
  int rectleft   = atoi(splitline[2].c_str());
  int recttop    = atoi(splitline[3].c_str());
  int rectright  = atoi(splitline[4].c_str());
  int rectbottom = atoi(splitline[5].c_str());
  Box* entry_box = boxCreate(rectleft, recttop,
      rectright-rectleft, rectbottom-recttop);
  vector<string> tmp = Basic_Utils::stringSplit(imgname, '.');
  int imgnum = atoi(tmp[0].c_str());
  GT_Entry::GTEntryType entrytype;
  if(type == "displayed")
    entrytype = GT_Entry::DISPLAYED;
  else if(type == "embedded")
    entrytype = GT_Entry::EMBEDDED;
  else if(type == "label")
    entrytype = GT_Entry::LABEL;
  else {
    cout << "ERROR: Groundtruth entry of unknown type. Invalid Groundtruth file!\n";
    exit(EXIT_FAILURE);
  }
  // use the parsed info to create the entry
  // caller is responsible for entry's destruction
  GroundTruthEntry* gt_entry = new GroundTruthEntry;
  gt_entry->entry = entrytype;
  gt_entry->image_index = imgnum;
  gt_entry->rect = entry_box;
  for(int i = 0; i < splitline.size(); i++)
    splitline[i].erase();
  splitline.clear();
  return gt_entry;
}
}

#endif
