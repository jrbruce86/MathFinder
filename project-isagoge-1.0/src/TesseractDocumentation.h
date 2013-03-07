/**************************************************************************
 * Project Isagoge: Enhancing the OCR Quality of Printed Scientific Documents
 * File name:		TesseractDocumentation.h
 * Written by:	Jake Bruce, Copyright (C) 2013
 * History: 		Created Feb 28, 2013 9:30:45 PM 
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

#ifndef TESSERACTDOCUMENTATION_H_
#define TESSERACTDOCUMENTATION_H_

/**
 * enum of the elements of the page hierarchy, used in ResultIterator
 * to provide functions that operate on each level without having to
 * have 5x as many functions.

 enum PageIteratorLevel {
 RIL_BLOCK,     // Block of text/image/separator line.
 RIL_PARA,      // Paragraph within a block.
 RIL_TEXTLINE,  // Line within a paragraph.
 RIL_WORD,      // Word within a textline.
 RIL_SYMBOL     // Symbol/character within a word.
 };
 ***/

/**
 * Possible modes for page layout analysis. These *must* be kept in order
 * of decreasing amount of layout analysis to be done, except for OSD_ONLY,
 * so that the inequality test macros below work.

 enum PageSegMode {
 PSM_OSD_ONLY,       ///< Orientation and script detection only.
 PSM_AUTO_OSD,       ///< Automatic page segmentation with orientation and
 ///< script detection. (OSD)
 PSM_AUTO_ONLY,      ///< Automatic page segmentation, but no OSD, or OCR.
 PSM_AUTO,           ///< Fully automatic page segmentation, but no OSD.
 PSM_SINGLE_COLUMN,  ///< Assume a single column of text of variable sizes.
 PSM_SINGLE_BLOCK_VERT_TEXT,  ///< Assume a single uniform block of vertically
 ///< aligned text.
 PSM_SINGLE_BLOCK,   ///< Assume a single uniform block of text. (Default.)
 PSM_SINGLE_LINE,    ///< Treat the image as a single text line.
 PSM_SINGLE_WORD,    ///< Treat the image as a single word.
 PSM_CIRCLE_WORD,    ///< Treat the image as a single word in a circle.
 PSM_SINGLE_CHAR,    ///< Treat the image as a single character.
 PSM_SPARSE_TEXT,    ///< Find as much text as possible in no particular order.
 PSM_SPARSE_TEXT_OSD,  ///< Sparse text with orientation and script det.

 PSM_COUNT           ///< Number of enum entries.
 };
 */

/* From ccstruct/blobbox.h:
 / // enum for special type of text characters, such as math symbol or italic.
 * enum BlobSpecialTextType {
 *   BSTT_NONE,  // No special.
 *   BSTT_ITALIC,  // Italic style.
 *   BSTT_DIGIT,  // Digit symbols.
 *   BSTT_MATH,  // Mathmatical symobls (not including digit).
 *   BSTT_UNCLEAR,  // Characters with low recognition rate.
 *   BSTT_SKIP,  // Characters that we skip labeling (usually too small).
 *   BSTT_COUNT
 * };
 // // From textord/equationdetectbase.cpp's RenderSpecialText() function
 * switch (blob->special_text_type()) {
 *    case BSTT_MATH:  // Red box.
 *        pixRenderBoxArb(pix, box, box_width, 255, 0, 0);
 *        break;
 *    case BSTT_DIGIT:  // cyan box.
 *        pixRenderBoxArb(pix, box, box_width, 0, 255, 255);
 *        break;
 *    case BSTT_ITALIC:  // Green box.
 *        pixRenderBoxArb(pix, box, box_width, 0, 255, 0);
 *        break;
 *    case BSTT_UNCLEAR:  // blue box.
 *        pixRenderBoxArb(pix, box, box_width, 0, 255, 0);
 *        break;
 *    case BSTT_NONE:
 *    default:
 *        // yellow box.
 *        pixRenderBoxArb(pix, box, box_width, 255, 255, 0);
 *        break;
 * }*/


#endif /* TESSERACTDOCUMENTATION_H_ */
