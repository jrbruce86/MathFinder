/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:       DocumentLayoutTest.cpp
 * Written by:  Jake Bruce, Copyright (C) 2013
 * History:         Created Dec 11, 2013 3:01:06 PM
 * Description: Some "Mathematical Expression Detection and Segmentation"
 *              typedefs.
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
#ifndef MEDS_TYPES_H
#define MEDS_TYPES_H

#include <MEDS.h>
#include <Detection.h>
#include <Segmentation.h>

// TODO: Come up with more descriptive detector names and explanations of what they do
//       and how each one differs

typedef MEDS<SVMDetector, Segmentor1> MEDS1;
typedef SVMDetector MEDS1Detector;

typedef MEDS<SVMDetector2, Segmentor1> MEDS2;
typedef SVMDetector2 MEDS2Detector;


#endif
