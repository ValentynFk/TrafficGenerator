#ifndef TRAFFICGENERATOR_PLOT_H
#define TRAFFICGENERATOR_PLOT_H

#include "customgraphics.h"

#include <GL/freeglut.h>
#include <GL/glu.h>

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>

class BasicFrame {
public:
    BasicFrame(size_t x, size_t y, size_t width, size_t height)
        : mX(x), mY(y), mWidth(width), mHeight(height) {
    }
    BasicFrame(const BasicFrame & basicFrame) = default;
    BasicFrame(BasicFrame && basicFrame)      = default;
protected:
    const size_t mX, mY;
    const size_t mWidth, mHeight;
};

template <class Value_t>
class Plot : public BasicFrame {
public:
    template <class D1,
            typename = typename std::enable_if<
                    std::is_convertible<D1, std::vector<Value_t>>::value
                    >::type>
    explicit Plot(D1&& initData, const BasicFrame & baseFrame)
        : mData(std::forward<D1>(initData)), BasicFrame(baseFrame) {
        auto pDataMaximum = std::max_element(mData.cbegin(), mData.cend());
        mDataMaximum = (pDataMaximum != mData.cend())? *pDataMaximum : 0;
    };

    template <class D1>
    void update(D1&& newData) {
        mData = std::forward<D1>(newData);
        auto pDataMaximum = std::max_element(mData.cbegin(), mData.cend());
        mDataMaximum = (pDataMaximum != mData.cend())? *pDataMaximum : 0;
    }

    void draw(const std::string & vLabel, const std::string & hLabel, size_t hLinesNum = 1);

private:
    std::vector<Value_t> mData;
    Value_t mDataMaximum;

    template <typename Numeric_t,
            typename = typename std::enable_if<
                    std::is_convertible<Numeric_t, long double>::value
            >::type>
    std::string valToStr(Numeric_t&& val)
    {
        std::stringstream stream;
        stream << val;
        return stream.str();
    }
};

template<class Value_t>
void Plot<Value_t>::draw(
        const std::string & vLabel,
        const std::string & hLabel,
        const size_t hLinesNum) {
    if (mData.size() < 2) {
        return;
    }

    const size_t vTextHeight{10};
    const size_t hTextHeight{15};
    size_t x{mX + vTextHeight}, y{mY + hTextHeight};
    size_t width{mWidth - vTextHeight}, height{mHeight - hTextHeight};

    double colorOfCurveBuff[4];
    glGetDoublev(GL_CURRENT_COLOR, colorOfCurveBuff);
    double widthOfLineBuff[1];
    glGetDoublev(GL_LINE_WIDTH, widthOfLineBuff);

    //==============================================================================================
    ///                                                                            <DRAWING_SECTION>
    //==============================================================================================
    glPushMatrix();
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw plot background & outline
    glColor3f(0.98, 0.98, 0.98);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
    glEnd();
    glLineWidth(0.7); // background outline
    glColor3f(0.75, 0.75, 0.75);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
    glEnd();
    if (hLinesNum) {
        glBegin(GL_LINES);
        const double vDistance{static_cast<double>(height) / static_cast<double>(hLinesNum)};
        double lineY{static_cast<double>(y) + vDistance};
        for (size_t hLineId = 0; hLineId < hLinesNum; ++hLineId) {
            glVertex2f(x, lineY);
            glVertex2f(x + width, lineY);
            lineY += vDistance;
        }
        glEnd();
    }

    // draw the curve
    glLineWidth(1.2);
    glColor4d(colorOfCurveBuff[0],
              colorOfCurveBuff[1],
              colorOfCurveBuff[2],
              colorOfCurveBuff[3]);
    glBegin(GL_LINE_STRIP);
    const double dX{static_cast<double>(width)  / static_cast<double>(mData.size()-1)};
    const double dY{static_cast<double>(height) / static_cast<double>(mDataMaximum)};
    size_t num = 0;
    std::for_each(mData.cbegin(), mData.cend(),
                  [&num, x, dX, y, dY, this] (const Value_t & val) {
                      glVertex2d(x + dX * num++, y + dY * val);
                  });
    glEnd();

    // draw plot axes along with corresponding arrows
    glLineWidth(1.2);
    glColor3f(0.1, 0.1, 0.1);
    glBegin(GL_LINES); // horizontal & vertical axes
    glVertex2f(x, y);
    glVertex2f(x, y + height - 4);
    glVertex2f(x, y);
    glVertex2f(x + width - 4, y);
    glEnd();
    glBegin(GL_POLYGON); // vertical axis arrow
    glVertex2f(float(x)-2.1, y + height - 5);
    glVertex2f(x, y + height);
    glVertex2f(float(x)+2.1, y + height - 5);
    glEnd();
    glBegin(GL_POLYGON); // horizontal axis arrow
    glVertex2f(x + width - 5, float(y)-2.1);
    glVertex2f(x + width, y);
    glVertex2f(x + width - 5, float(y)+2.1);
    glEnd();

    // draw both vertical and horizontal labels
    glColor3f(0.1, 0.1, 0.1);
    renderStrokeText(vLabel, mX, y + height/2.0f, 90,
                     1.8f, 0.7f, ALIGN_H_CENTER | ALIGN_V_UP);
    renderStrokeText(hLabel, x + width/2.0f, mY, 0,
                     1.8f, 0.7f, ALIGN_H_CENTER | ALIGN_V_DOWN);

    if (hLinesNum) {
        const Value_t vDiff{mDataMaximum / static_cast<Value_t>(hLinesNum)};
        Value_t lineVal{vDiff};
        const double vDistance{static_cast<double>(height) / static_cast<double>(hLinesNum)};
        double lineY{static_cast<double>(y) + vDistance};
        for (size_t hLineId = 0; hLineId < hLinesNum; ++hLineId) {
            renderStrokeText(valToStr(lineVal), x + 4, lineY, 0,
                             1.2f, 0.4f, ALIGN_H_LEFT | ALIGN_V_CENTER | HIGHLIGHT_TEXT);
            lineVal += vDiff;
            lineY += vDistance;
        }
    }

    renderStrokeText(valToStr(0), x, y - 3, 0,
                     1.2f, 0.5f, ALIGN_H_CENTER | ALIGN_V_UP);
    renderStrokeText(valToStr(mData.size()), x + width, y - 3, 0,
                     1.2f, 0.5f, ALIGN_H_CENTER | ALIGN_V_UP);

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glPopMatrix();
    //==============================================================================================
    ///                                                                           </DRAWING_SECTION>
    //==============================================================================================

    glColor4dv(colorOfCurveBuff);
    glLineWidth(widthOfLineBuff[0]);
}


#endif //TRAFFICGENERATOR_PLOT_H
