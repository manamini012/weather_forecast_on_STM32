/*
 * image.h
 *
 *  Created on: 31 ����. 2019 �.
 *      Author: tabur
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdint.h>

typedef struct _tImage
{
  uint16_t 					xSize;
  uint16_t 					ySize;
  uint16_t 					bytesPerLine;
  uint8_t					bitsPerPixel;
  const uint16_t* 		pData;
} sImage;

#define GUI_BITMAP			sImage
#define GUI_CONST_STORAGE	const

extern GUI_CONST_STORAGE GUI_BITMAP imageNight;
extern GUI_CONST_STORAGE GUI_BITMAP imageDay;

#endif /* IMAGE_H_ */
