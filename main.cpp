#include "customgraphics.h"
#include <unordered_map>
#include <iostream>
#include <random>
#include <map>
#include <algorithm>
#include "plot.h"

const struct {
    // Constant global dataset with application' global
    // parameters is to be used everywhere
    const char * name;
    size_t x;
    size_t y;
    size_t width;
    size_t height;
    mutable uint8_t id;
} gcWindowParameters {
    "Traffic generator 1.0.1 by Valentyn Faychuk",
    20, 20, 640, 480, 0
};

template <typename Value_t>
static void drawPlot(
        const uint16_t & x,
        const uint16_t & y,
        const float & width,
        const float & height,
        const std::vector<Value_t> & values);

template <typename Value_t>
static void drawPlot3Curves(
        const uint16_t & x,
        const uint16_t & y,
        const float & width,
        const float & height,
        const std::vector<Value_t> & values1,
        const std::vector<Value_t> & values2,
        const std::vector<Value_t> & values3);


bool processPaused{ false };

struct ServiceParameters {
    unsigned mPacketLen = 0;
    double   mIntensity = 0;
    double   mFraction  = 0;
};

enum Service : int {
    DataService,
    VoiceService,
    VideoService,
    CustomService
};
std::unordered_map<Service, double> gProbabilities;
std::unordered_map<Service, double> gPacketLengths;
std::unordered_map<Service, double> gIntensities;

std::unordered_map<Service, ServiceParameters> gParamsOfServices;

unsigned long int gTerminalsCount;
unsigned long int gDurationSeconds;

std::vector<double> generateTraffic(ServiceParameters serviceParams);
std::vector<double> combineTraffic(std::unordered_map<Service, std::vector<double>> allTraffic);
static const BasicFrame gcStatisticsPlotFrame{10, 220, 620, 200};

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static std::unordered_map<Service, std::vector<double>> allTraffic;
    static Plot<size_t> statisticsPlot(std::vector<size_t>({}), gcStatisticsPlotFrame);

    //static std::vector<std::pair<ServiceParameters, std::vector<double>>> allTraffic;

    if (!processPaused)
    {
        allTraffic[Service::DataService]  = generateTraffic(gParamsOfServices[Service::DataService]);
        allTraffic[Service::VideoService] = generateTraffic(gParamsOfServices[Service::VideoService]);
        allTraffic[Service::VoiceService] = generateTraffic(gParamsOfServices[Service::VoiceService]);
        gParamsOfServices[Service::DataService].mIntensity += 0.1;

        std::vector<double> totalTraffic = allTraffic[Service::DataService];
        std::vector<size_t> stat(100, 0);
        std::for_each(totalTraffic.cbegin(), totalTraffic.cend(),
                      [&stat] (const size_t & intensity)
                      {
                          if (intensity < stat.size())
                          {
                              ++stat[intensity];
                          }
                      });
        statisticsPlot.update(stat);
    }

    glColor3d(0.88, 0.15, 0.35);
    statisticsPlot.draw("N occurrences", "intensity", 10);
    // Draw total traffic as well as each traffic type separately
    //drawPlot(10, 220, 620, 200, allTraffic[Service::DataService]);//combineTraffic(allTraffic));
    std::string horizontalLabel{"intensity"};
    std::string verticalLabel  {"time"};
    render3CurvePlot(10, 10, 620, 200,
                     horizontalLabel, verticalLabel,
                     allTraffic[Service::DataService],    // red curve
                     allTraffic[Service::VideoService],   // dashed curve
                     allTraffic[Service::VoiceService]);  // dotted curve);

    /*
    Plot<double> plot(std::vector<double>({1, 2, 3, 4, 5, 6, 9, 1, 1, 12.1664434, 9, 8, 7, 6}),
                      BasicFrame(10,330,200,100));
    glColor4d(0.88, 0.15, 0.35, 1);
    plot.draw("intensity", "time");
    */
    /*

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(190, 230);
    glVertex2f(210, 230);
    glVertex2f(200, 220);
    glVertex2f(200, 240);
    glEnd();


    glColor3f(1.0f, 0.0f, 0.0f);
    renderStrokeText("center highlighted", 200, 230, angle, 1.5f, 0.9f, ALIGN_H_RIGHT | ALIGN_V_UP | HIGHLIGHT_TEXT);
    */
    /*
    renderHorizontalStrokeText("right top", 200, 230, 1.5f, 0.9f, ALIGN_H_RIGHT | ALIGN_V_UP);
    renderHorizontalStrokeText("right bottom", 200, 230, 1.5f, 0.9f, ALIGN_H_RIGHT | ALIGN_V_DOWN);
    renderHorizontalStrokeText("left top", 200, 230, 1.5f, 0.9f, ALIGN_H_LEFT | ALIGN_V_UP);
    renderHorizontalStrokeText("left bottom", 200, 230, 1.5f, 0.9f, ALIGN_H_LEFT | ALIGN_V_DOWN);
    renderHorizontalStrokeText("center highlighted", 200, 230, 1.5f, 0.9f, ALIGN_H_CENTER | ALIGN_V_CENTER | HIGHLIGHT_TEXT);
    */

    glFlush();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case char(27):  // ESCAPE is pressed
            glutDestroyWindow(gcWindowParameters.id);
            exit(EXIT_SUCCESS);
        case char(32):  // SPACE is pressed
            processPaused = !processPaused;
            break;
        default:        // other key is pressed
            std::cout << "x: " << x << ", y: " << y << std::endl;
            std::cout << "key: " << key << std::endl;
            std::cout << "keycode: " << int(key) << std::endl;
    }
}

