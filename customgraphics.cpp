#include <GL/freeglut.h>
#include <GL/glu.h>

#include "customgraphics.h"
#include <algorithm>

void renderStrokeText(
        const std::string & context,
        const size_t x, const size_t y, const size_t angle,
        const float thickness, const float size,
        uint8_t flags)
{
    float horizontalOffset = 0.0f;
    float verticalOffset   = 0.0f;
    // Configure for horizontal alignment
    if      (flags & ALIGN_H_CENTER) horizontalOffset = (context.length() / 2.0f) * 9.4f * size;
    else if (flags & ALIGN_H_RIGHT)  horizontalOffset = context.length() * 9.4f * size;
    else if (flags & ALIGN_H_LEFT)   horizontalOffset = 0.0f;
    // Configure for vertical alignment
    if      (flags & ALIGN_V_CENTER) verticalOffset = 4.0f * size;
    else if (flags & ALIGN_V_UP)     verticalOffset = 8.0f * size;
    else if (flags & ALIGN_V_DOWN)   verticalOffset = 0.0f;

    glPushMatrix();
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    if (flags & HIGHLIGHT_TEXT)
    {
        // Push current active color to the buffer
        float colorOfTheTextBuff[4];
        glGetFloatv(GL_CURRENT_COLOR, colorOfTheTextBuff);
        // Draw set of rectangles on corresponding layers
        for (uint8_t layer = 0; layer < 5; ++layer) {
            float layerTransparency = 0.6f - static_cast<float>(layer) * 0.1f;
            glColor4f(1.0f, 1.0f, 1.0f, layerTransparency);

            float layerScale = static_cast<float>(layer) * 0.7f;
            glRectf(
                    0 - horizontalOffset - layerScale,
                    0 - verticalOffset - layerScale,
                    0 - horizontalOffset + context.length() * 9.4f * size + layerScale,
                    0 - verticalOffset + 8 * size + layerScale
            );
        }
        glColor4fv(colorOfTheTextBuff);
    }

    glTranslatef(-horizontalOffset, -verticalOffset, 0);
    glScalef(0.09 * size, 0.08 * size, 0);
    glLineWidth(thickness);
    std::for_each(context.cbegin(), context.cend(),
                  [] (const char & ch)
                  {
                      glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, ch);
                  });

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glPopMatrix();
}

