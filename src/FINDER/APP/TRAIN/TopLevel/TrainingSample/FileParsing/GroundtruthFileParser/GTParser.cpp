/*
 * GTParser.cpp
 *
 *  Created on: Dec 29, 2016
 *      Author: jake
 */

#include <GTParser.h>

GroundTruthEntry* GtParser::parseGTLine(const std::string& line) {
  if(line.empty())
    return NULL;
  std::vector<std::string> splitline = Utils::stringSplit(line);
  std::string imgname = splitline[0];
  std::string type = splitline[1];
  int rectleft   = atoi(splitline[2].c_str());
  int recttop    = atoi(splitline[3].c_str());
  int rectright  = atoi(splitline[4].c_str());
  int rectbottom = atoi(splitline[5].c_str());
  Box* entry_box = boxCreate(rectleft, recttop,
      rectright-rectleft, rectbottom-recttop);
  std::vector<std::string> tmp = Utils::stringSplit(imgname, '.');
  int imgnum = atoi(tmp[0].c_str());
  GT_Entry::GTEntryType entrytype;
  if(type == "displayed")
    entrytype = GT_Entry::DISPLAYED;
  else if(type == "embedded")
    entrytype = GT_Entry::EMBEDDED;
  else if(type == "label")
    entrytype = GT_Entry::LABEL;
  else {
    std::cout << "ERROR: Groundtruth entry of unknown type. Invalid Groundtruth file!\n";
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


