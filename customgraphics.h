//
// Created by faitc on 9/26/2019.
//

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

#include <GL/freeglut.h>
#include <GL/glu.h>
#include <algorithm>
#include <string>
#include <vector>

void renderHorizontalStrokeText(const std::string & context,
                                size_t x, size_t y, float thickness, float size, uint8_t flags);
void renderStrokeText(const std::string & context,
                      size_t x, size_t y, size_t angle, float thickness, float size, uint8_t flags);

template <typename Value_t>
void render3CurvePlot(
        const size_t x, const size_t y,
        const float width, const float height,
        const std::string & verticalLabel,
        const std::string & horizontalLabel,
        const std::vector<Value_t> & values1,
        const std::vector<Value_t> & values2,
        const std::vector<Value_t> & values3);

template <typename Value_t>
void render3CurvePlot(
        const size_t x, const size_t y,
        const float width, const float height,
        const std::string & verticalLabel,
        const std::string & horizontalLabel,
        const std::vector<Value_t> & values1,
        const std::vector<Value_t> & values2,
        const std::vector<Value_t> & values3)
{
    if (values1.empty() || values2.empty() || values3.empty())
    {
        return;
    }

    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw plot background
    glColor3f(0.98, 0.98, 0.98);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
    glEnd();

    // draw plot borders
    glLineWidth(0.5);
    glLineStipple(1, 0xFFFF);
    glColor3f(0.75, 0.75, 0.75);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
    glEnd();

    const Value_t maxValue{ std::max({
                                             *std::max_element(values1.cbegin(), values1.cend()),
                                             *std::max_element(values2.cbegin(), values2.cend()),
                                             *std::max_element(values3.cbegin(), values3.cend())
                                     }) };
    // draw plot axes
    glLineWidth(0.6);
    glColor4f(0.1, 0.1, 0.1, 0.5);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glEnd();

    // draw vertical axis arrow
    glBegin(GL_POLYGON);
    glVertex2f(float(x)-2.5, y + height - 5);
    glVertex2f(x, y + height);
    glVertex2f(float(x)+2.5, y + height - 5);
    glEnd();

    // draw horizontal axis arrow
    glBegin(GL_POLYGON);
    glVertex2f(x + width - 5, float(y)-2.5);
    glVertex2f(x + width, y);
    glVertex2f(x + width - 5, float(y)+2.5);
    glEnd();

    glColor4f(0.1, 0.1, 0.1, 1);;
    renderStrokeText(verticalLabel, x-2, y + height/2.0f, 90, 1.8f, 0.9f, ALIGN_H_CENTER | ALIGN_V_DOWN);
    renderStrokeText(verticalLabel, x + width/2.0f, y-2, 0, 1.8f, 0.9f, ALIGN_H_CENTER | ALIGN_V_UP);

    // calculate parameters of the curves
    const decltype(values1.size()) maxSize{ std::max({
                                                             values1.size(),
                                                             values2.size(),
                                                             values3.size()
                                                     }) };
    const float stepX{ float(width) / float(maxSize) };
    const float stepY{ float(height) / float(maxValue)};
    // draw curves
    // first curve (red)
    glLineWidth(1.2);
    glLineStipple(1, 0xFFFF);
    glColor3f(0.88, 0.11, 0.23);
    glBegin(GL_LINE_STRIP);
    decltype(values1.size()) pos{ 0 };
    for (const auto & value : values1) {
        glVertex2f(x + stepX/float(2) + stepX*pos++, y + stepY*float(value));
    }
    glEnd();
    // second curve (black dashed)
    glLineStipple(1, 0xF0F0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    pos = 0;
    for (const auto & value : values2) {
        glVertex2f(x + stepX/float(2) + stepX*pos++, y + stepY*float(value));
    }
    glEnd();
    // third curve (black dotted)
    glLineStipple(1, 0x3333);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    pos = 0;
    for (const auto & value : values3) {
        glVertex2f(x + stepX/float(2) + stepX*pos++, y + stepY*float(value));
    }
    glEnd();

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
    glPopMatrix();
}

#endif //TRAFFICGENERATOR_CUSTOMGRAPHICS_H
