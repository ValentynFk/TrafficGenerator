#include "customgraphics.h"
#include <unordered_map>
#include <iostream>
#include <random>
#include <map>
#include <algorithm>
#include "plot.h"

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
std::unordered_map<Service, size_t>              gCurvesId;

std::unordered_map<Service, ServiceParameters> gParamsOfServices;

unsigned long int gTerminalsCount;
unsigned long int gDurationSeconds;

std::vector<double> generateTraffic(ServiceParameters serviceParams);
std::vector<double> combineTraffic(std::unordered_map<Service, std::vector<double>> allTraffic);
const BasicFrame gcStatisticsPlotFrame{10, 220, 620, 200};

Plot<double> gTrafficPlot(BasicFrame{10, 10, 620, 200});

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static std::unordered_map<Service, std::vector<double>> allTraffic;
    static Plot<size_t> statisticsPlot(gcStatisticsPlotFrame);
    static size_t firstCurveId;
    static bool isInitCycle = true;

    if (isInitCycle) {
        glColor3d(0.88, 0.15, 0.23);
        firstCurveId = statisticsPlot.addCurve("Statistics",
                std::vector<size_t>({}),
                "1111111101010101");
        isInitCycle = false;
    }

    if (!processPaused)
    {
        gTraffics[Service::DataService] = generateTraffic(gParamsOfServices[Service::DataService]);
        gTraffics[Service::VoiceService] = generateTraffic(gParamsOfServices[Service::VoiceService]);
        gTraffics[Service::VideoService] = generateTraffic(gParamsOfServices[Service::VideoService]);

        gParamsOfServices[Service::DataService].mIntensity += 0.1;

        std::vector<size_t> dataTrafficStatistics(100, 0);
        std::vector<size_t> something(300, 500);
        std::for_each(gTraffics[Service::DataService].cbegin(), gTraffics[Service::DataService].cend(),
                      [&dataTrafficStatistics] (const size_t & intensity)
                      {
                          if (intensity < dataTrafficStatistics.size())
                          {
                              ++dataTrafficStatistics[intensity];
                          }
                      });
        statisticsPlot.updateCurve(dataTrafficStatistics);
        gTrafficPlot.updateCurve(gTraffics[Service::DataService],  gCurvesId[Service::DataService]);
        gTrafficPlot.updateCurve(gTraffics[Service::VoiceService], gCurvesId[Service::VoiceService]);
        gTrafficPlot.updateCurve(gTraffics[Service::VideoService], gCurvesId[Service::VideoService]);
    }

    statisticsPlot.draw("N occurrences", "intensity", 10);
    gTrafficPlot.draw("Intensity", "time", 4);

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
    gDurationSeconds = 400;

    ServiceParameters dataServiceParams;
    dataServiceParams.mIntensity = 1;
    dataServiceParams.mPacketLen = 100;
    dataServiceParams.mFraction  = 0.1;
    gParamsOfServices[Service::DataService] = dataServiceParams;

    ServiceParameters voiceServiceParams;
    voiceServiceParams.mIntensity = 140;
    voiceServiceParams.mPacketLen = 1000;
    voiceServiceParams.mFraction  = 0.5;
    gParamsOfServices[Service::VoiceService] = voiceServiceParams;

    ServiceParameters videoServiceParams;
    videoServiceParams.mIntensity = 300;
    videoServiceParams.mPacketLen = 300;
    videoServiceParams.mFraction  = 0.4;
    gParamsOfServices[Service::VideoService] = videoServiceParams;

    glColor3d(0.88, 0.15, 0.23);
    gCurvesId[Service::DataService]  = gTrafficPlot.addCurve("Data service", std::vector<double>(), "1111111111111111");
    glColor3d(0.15, 0.92, 0.33);
    gCurvesId[Service::VoiceService] = gTrafficPlot.addCurve("Voice service", std::vector<double>(), "0011001100110011");
    glColor3d(0.21, 0.21, 0.21);
    gCurvesId[Service::VideoService] = gTrafficPlot.addCurve("Video service", std::vector<double>(), "1100110011001100");
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