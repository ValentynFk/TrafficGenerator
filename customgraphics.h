#ifndef TRAFFICGENERATOR_CUSTOMGRAPHICS_H
#define TRAFFICGENERATOR_CUSTOMGRAPHICS_H

// Custom definitions for
#define ALIGN_H_CENTER	(uint8_t)0x01
#define ALIGN_H_RIGHT   (uint8_t)0x02
#define ALIGN_H_LEFT	(uint8_t)0x04
#define ALIGN_V_CENTER	(uint8_t)0x10
#define ALIGN_V_UP		(uint8_t)0x20
#define ALIGN_V_DOWN	(uint8_t)0x40
#define HIGHLIGHT_TEXT  (uint8_t)0x08

#include <string>

void renderStrokeText(const std::string & context,
                      size_t x, size_t y, size_t angle, float thickness, float size, uint8_t flags);

#endif //TRAFFICGENERATOR_CUSTOMGRAPHICS_H
