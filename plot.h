#ifndef TRAFFICGENERATOR_PLOT_H
#define TRAFFICGENERATOR_PLOT_H

#include "customgraphics.h"

#include <GL/freeglut.h>
#include <GL/glu.h>

#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <bitset>

class BasicFrame {
public:
    BasicFrame(size_t x, size_t y, size_t width, size_t height) noexcept
        : mX(x), mY(y), mWidth(width), mHeight(height) {
    }
    BasicFrame(const BasicFrame & basicFrame) noexcept = default;
    BasicFrame(BasicFrame && basicFrame)      noexcept = default;
protected:
    const size_t mX, mY;
    const size_t mWidth, mHeight;
};

template <class Value_t>
class Plot : public BasicFrame {
    struct Curve {
        std::vector<Value_t> mData;
        double               mColorR, mColorG, mColorB, mColorA;
        double               mWidth;
        unsigned short int   mPattern;
        Curve() = default;
    }; std::unordered_map<std::string, Curve> mCurves;
public:
    explicit Plot(const BasicFrame & baseFrame) noexcept
        : BasicFrame(baseFrame){
    };

    void addCurve(const std::string & curveName    = "",
                  const std::string & curvePattern = "1111111111111111") {
        // Append curve
        Curve newCurve;
        newCurve.mPattern = std::bitset<16>(curvePattern).to_ulong();
        double curveColor[4]; // Embed color of the curve from current color
        glGetDoublev(GL_CURRENT_COLOR, curveColor);
        newCurve.mColorR = curveColor[0];
        newCurve.mColorG = curveColor[1];
        newCurve.mColorB = curveColor[2];
        newCurve.mColorA = curveColor[3];
        double curveWidth[1]; // Embed width of the curve from current width
        glGetDoublev(GL_LINE_WIDTH, curveWidth);
        newCurve.mWidth = curveWidth[0];
        mCurves.emplace(curveName, newCurve);
    }

    template <class D1,
            typename = typename std::enable_if<
                    std::is_convertible<D1, std::vector<Value_t>>::value
            >::type>
    void updateCurve(D1&& curveUpdatedData, const std::string & curveName = "") {
        if (mCurves.find(curveName) != mCurves.end()) {
            mCurves[curveName].mData = std::forward<D1>(curveUpdatedData);
            // Update curves shared maximum and length
            mCurvesTotalMax = 0;
            mCurvesTotalLen = 0;
            for (const auto & curve : mCurves) {
                auto pCurrentCurveMax =
                        std::max_element(curve.second.mData.cbegin(), curve.second.mData.cend());
                if (pCurrentCurveMax != curve.second.mData.cend()) {
                    mCurvesTotalMax = (*pCurrentCurveMax > mCurvesTotalMax) ?
                            *pCurrentCurveMax : mCurvesTotalMax;
                }
                mCurvesTotalLen = (curve.second.mData.size() > mCurvesTotalLen) ?
                        curve.second.mData.size() : mCurvesTotalLen;
            }
        }
    }

    void draw(const std::string & vLabel, const std::string & hLabel, size_t hLinesNum = 1);

private:
    Value_t   mCurvesTotalMax = 0;
    size_t    mCurvesTotalLen = 0;

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
    if (mCurves.empty()) {
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
    glPushAttrib(GL_ENABLE_BIT);
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

    // Draw curves
    glEnable(GL_LINE_STIPPLE);
    for(const auto & keyCurveRelation : mCurves) {
        const Curve currentCurve = keyCurveRelation.second;
        if (currentCurve.mData.empty()) {
            continue;
        }
        glLineStipple(1, currentCurve.mPattern);
        glLineWidth(currentCurve.mWidth);
        glColor4d(currentCurve.mColorR,
                  currentCurve.mColorG,
                  currentCurve.mColorB,
                  currentCurve.mColorA);
        glBegin(GL_LINE_STRIP);
        const double dX{static_cast<double>(width)  / static_cast<double>(mCurvesTotalLen-1)};
        const double dY{static_cast<double>(height) / static_cast<double>(mCurvesTotalMax)};
        size_t num = 0;
        std::for_each(currentCurve.mData.cbegin(), currentCurve.mData.cend(),
                      [&num, x, dX, y, dY, this] (const Value_t & val) {
                          glVertex2d(x + dX * num++, y + dY * val);
                      });
        glEnd();
    }
    glDisable(GL_LINE_STIPPLE);

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
        const Value_t vDiff{mCurvesTotalMax / static_cast<Value_t>(hLinesNum)};
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
    renderStrokeText(valToStr(mCurvesTotalLen), x + width, y - 3, 0,
                     1.2f, 0.5f, ALIGN_H_CENTER | ALIGN_V_UP);

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glPopAttrib();
    glPopMatrix();
    //==============================================================================================
    ///                                                                           </DRAWING_SECTION>
    //==============================================================================================

    glColor4dv(colorOfCurveBuff);
    glLineWidth(widthOfLineBuff[0]);
}


#endif //TRAFFICGENERATOR_PLOT_H
