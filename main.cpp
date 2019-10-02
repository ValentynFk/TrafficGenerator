#include "customgraphics.h"
#include <unordered_map>
#include <iostream>
#include <random>
#include <map>
#include <algorithm>
#include "plot.h"
#include "trafficgenerator.h"

static bool processPaused{ false };

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

enum Service : int {
    DataService,
    VoiceService,
    VideoService
};
struct ServiceParameters {
    unsigned mPacketLen = 0;
    double   mIntensity = 0;
    double   mFraction  = 0;
};

std::unordered_map<Service, std::vector<double>> gTraffics;
std::vector<double> gCombinedTraffics;

std::unordered_map<Service, ServiceParameters> gServicesParam;

unsigned long int gTerminalsCount;
unsigned long int gDurationSeconds;

std::vector<double> combineTraffic(std::unordered_map<Service, std::vector<double>> allTraffic);

Plot<size_t> gStatistPlot(BasicFrame{10, 220, 620, 200});
Plot<double> gTrafficPlot(BasicFrame{10, 10, 300, 200});
Plot<double> gTotalPlot(BasicFrame{320, 10, 300, 200});

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!processPaused)
    {
        gTraffics[Service::DataService]  = generateTraffic(gServicesParam[Service::DataService].mIntensity,  gDurationSeconds);
        gTraffics[Service::VoiceService] = generateTraffic(gServicesParam[Service::VoiceService].mIntensity, gDurationSeconds);
        gTraffics[Service::VideoService] = generateTraffic(gServicesParam[Service::VideoService].mIntensity, gDurationSeconds);
        gCombinedTraffics = combineTraffic(gTraffics);

        gTrafficPlot.updateCurve(gTraffics[Service::DataService],  "Data service");
        gTrafficPlot.updateCurve(gTraffics[Service::VoiceService], "Voice service");
        gTrafficPlot.updateCurve(gTraffics[Service::VideoService], "Video service");
        gTotalPlot.updateCurve(gCombinedTraffics, "All services");

        std::vector<size_t> dataTrafficStatistics(100, 0);
        std::for_each(gTraffics[Service::DataService].cbegin(), gTraffics[Service::DataService].cend(),
                      [&dataTrafficStatistics] (const size_t & intensity)
                      {
                          if (intensity < dataTrafficStatistics.size())
                          {
                              ++dataTrafficStatistics[intensity];
                          }
                      });
        std::vector<size_t> combinedTrafficStatistics(100, 0);
        std::for_each(gCombinedTraffics.cbegin(), gCombinedTraffics.cend(),
                      [&combinedTrafficStatistics] (const size_t & intensity)
                      {
                          if (intensity < combinedTrafficStatistics.size())
                          {
                              ++combinedTrafficStatistics[intensity];
                          }
                      });
        gStatistPlot.updateCurve(dataTrafficStatistics, "Data service statistics");
        gStatistPlot.updateCurve(combinedTrafficStatistics, "All services statistics");
    }

    gStatistPlot.draw("N occurrences", "Intensity, c/s", 7);
    gTrafficPlot.draw("Intensity", "time, s", 4);
    gTotalPlot.draw("Intensity", "time, s", 10);

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
        gServicesParam[Service::DataService].mIntensity += 0.1;
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
    gTerminalsCount  = 10;
    gDurationSeconds = 750;

    ServiceParameters dataServiceParams;
    dataServiceParams.mIntensity = 1;
    dataServiceParams.mPacketLen = 1;
    dataServiceParams.mFraction  = 0.6;
    gServicesParam[Service::DataService] = dataServiceParams;

    ServiceParameters voiceServiceParams;
    voiceServiceParams.mIntensity = 10;
    voiceServiceParams.mPacketLen = 1;
    voiceServiceParams.mFraction  = 0.2;
    gServicesParam[Service::VoiceService] = voiceServiceParams;

    ServiceParameters videoServiceParams;
    videoServiceParams.mIntensity = 15;
    videoServiceParams.mPacketLen = 3;
    videoServiceParams.mFraction  = 0.2;
    gServicesParam[Service::VideoService] = videoServiceParams;

    glLineWidth(1.2);

    glColor3d(0.88, 0.15, 0.23);
    gStatistPlot.addCurve("Data service statistics", "1100110011001100");
    glColor3d(0.1, 0.1, 0.1);
    gStatistPlot.addCurve("All services statistics");


    glColor3d(0.1, 0.1, 0.1);
    gTotalPlot.addCurve("All services",    "1111111111111111");
    glColor3d(0.88, 0.15, 0.23);
    gTrafficPlot.addCurve("Data service",  "1111111111111111");
    glColor3d(0.15, 0.72, 0.33);
    gTrafficPlot.addCurve("Voice service", "1111001100111111");
    glColor3d(0.21, 0.21, 0.21);
    gTrafficPlot.addCurve("Video service", "1100110011001100");
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

std::vector<double> combineTraffic(std::unordered_map<Service, std::vector<double>> allTraffic) {
    std::vector<double> combinedTraffic(gDurationSeconds, 0);
    std::for_each(allTraffic.cbegin(), allTraffic.cend(),
            [&combinedTraffic] (const std::pair<const Service, std::vector<double>> & serviceTraffic)
            {
                for (decltype(gDurationSeconds) i = 0; i < gDurationSeconds; ++i)
                {
                    combinedTraffic[i] += serviceTraffic.second[i] *
                                          gServicesParam[serviceTraffic.first].mPacketLen *
                                          gServicesParam[serviceTraffic.first].mFraction;
                }
            });
    return combinedTraffic;
}