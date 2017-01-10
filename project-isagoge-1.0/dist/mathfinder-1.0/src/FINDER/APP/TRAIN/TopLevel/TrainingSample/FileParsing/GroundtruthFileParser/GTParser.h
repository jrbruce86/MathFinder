/**************************************************************************
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
 ***************************************************************************/
#ifndef GTPARSER_H
#define GTPARSER_H

#include <string>

#include <Sample.h>

namespace GtParser {
GroundTruthEntry* parseGTLine(const std::string& line);
}

#endif