void timer(int = 0)
{
    display();
    if (!processPaused)
    {
        // Idle by now
    }
    glutTimerFunc(100, timer, 0);
}

static void customInit()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, gcWindowParameters.width, gcWindowParameters.height);
    gluOrtho2D(0, gcWindowParameters.width, 0, gcWindowParameters.height);
    glutPostRedisplay();

    // Fill duration and terminals
    gTerminalsCount  = 1;
    gDurationSeconds = 7000;

    ServiceParameters dataServiceParams;
    dataServiceParams.mIntensity = 1;
    dataServiceParams.mPacketLen = 100;
    dataServiceParams.mFraction  = 0.1;
    gParamsOfServices[Service::DataService] = dataServiceParams;

    ServiceParameters voiceServiceParams;
    voiceServiceParams.mIntensity = 2140;
    voiceServiceParams.mPacketLen = 1000;
    voiceServiceParams.mFraction  = 0.5;
    gParamsOfServices[Service::VoiceService] = voiceServiceParams;

    ServiceParameters videoServiceParams;
    videoServiceParams.mIntensity = 7000;
    videoServiceParams.mPacketLen = 300;
    videoServiceParams.mFraction  = 0.4;
    gParamsOfServices[Service::VideoService] = videoServiceParams;
}

int main(int argc, char **argv)
{
    glutInitWindowSize(gcWindowParameters.width, gcWindowParameters.height);
    glutInitWindowPosition(gcWindowParameters.x, gcWindowParameters.y);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

    glutCreateWindow(gcWindowParameters.name);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000, timer, 0);

    glutSetOption ( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION ) ;
    glClearColor(1, 1, 1, 1);

    gcWindowParameters.id = glutGetWindow();

    customInit();
    glutMainLoop();

    return EXIT_SUCCESS;
}




template <typename Value_t>
static void drawPlot(
        const uint16_t & x,
        const uint16_t & y,
        const float & width,
        const float & height,
        const std::vector<Value_t> & values)
{
    if (values.empty()) return;

    glPushMatrix();
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
    glColor3f(0.75, 0.75, 0.75);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
    glEnd();

    const Value_t maxValue{ *std::max_element(values.cbegin(), values.cend()) };

    // draw plot axes
    glLineWidth(1.2);
    glColor3f(0.1, 0.1, 0.1);
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

    // draw curve
    glLineWidth(1.2);
    glColor3f(0.88, 0.11, 0.23);
    glBegin(GL_LINE_STRIP);
    const float stepX{ width / float(values.size()) };
    const float stepY{ height / float(maxValue)};
    decltype(values.size()) pos{ 0 };
    for (const auto & value : values) {
        glVertex2f(x + stepX/float(2) + stepX*pos++, y + stepY*float(value));
    }
    glEnd();

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glPopMatrix();
}

template <typename Value_t>
static void drawPlot3Curves(
        const uint16_t & x,
        const uint16_t & y,
        const float & width,
        const float & height,
        const std::vector<Value_t> & values1,
        const std::vector<Value_t> & values2,
        const std::vector<Value_t> & values3)
{
    if (values1.empty() || values2.empty() || values3.empty()) return;

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

std::vector<double> generateTraffic(ServiceParameters serviceParams) {
    // Setup randomization engine
    static std::random_device randomDevice;
    static std::mt19937 randomGenerator(randomDevice());

    // Setup calls distribution
    auto callsPerSecond = serviceParams.mIntensity;
    std::poisson_distribution<> poissonianDistribution(callsPerSecond);
    //std::normal_distribution<> poissonianDistribution{callsPerSecond, 7.0};
    //std::uniform_real_distribution<> poissonianDistribution{callsPerSecond};

    // Generate equilibrium traffic for given terminals
    std::vector<double> traffic(gDurationSeconds, 0);
    for (decltype(gTerminalsCount) terminal = 0; terminal < gTerminalsCount; ++terminal)
    {
        for (decltype(gDurationSeconds) secondsPassed = 0; secondsPassed < gDurationSeconds; ++ secondsPassed)
        {
            traffic[secondsPassed] += poissonianDistribution(randomGenerator);
        }
    }
    return traffic;
}

std::vector<double> combineTraffic(std::unordered_map<Service, std::vector<double>> allTraffic) {
    std::vector<double> combinedTraffic(gDurationSeconds, 0);
    std::for_each(allTraffic.cbegin(), allTraffic.cend(),
            [&combinedTraffic] (const std::pair<const Service, std::vector<double>> & serviceTraffic)
            {
                for (decltype(gDurationSeconds) i = 0; i < gDurationSeconds; ++i)
                {
                    combinedTraffic[i] += serviceTraffic.second[i] *
                            gParamsOfServices[serviceTraffic.first].mPacketLen *
                            gParamsOfServices[serviceTraffic.first].mFraction;
                }
            });
    return combinedTraffic;
}