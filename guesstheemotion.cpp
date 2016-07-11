#include "guesstheemotion.h"

#include <alcommon/albroker.h>
#include <alcommon/almodule.h>
#include <alcommon/albrokermanager.h>
#include <alcommon/altoolsmain.h>

#include <alproxies/almotionproxy.h>
#include <alproxies/altexttospeechproxy.h>
#include <alproxies/alledsproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/alspeechrecognitionproxy.h>
#include <alproxies/alrobotpostureproxy.h>
#include <alproxies/altouchproxy.h>
#include <alproxies/alanimatedspeechproxy.h>

#include <althread/almutex.h>
#include <althread/alcriticalsection.h>

#include <alvalue/alvalue.h>

#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <qi/os.hpp>
#include <time.h>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>


/*Initialize the starting static variables*/
bool GuessTheEmotion::start = false;
int GuessTheEmotion::asrCheck = 0;
int GuessTheEmotion::emotionCheck = 0;

GuessTheEmotion::GuessTheEmotion(boost::shared_ptr<AL::ALBroker> broker,
                   const std::string& name)
  : AL::ALModule(broker, name),
    mutex(AL::ALMutex::createALMutex())
{
    // Module Description for webpage.
    setModuleDescription("Interactive game where the robot displays different emotions and asks the user to classify them.");

    std::cout << "Module Loaded" << std::endl;



    // // Binding methods to the module
    // Pattern:
    // functionName("name", getName(), "description");
    // addParam("param", "");
    // setReturn("return", "");
    // BIND_METHOD(GuessTheEmotion::name);


    functionName("faceDetection", getName(), "The robot begins to look for a face in its camera view.");
    BIND_METHOD(GuessTheEmotion::faceDetection);

    functionName("onFaceDetected", getName(), "Event handler for recognizing a face.");
    BIND_METHOD(GuessTheEmotion::onFaceDetected);

    functionName("asrGame", getName(), "Initiates speech recognition for the emotion");
    BIND_METHOD(GuessTheEmotion::asrGame);

    functionName("onWordRecognized", getName(), "Event handler for word recognition");
    addParam("name", "");
    addParam("val", "");
    addParam("name", "");
    BIND_METHOD(GuessTheEmotion::onWordRecognized);

    functionName("rightBumperTouch", getName(), "Right bumper touch initiator");
    BIND_METHOD(GuessTheEmotion::rightBumperTouch);

    functionName("onRightBumperTouch", getName(), "Right bumper touch event handler");
    BIND_METHOD(GuessTheEmotion::onRightBumperTouch);

    functionName("getStartState", getName(), "Returns the start state boolean value");
    setReturn("bool", "True or False depending on if the robot has started the interaction or not");
    BIND_METHOD(GuessTheEmotion::getStartState);

    functionName("setStartState", getName(), "Sets the start state to the desired value (true or false)");
    addParam("state", "");
    BIND_METHOD(GuessTheEmotion::setStartState);

    functionName("greeting", getName(), "Displays the greeting motion.");
    BIND_METHOD(GuessTheEmotion::greeting);

    functionName("say", getName(), "Says the given phrase.");
    addParam("phrase", "");
    BIND_METHOD(GuessTheEmotion::say);

    functionName("animatedSay", getName(), "Says the given phrase with animated speech");
    addParam("phrase", "");
    BIND_METHOD(GuessTheEmotion::animatedSay);

    functionName("ledsOff", getName(), "Turns the eye LEDs to white");
    BIND_METHOD(GuessTheEmotion::ledsOff);

    functionName("cheer", getName(), "Displays the cheer motion.");
    BIND_METHOD(GuessTheEmotion::cheer);

    functionName("wave", getName(), "Displays the wave motion.");
    BIND_METHOD(GuessTheEmotion::wave);

    functionName("rest", getName(), "Turns joint stiffness to zero.");
    BIND_METHOD(GuessTheEmotion::rest);

    functionName("stand", getName(), "Robot goes to standing posture.");
    BIND_METHOD(GuessTheEmotion::stand);

    functionName("robotIsWakeUp", getName(), "Checks robot to see if it's in rest state or not.");
    setReturn("bool", "True or False depending on if the robot is awake or at rest");
    BIND_METHOD(GuessTheEmotion::robotIsWakeUp);

    functionName("wakeUp", getName(), "Wakes the robot up.");
    BIND_METHOD(GuessTheEmotion::wakeUp);

    functionName("asrPause", getName(), "Pauses speech recognition.");
    BIND_METHOD(GuessTheEmotion::asrPause);

    functionName("asrAgain", getName(), "Final speech recognition method for restarting a new game");
    BIND_METHOD(GuessTheEmotion::asrAgain);

    functionName("happy", getName(), "The robot performs the happy emotion.");
    BIND_METHOD(GuessTheEmotion::happy);

    functionName("sad", getName(), "The robot displays the sad emotion");
    BIND_METHOD(GuessTheEmotion::sad);

    functionName("angry", getName(), "The robot displays the angry emotion");
    BIND_METHOD(GuessTheEmotion::angry);

    functionName("scared", getName(), "The robot displays the scared emotion");
    BIND_METHOD(GuessTheEmotion::scared);

    /* Necessary Proxies for this Module" */

    try
    {
        motion = AL::ALMotionProxy(broker);
    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }
    try
    {
        eyes = AL::ALLedsProxy(broker);
        // Group of eye LEDs
        std::vector<std::string> lednames;
        lednames.push_back("FaceLedRight0");
        lednames.push_back("FaceLedRight1");
        lednames.push_back("FaceLedRight2");
        lednames.push_back("FaceLedRight3");
        lednames.push_back("FaceLedRight4");
        lednames.push_back("FaceLedRight5");
        lednames.push_back("FaceLedRight6");
        lednames.push_back("FaceLedRight7");
        lednames.push_back("FaceLedLeft0");
        lednames.push_back("FaceLedLeft1");
        lednames.push_back("FaceLedLeft2");
        lednames.push_back("FaceLedLeft3");
        lednames.push_back("FaceLedLeft4");
        lednames.push_back("FaceLedLeft5");
        lednames.push_back("FaceLedLeft6");
        lednames.push_back("FaceLedLeft7");
        eyes.createGroup("Eyes",lednames);



    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }
    try
    {
        speech = AL::ALTextToSpeechProxy(broker);
        speech.setParameter("pitchShift", 1.0);

    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }

    try
    {
        memory = AL::ALMemoryProxy(broker);

    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }

    try
    {
        asr = AL::ALSpeechRecognitionProxy(broker);

    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }

    try
    {
        posture = AL::ALRobotPostureProxy(broker);
    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }

}

/* Destructor */
GuessTheEmotion::~GuessTheEmotion()
{
    try
    {
        memory.unsubscribeToEvent("WordRecognized", "GuessTheEmotion");
        memory.unsubscribeToEvent("FaceDetected", "GuessTheEmotion");
        asr.pause(true);
    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }

}

/* Method Definitions */

bool GuessTheEmotion::getStartState()
{
    return this->start;
}

void GuessTheEmotion::setStartState(bool startstate)
{
    this->start = startstate;
}

bool GuessTheEmotion::robotIsWakeUp()
{
  return motion.robotIsWakeUp();
}

void GuessTheEmotion::wakeUp()
{
  if (motion.robotIsWakeUp() == false)
  {
      motion.wakeUp();
  }
}

void GuessTheEmotion::rest()
{
    motion.rest();
}

void GuessTheEmotion::asrPause()
{
    asr.pause(true);
    this->resetEyes();
}

void GuessTheEmotion::asrGame()
{
    asr.setLanguage("English");
    std::vector<std::string> emotionlist;
    emotionlist.push_back("happy");
    emotionlist.push_back("sad");
    emotionlist.push_back("scared");
    emotionlist.push_back("angry");
    asr.setVocabulary(emotionlist, false);
    memory.subscribeToEvent("WordRecognized", "GuessTheEmotion","onWordRecognized");
    asr.pause(false);
}

void GuessTheEmotion::asrAgain()
{
    std::vector<std::string> endlist;
    endlist.push_back("yes");
    endlist.push_back("no");
    asr.setVocabulary(endlist, false);
    memory.subscribeToEvent("WordRecognized", "GuessTheEmotion","onWordRecognized");
    asr.pause(false);
}


void GuessTheEmotion::resetEyes()
{
    eyes.fadeRGB("Eyes", "white", 0.5);
}

void GuessTheEmotion::wave()
{
    // Custom bezier motion imported from Choregraphe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(14);
    times.arraySetSize(14);
    keys.arraySetSize(14);

    names.push_back("HeadPitch");
    times[0].arraySetSize(7);
    keys[0].arraySetSize(7);

    times[0][0] = 0.8;
    keys[0][0] = AL::ALValue::array(0.29602, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.253333, 0));
    times[0][1] = 1.56;
    keys[0][1] = AL::ALValue::array(-0.170316, AL::ALValue::array(3, -0.253333, 0.111996), AL::ALValue::array(3, 0.226667, -0.100207));
    times[0][2] = 2.24;
    keys[0][2] = AL::ALValue::array(-0.340591, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[0][3] = 2.8;
    keys[0][3] = AL::ALValue::array(-0.0598679, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[0][4] = 3.48;
    keys[0][4] = AL::ALValue::array(-0.193327, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[0][5] = 4.6;
    keys[0][5] = AL::ALValue::array(-0.01078, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][6] = 5;
    keys[0][6] = AL::ALValue::array(-0.14884, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(7);
    keys[1].arraySetSize(7);

    times[1][0] = 0.8;
    keys[1][0] = AL::ALValue::array(-0.135034, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.253333, 0));
    times[1][1] = 1.56;
    keys[1][1] = AL::ALValue::array(-0.351328, AL::ALValue::array(3, -0.253333, 0.0493864), AL::ALValue::array(3, 0.226667, -0.0441878));
    times[1][2] = 2.24;
    keys[1][2] = AL::ALValue::array(-0.415757, AL::ALValue::array(3, -0.226667, 0.00372364), AL::ALValue::array(3, 0.186667, -0.00306653));
    times[1][3] = 2.8;
    keys[1][3] = AL::ALValue::array(-0.418823, AL::ALValue::array(3, -0.186667, 0.00306653), AL::ALValue::array(3, 0.226667, -0.00372364));
    times[1][4] = 3.48;
    keys[1][4] = AL::ALValue::array(-0.520068, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[1][5] = 4.6;
    keys[1][5] = AL::ALValue::array(-0.375872, AL::ALValue::array(3, -0.373333, -0.122074), AL::ALValue::array(3, 0.133333, 0.0435979));
    times[1][6] = 5;
    keys[1][6] = AL::ALValue::array(-0.023052, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[2].arraySetSize(7);
    keys[2].arraySetSize(7);

    times[2][0] = 0.72;
    keys[2][0] = AL::ALValue::array(-1.37902, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[2][1] = 1.48;
    keys[2][1] = AL::ALValue::array(-1.29005, AL::ALValue::array(3, -0.253333, -0.0345436), AL::ALValue::array(3, 0.226667, 0.0309074));
    times[2][2] = 2.16;
    keys[2][2] = AL::ALValue::array(-1.18267, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[2][3] = 2.72;
    keys[2][3] = AL::ALValue::array(-1.24863, AL::ALValue::array(3, -0.186667, 0.0205524), AL::ALValue::array(3, 0.226667, -0.0249565));
    times[2][4] = 3.4;
    keys[2][4] = AL::ALValue::array(-1.3192, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[2][5] = 4.52;
    keys[2][5] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.373333, -0.134993), AL::ALValue::array(3, 0.16, 0.057854));
    times[2][6] = 5;
    keys[2][6] = AL::ALValue::array(-0.41874, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[3].arraySetSize(7);
    keys[3].arraySetSize(7);

    times[3][0] = 0.72;
    keys[3][0] = AL::ALValue::array(-0.803859, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[3][1] = 1.48;
    keys[3][1] = AL::ALValue::array(-0.691876, AL::ALValue::array(3, -0.253333, -0.0137171), AL::ALValue::array(3, 0.226667, 0.0122732));
    times[3][2] = 2.16;
    keys[3][2] = AL::ALValue::array(-0.679603, AL::ALValue::array(3, -0.226667, -0.0122732), AL::ALValue::array(3, 0.186667, 0.0101073));
    times[3][3] = 2.72;
    keys[3][3] = AL::ALValue::array(-0.610574, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[3][4] = 3.4;
    keys[3][4] = AL::ALValue::array(-0.753235, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[3][5] = 4.52;
    keys[3][5] = AL::ALValue::array(-0.6704, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.16, 0));
    times[3][6] = 5;
    keys[3][6] = AL::ALValue::array(-1.17048, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[4].arraySetSize(3);
    keys[4].arraySetSize(3);

    times[4][0] = 1.48;
    keys[4][0] = AL::ALValue::array(0.238207, AL::ALValue::array(3, -0.493333, 0), AL::ALValue::array(3, 1.01333, 0));
    times[4][1] = 4.52;
    keys[4][1] = AL::ALValue::array(0.240025, AL::ALValue::array(3, -1.01333, -0.001818), AL::ALValue::array(3, 0.16, 0.000287053));
    times[4][2] = 5;
    keys[4][2] = AL::ALValue::array(0.282, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[5].arraySetSize(7);
    keys[5].arraySetSize(7);

    times[5][0] = 0.72;
    keys[5][0] = AL::ALValue::array(1.11824, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[5][1] = 1.48;
    keys[5][1] = AL::ALValue::array(0.928028, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[5][2] = 2.16;
    keys[5][2] = AL::ALValue::array(0.9403, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[5][3] = 2.72;
    keys[5][3] = AL::ALValue::array(0.862065, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[5][4] = 3.4;
    keys[5][4] = AL::ALValue::array(0.897349, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[5][5] = 4.52;
    keys[5][5] = AL::ALValue::array(0.842125, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.16, 0));
    times[5][6] = 5;
    keys[5][6] = AL::ALValue::array(1.46186, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[6].arraySetSize(7);
    keys[6].arraySetSize(7);

    times[6][0] = 0.72;
    keys[6][0] = AL::ALValue::array(0.363515, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[6][1] = 1.48;
    keys[6][1] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.253333, 0.0257175), AL::ALValue::array(3, 0.226667, -0.0230104));
    times[6][2] = 2.16;
    keys[6][2] = AL::ALValue::array(0.20398, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[6][3] = 2.72;
    keys[6][3] = AL::ALValue::array(0.217786, AL::ALValue::array(3, -0.186667, -0.00669692), AL::ALValue::array(3, 0.226667, 0.00813198));
    times[6][4] = 3.4;
    keys[6][4] = AL::ALValue::array(0.248467, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[6][5] = 4.52;
    keys[6][5] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.373333, 0.0214763), AL::ALValue::array(3, 0.16, -0.00920412));
    times[6][6] = 5;
    keys[6][6] = AL::ALValue::array(0.154892, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[7].arraySetSize(3);
    keys[7].arraySetSize(3);

    times[7][0] = 1.48;
    keys[7][0] = AL::ALValue::array(0.147222, AL::ALValue::array(3, -0.493333, 0), AL::ALValue::array(3, 1.01333, 0));
    times[7][1] = 4.52;
    keys[7][1] = AL::ALValue::array(0.11961, AL::ALValue::array(3, -1.01333, 0.0097153), AL::ALValue::array(3, 0.16, -0.00153399));
    times[7][2] = 5;
    keys[7][2] = AL::ALValue::array(0.113474, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[8].arraySetSize(11);
    keys[8].arraySetSize(11);

    times[8][0] = 0.64;
    keys[8][0] = AL::ALValue::array(1.38524, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[8][1] = 1.4;
    keys[8][1] = AL::ALValue::array(0.242414, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.0933333, 0));
    times[8][2] = 1.68;
    keys[8][2] = AL::ALValue::array(0.349066, AL::ALValue::array(3, -0.0933333, -0.0949577), AL::ALValue::array(3, 0.133333, 0.135654));
    times[8][3] = 2.08;
    keys[8][3] = AL::ALValue::array(0.934249, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.106667, 0));
    times[8][4] = 2.4;
    keys[8][4] = AL::ALValue::array(0.680678, AL::ALValue::array(3, -0.106667, 0.141383), AL::ALValue::array(3, 0.08, -0.106037));
    times[8][5] = 2.64;
    keys[8][5] = AL::ALValue::array(0.191986, AL::ALValue::array(3, -0.08, 0), AL::ALValue::array(3, 0.133333, 0));
    times[8][6] = 3.04;
    keys[8][6] = AL::ALValue::array(0.261799, AL::ALValue::array(3, -0.133333, -0.0698132), AL::ALValue::array(3, 0.0933333, 0.0488692));
    times[8][7] = 3.32;
    keys[8][7] = AL::ALValue::array(0.707216, AL::ALValue::array(3, -0.0933333, -0.103967), AL::ALValue::array(3, 0.133333, 0.148524));
    times[8][8] = 3.72;
    keys[8][8] = AL::ALValue::array(1.01927, AL::ALValue::array(3, -0.133333, -0.0664734), AL::ALValue::array(3, 0.24, 0.119652));
    times[8][9] = 4.44;
    keys[8][9] = AL::ALValue::array(1.26559, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.186667, 0));
    times[8][10] = 5;
    keys[8][10] = AL::ALValue::array(0.403484, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[9].arraySetSize(8);
    keys[9].arraySetSize(8);

    times[9][0] = 0.64;
    keys[9][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[9][1] = 1.4;
    keys[9][1] = AL::ALValue::array(0.564471, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[9][2] = 2.08;
    keys[9][2] = AL::ALValue::array(0.391128, AL::ALValue::array(3, -0.226667, 0.0395378), AL::ALValue::array(3, 0.186667, -0.0325606));
    times[9][3] = 2.64;
    keys[9][3] = AL::ALValue::array(0.348176, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[9][4] = 3.32;
    keys[9][4] = AL::ALValue::array(0.381923, AL::ALValue::array(3, -0.226667, -0.0337477), AL::ALValue::array(3, 0.133333, 0.0198516));
    times[9][5] = 3.72;
    keys[9][5] = AL::ALValue::array(0.977384, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.24, 0));
    times[9][6] = 4.44;
    keys[9][6] = AL::ALValue::array(0.826783, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.186667, 0));
    times[9][7] = 5;
    keys[9][7] = AL::ALValue::array(1.175, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[10].arraySetSize(4);
    keys[10].arraySetSize(4);

    times[10][0] = 1.4;
    keys[10][0] = AL::ALValue::array(0.853478, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.64, 0));
    times[10][1] = 3.32;
    keys[10][1] = AL::ALValue::array(0.854933, AL::ALValue::array(3, -0.64, 0), AL::ALValue::array(3, 0.373333, 0));
    times[10][2] = 4.44;
    keys[10][2] = AL::ALValue::array(0.425116, AL::ALValue::array(3, -0.373333, 0.119852), AL::ALValue::array(3, 0.186667, -0.0599259));
    times[10][3] = 5;
    keys[10][3] = AL::ALValue::array(0.3156, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[11].arraySetSize(7);
    keys[11].arraySetSize(7);

    times[11][0] = 0.64;
    keys[11][0] = AL::ALValue::array(0.247016, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[11][1] = 1.4;
    keys[11][1] = AL::ALValue::array(-1.17193, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[11][2] = 2.08;
    keys[11][2] = AL::ALValue::array(-1.0891, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[11][3] = 2.64;
    keys[11][3] = AL::ALValue::array(-1.26091, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[11][4] = 3.32;
    keys[11][4] = AL::ALValue::array(-1.14892, AL::ALValue::array(3, -0.226667, -0.111982), AL::ALValue::array(3, 0.373333, 0.184441));
    times[11][5] = 4.44;
    keys[11][5] = AL::ALValue::array(1.02015, AL::ALValue::array(3, -0.373333, -0.581557), AL::ALValue::array(3, 0.186667, 0.290778));
    times[11][6] = 5;
    keys[11][6] = AL::ALValue::array(1.46808, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[12].arraySetSize(7);
    keys[12].arraySetSize(7);

    times[12][0] = 0.64;
    keys[12][0] = AL::ALValue::array(-0.242414, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[12][1] = 1.4;
    keys[12][1] = AL::ALValue::array(-0.954191, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[12][2] = 2.08;
    keys[12][2] = AL::ALValue::array(-0.460242, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[12][3] = 2.64;
    keys[12][3] = AL::ALValue::array(-0.960325, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[12][4] = 3.32;
    keys[12][4] = AL::ALValue::array(-0.328317, AL::ALValue::array(3, -0.226667, -0.0474984), AL::ALValue::array(3, 0.373333, 0.0782326));
    times[12][5] = 4.44;
    keys[12][5] = AL::ALValue::array(-0.250085, AL::ALValue::array(3, -0.373333, -0.0381794), AL::ALValue::array(3, 0.186667, 0.0190897));
    times[12][6] = 5;
    keys[12][6] = AL::ALValue::array(-0.15651, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[13].arraySetSize(4);
    keys[13].arraySetSize(4);

    times[13][0] = 1.4;
    keys[13][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.64, 0));
    times[13][1] = 3.32;
    keys[13][1] = AL::ALValue::array(-0.303775, AL::ALValue::array(3, -0.64, -0.00920312), AL::ALValue::array(3, 0.373333, 0.00536849));
    times[13][2] = 4.44;
    keys[13][2] = AL::ALValue::array(0.182504, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.186667, 0));
    times[13][3] = 5;
    keys[13][3] = AL::ALValue::array(0.118076, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
}

void GuessTheEmotion::greeting()
{
    try
    {
        this->wakeUp();
    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }


    // Custom bezier motion imported from Choregraphe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(14);
    times.arraySetSize(14);
    keys.arraySetSize(14);

    names.push_back("HeadPitch");
    times[0].arraySetSize(7);
    keys[0].arraySetSize(7);

    times[0][0] = 0.8;
    keys[0][0] = AL::ALValue::array(0.29602, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.253333, 0));
    times[0][1] = 1.56;
    keys[0][1] = AL::ALValue::array(-0.170316, AL::ALValue::array(3, -0.253333, 0.111996), AL::ALValue::array(3, 0.226667, -0.100207));
    times[0][2] = 2.24;
    keys[0][2] = AL::ALValue::array(-0.340591, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[0][3] = 2.8;
    keys[0][3] = AL::ALValue::array(-0.0598679, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[0][4] = 3.48;
    keys[0][4] = AL::ALValue::array(-0.193327, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[0][5] = 4.6;
    keys[0][5] = AL::ALValue::array(-0.01078, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][6] = 5;
    keys[0][6] = AL::ALValue::array(-0.14884, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(7);
    keys[1].arraySetSize(7);

    times[1][0] = 0.8;
    keys[1][0] = AL::ALValue::array(-0.135034, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.253333, 0));
    times[1][1] = 1.56;
    keys[1][1] = AL::ALValue::array(-0.351328, AL::ALValue::array(3, -0.253333, 0.0493864), AL::ALValue::array(3, 0.226667, -0.0441878));
    times[1][2] = 2.24;
    keys[1][2] = AL::ALValue::array(-0.415757, AL::ALValue::array(3, -0.226667, 0.00372364), AL::ALValue::array(3, 0.186667, -0.00306653));
    times[1][3] = 2.8;
    keys[1][3] = AL::ALValue::array(-0.418823, AL::ALValue::array(3, -0.186667, 0.00306653), AL::ALValue::array(3, 0.226667, -0.00372364));
    times[1][4] = 3.48;
    keys[1][4] = AL::ALValue::array(-0.520068, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[1][5] = 4.6;
    keys[1][5] = AL::ALValue::array(-0.375872, AL::ALValue::array(3, -0.373333, -0.122074), AL::ALValue::array(3, 0.133333, 0.0435979));
    times[1][6] = 5;
    keys[1][6] = AL::ALValue::array(-0.023052, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[2].arraySetSize(7);
    keys[2].arraySetSize(7);

    times[2][0] = 0.72;
    keys[2][0] = AL::ALValue::array(-1.37902, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[2][1] = 1.48;
    keys[2][1] = AL::ALValue::array(-1.29005, AL::ALValue::array(3, -0.253333, -0.0345436), AL::ALValue::array(3, 0.226667, 0.0309074));
    times[2][2] = 2.16;
    keys[2][2] = AL::ALValue::array(-1.18267, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[2][3] = 2.72;
    keys[2][3] = AL::ALValue::array(-1.24863, AL::ALValue::array(3, -0.186667, 0.0205524), AL::ALValue::array(3, 0.226667, -0.0249565));
    times[2][4] = 3.4;
    keys[2][4] = AL::ALValue::array(-1.3192, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[2][5] = 4.52;
    keys[2][5] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.373333, -0.134993), AL::ALValue::array(3, 0.16, 0.057854));
    times[2][6] = 5;
    keys[2][6] = AL::ALValue::array(-0.41874, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[3].arraySetSize(7);
    keys[3].arraySetSize(7);

    times[3][0] = 0.72;
    keys[3][0] = AL::ALValue::array(-0.803859, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[3][1] = 1.48;
    keys[3][1] = AL::ALValue::array(-0.691876, AL::ALValue::array(3, -0.253333, -0.0137171), AL::ALValue::array(3, 0.226667, 0.0122732));
    times[3][2] = 2.16;
    keys[3][2] = AL::ALValue::array(-0.679603, AL::ALValue::array(3, -0.226667, -0.0122732), AL::ALValue::array(3, 0.186667, 0.0101073));
    times[3][3] = 2.72;
    keys[3][3] = AL::ALValue::array(-0.610574, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[3][4] = 3.4;
    keys[3][4] = AL::ALValue::array(-0.753235, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[3][5] = 4.52;
    keys[3][5] = AL::ALValue::array(-0.6704, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.16, 0));
    times[3][6] = 5;
    keys[3][6] = AL::ALValue::array(-1.17048, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[4].arraySetSize(3);
    keys[4].arraySetSize(3);

    times[4][0] = 1.48;
    keys[4][0] = AL::ALValue::array(0.238207, AL::ALValue::array(3, -0.493333, 0), AL::ALValue::array(3, 1.01333, 0));
    times[4][1] = 4.52;
    keys[4][1] = AL::ALValue::array(0.240025, AL::ALValue::array(3, -1.01333, -0.001818), AL::ALValue::array(3, 0.16, 0.000287053));
    times[4][2] = 5;
    keys[4][2] = AL::ALValue::array(0.282, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[5].arraySetSize(7);
    keys[5].arraySetSize(7);

    times[5][0] = 0.72;
    keys[5][0] = AL::ALValue::array(1.11824, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[5][1] = 1.48;
    keys[5][1] = AL::ALValue::array(0.928028, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[5][2] = 2.16;
    keys[5][2] = AL::ALValue::array(0.9403, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[5][3] = 2.72;
    keys[5][3] = AL::ALValue::array(0.862065, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[5][4] = 3.4;
    keys[5][4] = AL::ALValue::array(0.897349, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[5][5] = 4.52;
    keys[5][5] = AL::ALValue::array(0.842125, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.16, 0));
    times[5][6] = 5;
    keys[5][6] = AL::ALValue::array(1.46186, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[6].arraySetSize(7);
    keys[6].arraySetSize(7);

    times[6][0] = 0.72;
    keys[6][0] = AL::ALValue::array(0.363515, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.253333, 0));
    times[6][1] = 1.48;
    keys[6][1] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.253333, 0.0257175), AL::ALValue::array(3, 0.226667, -0.0230104));
    times[6][2] = 2.16;
    keys[6][2] = AL::ALValue::array(0.20398, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[6][3] = 2.72;
    keys[6][3] = AL::ALValue::array(0.217786, AL::ALValue::array(3, -0.186667, -0.00669692), AL::ALValue::array(3, 0.226667, 0.00813198));
    times[6][4] = 3.4;
    keys[6][4] = AL::ALValue::array(0.248467, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.373333, 0));
    times[6][5] = 4.52;
    keys[6][5] = AL::ALValue::array(0.226991, AL::ALValue::array(3, -0.373333, 0.0214763), AL::ALValue::array(3, 0.16, -0.00920412));
    times[6][6] = 5;
    keys[6][6] = AL::ALValue::array(0.154892, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[7].arraySetSize(3);
    keys[7].arraySetSize(3);

    times[7][0] = 1.48;
    keys[7][0] = AL::ALValue::array(0.147222, AL::ALValue::array(3, -0.493333, 0), AL::ALValue::array(3, 1.01333, 0));
    times[7][1] = 4.52;
    keys[7][1] = AL::ALValue::array(0.11961, AL::ALValue::array(3, -1.01333, 0.0097153), AL::ALValue::array(3, 0.16, -0.00153399));
    times[7][2] = 5;
    keys[7][2] = AL::ALValue::array(0.113474, AL::ALValue::array(3, -0.16, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[8].arraySetSize(11);
    keys[8].arraySetSize(11);

    times[8][0] = 0.64;
    keys[8][0] = AL::ALValue::array(1.38524, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[8][1] = 1.4;
    keys[8][1] = AL::ALValue::array(0.242414, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.0933333, 0));
    times[8][2] = 1.68;
    keys[8][2] = AL::ALValue::array(0.349066, AL::ALValue::array(3, -0.0933333, -0.0949577), AL::ALValue::array(3, 0.133333, 0.135654));
    times[8][3] = 2.08;
    keys[8][3] = AL::ALValue::array(0.934249, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.106667, 0));
    times[8][4] = 2.4;
    keys[8][4] = AL::ALValue::array(0.680678, AL::ALValue::array(3, -0.106667, 0.141383), AL::ALValue::array(3, 0.08, -0.106037));
    times[8][5] = 2.64;
    keys[8][5] = AL::ALValue::array(0.191986, AL::ALValue::array(3, -0.08, 0), AL::ALValue::array(3, 0.133333, 0));
    times[8][6] = 3.04;
    keys[8][6] = AL::ALValue::array(0.261799, AL::ALValue::array(3, -0.133333, -0.0698132), AL::ALValue::array(3, 0.0933333, 0.0488692));
    times[8][7] = 3.32;
    keys[8][7] = AL::ALValue::array(0.707216, AL::ALValue::array(3, -0.0933333, -0.103967), AL::ALValue::array(3, 0.133333, 0.148524));
    times[8][8] = 3.72;
    keys[8][8] = AL::ALValue::array(1.01927, AL::ALValue::array(3, -0.133333, -0.0664734), AL::ALValue::array(3, 0.24, 0.119652));
    times[8][9] = 4.44;
    keys[8][9] = AL::ALValue::array(1.26559, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.186667, 0));
    times[8][10] = 5;
    keys[8][10] = AL::ALValue::array(0.403484, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[9].arraySetSize(8);
    keys[9].arraySetSize(8);

    times[9][0] = 0.64;
    keys[9][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[9][1] = 1.4;
    keys[9][1] = AL::ALValue::array(0.564471, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[9][2] = 2.08;
    keys[9][2] = AL::ALValue::array(0.391128, AL::ALValue::array(3, -0.226667, 0.0395378), AL::ALValue::array(3, 0.186667, -0.0325606));
    times[9][3] = 2.64;
    keys[9][3] = AL::ALValue::array(0.348176, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[9][4] = 3.32;
    keys[9][4] = AL::ALValue::array(0.381923, AL::ALValue::array(3, -0.226667, -0.0337477), AL::ALValue::array(3, 0.133333, 0.0198516));
    times[9][5] = 3.72;
    keys[9][5] = AL::ALValue::array(0.977384, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.24, 0));
    times[9][6] = 4.44;
    keys[9][6] = AL::ALValue::array(0.826783, AL::ALValue::array(3, -0.24, 0), AL::ALValue::array(3, 0.186667, 0));
    times[9][7] = 5;
    keys[9][7] = AL::ALValue::array(1.175, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[10].arraySetSize(4);
    keys[10].arraySetSize(4);

    times[10][0] = 1.4;
    keys[10][0] = AL::ALValue::array(0.853478, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.64, 0));
    times[10][1] = 3.32;
    keys[10][1] = AL::ALValue::array(0.854933, AL::ALValue::array(3, -0.64, 0), AL::ALValue::array(3, 0.373333, 0));
    times[10][2] = 4.44;
    keys[10][2] = AL::ALValue::array(0.425116, AL::ALValue::array(3, -0.373333, 0.119852), AL::ALValue::array(3, 0.186667, -0.0599259));
    times[10][3] = 5;
    keys[10][3] = AL::ALValue::array(0.3156, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[11].arraySetSize(7);
    keys[11].arraySetSize(7);

    times[11][0] = 0.64;
    keys[11][0] = AL::ALValue::array(0.247016, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[11][1] = 1.4;
    keys[11][1] = AL::ALValue::array(-1.17193, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[11][2] = 2.08;
    keys[11][2] = AL::ALValue::array(-1.0891, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[11][3] = 2.64;
    keys[11][3] = AL::ALValue::array(-1.26091, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[11][4] = 3.32;
    keys[11][4] = AL::ALValue::array(-1.14892, AL::ALValue::array(3, -0.226667, -0.111982), AL::ALValue::array(3, 0.373333, 0.184441));
    times[11][5] = 4.44;
    keys[11][5] = AL::ALValue::array(1.02015, AL::ALValue::array(3, -0.373333, -0.581557), AL::ALValue::array(3, 0.186667, 0.290778));
    times[11][6] = 5;
    keys[11][6] = AL::ALValue::array(1.46808, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[12].arraySetSize(7);
    keys[12].arraySetSize(7);

    times[12][0] = 0.64;
    keys[12][0] = AL::ALValue::array(-0.242414, AL::ALValue::array(3, -0.213333, 0), AL::ALValue::array(3, 0.253333, 0));
    times[12][1] = 1.4;
    keys[12][1] = AL::ALValue::array(-0.954191, AL::ALValue::array(3, -0.253333, 0), AL::ALValue::array(3, 0.226667, 0));
    times[12][2] = 2.08;
    keys[12][2] = AL::ALValue::array(-0.460242, AL::ALValue::array(3, -0.226667, 0), AL::ALValue::array(3, 0.186667, 0));
    times[12][3] = 2.64;
    keys[12][3] = AL::ALValue::array(-0.960325, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0.226667, 0));
    times[12][4] = 3.32;
    keys[12][4] = AL::ALValue::array(-0.328317, AL::ALValue::array(3, -0.226667, -0.0474984), AL::ALValue::array(3, 0.373333, 0.0782326));
    times[12][5] = 4.44;
    keys[12][5] = AL::ALValue::array(-0.250085, AL::ALValue::array(3, -0.373333, -0.0381794), AL::ALValue::array(3, 0.186667, 0.0190897));
    times[12][6] = 5;
    keys[12][6] = AL::ALValue::array(-0.15651, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[13].arraySetSize(4);
    keys[13].arraySetSize(4);

    times[13][0] = 1.4;
    keys[13][0] = AL::ALValue::array(-0.312978, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.64, 0));
    times[13][1] = 3.32;
    keys[13][1] = AL::ALValue::array(-0.303775, AL::ALValue::array(3, -0.64, -0.00920312), AL::ALValue::array(3, 0.373333, 0.00536849));
    times[13][2] = 4.44;
    keys[13][2] = AL::ALValue::array(0.182504, AL::ALValue::array(3, -0.373333, 0), AL::ALValue::array(3, 0.186667, 0));
    times[13][3] = 5;
    keys[13][3] = AL::ALValue::array(0.118076, AL::ALValue::array(3, -0.186667, 0), AL::ALValue::array(3, 0, 0));

    speech.post.say("Hi there, my name is Leia. Would you like to play a game with me? Say yes or no");
    std::cout << "Say yes or no" << std::endl;
    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
}

void GuessTheEmotion::cheer()
{
    //Bezier custom motion imported from Choregrahe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(26);
    times.arraySetSize(26);
    keys.arraySetSize(26);

    names.push_back("HeadPitch");
    times[0].arraySetSize(4);
    keys[0].arraySetSize(4);

    times[0][0] = 1.6;
    keys[0][0] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[0][1] = 2.6;
    keys[0][1] = AL::ALValue::array(0.00149202, AL::ALValue::array(3, -0.333333, -7.5855e-10), AL::ALValue::array(3, 0.4, 9.1026e-10));
    times[0][2] = 3.8;
    keys[0][2] = AL::ALValue::array(0.00149202, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[0][3] = 6.2;
    keys[0][3] = AL::ALValue::array(-0.14884, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(4);
    keys[1].arraySetSize(4);

    times[1][0] = 1.6;
    keys[1][0] = AL::ALValue::array(0, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[1][1] = 2.6;
    keys[1][1] = AL::ALValue::array(0, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[1][2] = 3.8;
    keys[1][2] = AL::ALValue::array(0, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[1][3] = 6.2;
    keys[1][3] = AL::ALValue::array(-0.016916, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnklePitch");
    times[2].arraySetSize(4);
    keys[2].arraySetSize(4);

    times[2][0] = 1.6;
    keys[2][0] = AL::ALValue::array(0.098134, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[2][1] = 2.6;
    keys[2][1] = AL::ALValue::array(-0.349794, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[2][2] = 3.8;
    keys[2][2] = AL::ALValue::array(-0.349794, AL::ALValue::array(3, -0.4, -2.66316e-07), AL::ALValue::array(3, 0.8, 5.32632e-07));
    times[2][3] = 6.2;
    keys[2][3] = AL::ALValue::array(0.0950661, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnkleRoll");
    times[3].arraySetSize(4);
    keys[3].arraySetSize(4);

    times[3][0] = 1.6;
    keys[3][0] = AL::ALValue::array(-0.125746, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[3][1] = 2.6;
    keys[3][1] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.333333, -2.16729e-09), AL::ALValue::array(3, 0.4, 2.60074e-09));
    times[3][2] = 3.8;
    keys[3][2] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[3][3] = 6.2;
    keys[3][3] = AL::ALValue::array(-0.121144, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[4].arraySetSize(6);
    keys[4].arraySetSize(6);

    times[4][0] = 1.6;
    keys[4][0] = AL::ALValue::array(-0.41107, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[4][1] = 2.6;
    keys[4][1] = AL::ALValue::array(-1.54462, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.2, 0));
    times[4][2] = 3.2;
    keys[4][2] = AL::ALValue::array(-0.476475, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[4][3] = 3.8;
    keys[4][3] = AL::ALValue::array(-1.54462, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[4][4] = 4.4;
    keys[4][4] = AL::ALValue::array(-0.486947, AL::ALValue::array(3, -0.2, -0.0247809), AL::ALValue::array(3, 0.6, 0.0743427));
    times[4][5] = 6.2;
    keys[4][5] = AL::ALValue::array(-0.412604, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[5].arraySetSize(4);
    keys[5].arraySetSize(4);

    times[5][0] = 1.6;
    keys[5][0] = AL::ALValue::array(-1.21804, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[5][1] = 2.6;
    keys[5][1] = AL::ALValue::array(-1.28456, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[5][2] = 3.8;
    keys[5][2] = AL::ALValue::array(-1.28456, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[5][3] = 6.2;
    keys[5][3] = AL::ALValue::array(-1.21957, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[6].arraySetSize(6);
    keys[6].arraySetSize(6);

    times[6][0] = 1.6;
    keys[6][0] = AL::ALValue::array(0.2916, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[6][1] = 2.6;
    keys[6][1] = AL::ALValue::array(0, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.2, 0));
    times[6][2] = 3.2;
    keys[6][2] = AL::ALValue::array(1, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[6][3] = 3.8;
    keys[6][3] = AL::ALValue::array(0, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[6][4] = 4.4;
    keys[6][4] = AL::ALValue::array(1, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.6, 0));
    times[6][5] = 6.2;
    keys[6][5] = AL::ALValue::array(0.9916, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipPitch");
    times[7].arraySetSize(4);
    keys[7].arraySetSize(4);

    times[7][0] = 1.6;
    keys[7][0] = AL::ALValue::array(0.131966, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[7][1] = 2.6;
    keys[7][1] = AL::ALValue::array(-0.444818, AL::ALValue::array(3, -0.333333, 8.32238e-08), AL::ALValue::array(3, 0.4, -9.98685e-08));
    times[7][2] = 3.8;
    keys[7][2] = AL::ALValue::array(-0.444818, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[7][3] = 6.2;
    keys[7][3] = AL::ALValue::array(0.1335, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipRoll");
    times[8].arraySetSize(4);
    keys[8].arraySetSize(4);

    times[8][0] = 1.6;
    keys[8][0] = AL::ALValue::array(0.09515, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[8][1] = 2.6;
    keys[8][1] = AL::ALValue::array(0.00924587, AL::ALValue::array(3, -0.333333, 6.93532e-09), AL::ALValue::array(3, 0.4, -8.32238e-09));
    times[8][2] = 3.8;
    keys[8][2] = AL::ALValue::array(0.00924586, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[8][3] = 6.2;
    keys[8][3] = AL::ALValue::array(0.0997519, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipYawPitch");
    times[9].arraySetSize(4);
    keys[9].arraySetSize(4);

    times[9][0] = 1.6;
    keys[9][0] = AL::ALValue::array(-0.164096, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[9][1] = 2.6;
    keys[9][1] = AL::ALValue::array(-0.00149202, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[9][2] = 3.8;
    keys[9][2] = AL::ALValue::array(-0.00149202, AL::ALValue::array(3, -0.4, 9.1026e-10), AL::ALValue::array(3, 0.8, -1.82052e-09));
    times[9][3] = 6.2;
    keys[9][3] = AL::ALValue::array(-0.170232, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LKneePitch");
    times[10].arraySetSize(4);
    keys[10].arraySetSize(4);

    times[10][0] = 1.6;
    keys[10][0] = AL::ALValue::array(-0.092082, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[10][1] = 2.6;
    keys[10][1] = AL::ALValue::array(0.704064, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[10][2] = 3.8;
    keys[10][2] = AL::ALValue::array(0.704064, AL::ALValue::array(3, -0.4, 6.6579e-08), AL::ALValue::array(3, 0.8, -1.33158e-07));
    times[10][3] = 6.2;
    keys[10][3] = AL::ALValue::array(-0.0844119, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[11].arraySetSize(6);
    keys[11].arraySetSize(6);

    times[11][0] = 1.6;
    keys[11][0] = AL::ALValue::array(1.46646, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[11][1] = 2.6;
    keys[11][1] = AL::ALValue::array(0.242601, AL::ALValue::array(3, -0.333333, 0.525861), AL::ALValue::array(3, 0.2, -0.315516));
    times[11][2] = 3.2;
    keys[11][2] = AL::ALValue::array(-1.05767, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[11][3] = 3.8;
    keys[11][3] = AL::ALValue::array(0.242601, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[11][4] = 4.4;
    keys[11][4] = AL::ALValue::array(-0.973894, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.6, 0));
    times[11][5] = 6.2;
    keys[11][5] = AL::ALValue::array(1.45726, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[12].arraySetSize(6);
    keys[12].arraySetSize(6);

    times[12][0] = 1.6;
    keys[12][0] = AL::ALValue::array(0.170232, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[12][1] = 2.6;
    keys[12][1] = AL::ALValue::array(0.118682, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[12][2] = 3.8;
    keys[12][2] = AL::ALValue::array(0.118682, AL::ALValue::array(3, -0.4, -8.32238e-09), AL::ALValue::array(3, 0.2, 4.16119e-09));
    times[12][3] = 4.4;
    keys[12][3] = AL::ALValue::array(0.136136, AL::ALValue::array(3, -0.2, -0.0174533), AL::ALValue::array(3, 0.173333, 0.0151262));
    times[12][4] = 4.92;
    keys[12][4] = AL::ALValue::array(0.382227, AL::ALValue::array(3, -0.173333, 0), AL::ALValue::array(3, 0.426667, 0));
    times[12][5] = 6.2;
    keys[12][5] = AL::ALValue::array(0.164096, AL::ALValue::array(3, -0.426667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[13].arraySetSize(4);
    keys[13].arraySetSize(4);

    times[13][0] = 1.6;
    keys[13][0] = AL::ALValue::array(0.0843279, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[13][1] = 2.6;
    keys[13][1] = AL::ALValue::array(-0.129154, AL::ALValue::array(3, -0.333333, 6.93532e-09), AL::ALValue::array(3, 0.4, -8.32238e-09));
    times[13][2] = 3.8;
    keys[13][2] = AL::ALValue::array(-0.129154, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[13][3] = 6.2;
    keys[13][3] = AL::ALValue::array(0.0873961, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnklePitch");
    times[14].arraySetSize(4);
    keys[14].arraySetSize(4);

    times[14][0] = 1.6;
    keys[14][0] = AL::ALValue::array(0.098218, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[14][1] = 2.6;
    keys[14][1] = AL::ALValue::array(-0.34971, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[14][2] = 3.8;
    keys[14][2] = AL::ALValue::array(-0.34971, AL::ALValue::array(3, -0.4, -1.33158e-07), AL::ALValue::array(3, 0.8, 2.66316e-07));
    times[14][3] = 6.2;
    keys[14][3] = AL::ALValue::array(0.0874801, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnkleRoll");
    times[15].arraySetSize(4);
    keys[15].arraySetSize(4);

    times[15][0] = 1.6;
    keys[15][0] = AL::ALValue::array(0.122762, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[15][1] = 2.6;
    keys[15][1] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[15][2] = 3.8;
    keys[15][2] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.4, -5.68913e-11), AL::ALValue::array(3, 0.8, 1.13783e-10));
    times[15][3] = 6.2;
    keys[15][3] = AL::ALValue::array(0.128898, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[16].arraySetSize(6);
    keys[16].arraySetSize(6);

    times[16][0] = 1.6;
    keys[16][0] = AL::ALValue::array(0.420358, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[16][1] = 2.6;
    keys[16][1] = AL::ALValue::array(0.624828, AL::ALValue::array(3, -0.333333, -0.20447), AL::ALValue::array(3, 0.2, 0.122682));
    times[16][2] = 3.2;
    keys[16][2] = AL::ALValue::array(1.54462, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[16][3] = 3.8;
    keys[16][3] = AL::ALValue::array(1.54462, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[16][4] = 4.4;
    keys[16][4] = AL::ALValue::array(0.486947, AL::ALValue::array(3, -0.2, 0.030889), AL::ALValue::array(3, 0.6, -0.0926669));
    times[16][5] = 6.2;
    keys[16][5] = AL::ALValue::array(0.39428, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[17].arraySetSize(4);
    keys[17].arraySetSize(4);

    times[17][0] = 1.6;
    keys[17][0] = AL::ALValue::array(1.21028, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[17][1] = 2.6;
    keys[17][1] = AL::ALValue::array(1.28456, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[17][2] = 3.8;
    keys[17][2] = AL::ALValue::array(1.28456, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[17][3] = 6.2;
    keys[17][3] = AL::ALValue::array(1.20568, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[18].arraySetSize(6);
    keys[18].arraySetSize(6);

    times[18][0] = 1.6;
    keys[18][0] = AL::ALValue::array(0.29, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[18][1] = 2.6;
    keys[18][1] = AL::ALValue::array(0.99, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.2, 0));
    times[18][2] = 3.2;
    keys[18][2] = AL::ALValue::array(0, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[18][3] = 3.8;
    keys[18][3] = AL::ALValue::array(0, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[18][4] = 4.4;
    keys[18][4] = AL::ALValue::array(1, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.6, 0));
    times[18][5] = 6.2;
    keys[18][5] = AL::ALValue::array(0.9864, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipPitch");
    times[19].arraySetSize(4);
    keys[19].arraySetSize(4);

    times[19][0] = 1.6;
    keys[19][0] = AL::ALValue::array(0.136484, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[19][1] = 2.6;
    keys[19][1] = AL::ALValue::array(-0.449504, AL::ALValue::array(3, -0.333333, 3.32895e-07), AL::ALValue::array(3, 0.4, -3.99474e-07));
    times[19][2] = 3.8;
    keys[19][2] = AL::ALValue::array(-0.449504, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[19][3] = 6.2;
    keys[19][3] = AL::ALValue::array(0.136484, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipRoll");
    times[20].arraySetSize(4);
    keys[20].arraySetSize(4);

    times[20][0] = 1.6;
    keys[20][0] = AL::ALValue::array(-0.098134, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[20][1] = 2.6;
    keys[20][1] = AL::ALValue::array(-0.00609398, AL::ALValue::array(3, -0.333333, -3.90112e-09), AL::ALValue::array(3, 0.4, 4.68134e-09));
    times[20][2] = 3.8;
    keys[20][2] = AL::ALValue::array(-0.00609397, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[20][3] = 6.2;
    keys[20][3] = AL::ALValue::array(-0.0966001, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipYawPitch");
    times[21].arraySetSize(4);
    keys[21].arraySetSize(4);

    times[21][0] = 1.6;
    keys[21][0] = AL::ALValue::array(-0.164096, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[21][1] = 2.6;
    keys[21][1] = AL::ALValue::array(-0.00149202, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[21][2] = 3.8;
    keys[21][2] = AL::ALValue::array(-0.00149202, AL::ALValue::array(3, -0.4, 9.1026e-10), AL::ALValue::array(3, 0.8, -1.82052e-09));
    times[21][3] = 6.2;
    keys[21][3] = AL::ALValue::array(-0.170232, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RKneePitch");
    times[22].arraySetSize(4);
    keys[22].arraySetSize(4);

    times[22][0] = 1.6;
    keys[22][0] = AL::ALValue::array(-0.082794, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[22][1] = 2.6;
    keys[22][1] = AL::ALValue::array(0.699546, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.4, 0));
    times[22][2] = 3.8;
    keys[22][2] = AL::ALValue::array(0.699545, AL::ALValue::array(3, -0.4, 3.99474e-07), AL::ALValue::array(3, 0.8, -7.98948e-07));
    times[22][3] = 6.2;
    keys[22][3] = AL::ALValue::array(-0.0873961, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[23].arraySetSize(6);
    keys[23].arraySetSize(6);

    times[23][0] = 1.6;
    keys[23][0] = AL::ALValue::array(1.46655, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[23][1] = 2.6;
    keys[23][1] = AL::ALValue::array(-0.998328, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.2, 0));
    times[23][2] = 3.2;
    keys[23][2] = AL::ALValue::array(0.211185, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[23][3] = 3.8;
    keys[23][3] = AL::ALValue::array(0.181514, AL::ALValue::array(3, -0.2, 0.0296706), AL::ALValue::array(3, 0.2, -0.0296706));
    times[23][4] = 4.4;
    keys[23][4] = AL::ALValue::array(-0.973894, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.6, 0));
    times[23][5] = 6.2;
    keys[23][5] = AL::ALValue::array(1.46961, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[24].arraySetSize(6);
    keys[24].arraySetSize(6);

    times[24][0] = 1.6;
    keys[24][0] = AL::ALValue::array(-0.17185, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[24][1] = 2.6;
    keys[24][1] = AL::ALValue::array(-0.0575959, AL::ALValue::array(3, -0.333333, -3.46766e-09), AL::ALValue::array(3, 0.4, 4.16119e-09));
    times[24][2] = 3.8;
    keys[24][2] = AL::ALValue::array(-0.0575959, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[24][3] = 4.4;
    keys[24][3] = AL::ALValue::array(-0.136136, AL::ALValue::array(3, -0.2, 0.0579699), AL::ALValue::array(3, 0.173333, -0.0502406));
    times[24][4] = 4.92;
    keys[24][4] = AL::ALValue::array(-0.382227, AL::ALValue::array(3, -0.173333, 0), AL::ALValue::array(3, 0.426667, 0));
    times[24][5] = 6.2;
    keys[24][5] = AL::ALValue::array(-0.170316, AL::ALValue::array(3, -0.426667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[25].arraySetSize(4);
    keys[25].arraySetSize(4);

    times[25][0] = 1.6;
    keys[25][0] = AL::ALValue::array(0.079726, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.333333, 0));
    times[25][1] = 2.6;
    keys[25][1] = AL::ALValue::array(0.129154, AL::ALValue::array(3, -0.333333, -6.93532e-09), AL::ALValue::array(3, 0.4, 8.32238e-09));
    times[25][2] = 3.8;
    keys[25][2] = AL::ALValue::array(0.129154, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.8, 0));
    times[25][3] = 6.2;
    keys[25][3] = AL::ALValue::array(0.079726, AL::ALValue::array(3, -0.8, 0), AL::ALValue::array(3, 0, 0));

    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
    posture.goToPosture("Stand", 0.5);


}

void GuessTheEmotion::happy()
{
    //Set eye color
    eyes.fadeRGB("Eyes", "yellow", 0.5);
    // Custom motion exported from Choregraphe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(25);
    times.arraySetSize(25);
    keys.arraySetSize(25);

    names.push_back("HeadPitch");
    times[0].arraySetSize(5);
    keys[0].arraySetSize(5);

    times[0][0] = 0.933333;
    keys[0][0] = AL::ALValue::array(-0.481718, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[0][1] = 1.53333;
    keys[0][1] = AL::ALValue::array(-0.116626, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[0][2] = 2.13333;
    keys[0][2] = AL::ALValue::array(-0.460242, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.222222, 0));
    times[0][3] = 2.8;
    keys[0][3] = AL::ALValue::array(0.01223, AL::ALValue::array(3, -0.222222, 0), AL::ALValue::array(3, 0.888889, 0));
    times[0][4] = 5.46667;
    keys[0][4] = AL::ALValue::array(0.010696, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(5);
    keys[1].arraySetSize(5);

    times[1][0] = 0.933333;
    keys[1][0] = AL::ALValue::array(0.608956, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[1][1] = 1.53333;
    keys[1][1] = AL::ALValue::array(-0.00464395, AL::ALValue::array(3, -0.2, 0.229333), AL::ALValue::array(3, 0.2, -0.229333));
    times[1][2] = 2.13333;
    keys[1][2] = AL::ALValue::array(-0.767043, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.222222, 0));
    times[1][3] = 2.8;
    keys[1][3] = AL::ALValue::array(0.00149204, AL::ALValue::array(3, -0.222222, 0), AL::ALValue::array(3, 0.888889, 0));
    times[1][4] = 5.46667;
    keys[1][4] = AL::ALValue::array(0.00149204, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnklePitch");
    times[2].arraySetSize(4);
    keys[2].arraySetSize(4);

    times[2][0] = 0.933333;
    keys[2][0] = AL::ALValue::array(-0.343659, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[2][1] = 1.53333;
    keys[2][1] = AL::ALValue::array(-0.346725, AL::ALValue::array(3, -0.2, 0.00306653), AL::ALValue::array(3, 0.2, -0.00306653));
    times[2][2] = 2.13333;
    keys[2][2] = AL::ALValue::array(-0.705598, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[2][3] = 5.46667;
    keys[2][3] = AL::ALValue::array(-0.346725, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnkleRoll");
    times[3].arraySetSize(4);
    keys[3].arraySetSize(4);

    times[3][0] = 0.933333;
    keys[3][0] = AL::ALValue::array(-0.42641, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[3][1] = 1.53333;
    keys[3][1] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.2, -0.0925374), AL::ALValue::array(3, 0.2, 0.0925374));
    times[3][2] = 2.13333;
    keys[3][2] = AL::ALValue::array(0.128814, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[3][3] = 5.46667;
    keys[3][3] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[4].arraySetSize(5);
    keys[4].arraySetSize(5);

    times[4][0] = 1.2;
    keys[4][0] = AL::ALValue::array(-0.225931, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][1] = 2;
    keys[4][1] = AL::ALValue::array(-1.43332, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][2] = 2.8;
    keys[4][2] = AL::ALValue::array(-1.06149, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.644444, 0));
    times[4][3] = 4.73333;
    keys[4][3] = AL::ALValue::array(-1.07376, AL::ALValue::array(3, -0.644444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[4][4] = 5.46667;
    keys[4][4] = AL::ALValue::array(-1.04921, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[5].arraySetSize(6);
    keys[5].arraySetSize(6);

    times[5][0] = 1.2;
    keys[5][0] = AL::ALValue::array(-1.02119, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.266667, 0));
    times[5][1] = 2;
    keys[5][1] = AL::ALValue::array(-0.922413, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[5][2] = 2.8;
    keys[5][2] = AL::ALValue::array(-1.25946, AL::ALValue::array(3, -0.266667, 0.0958968), AL::ALValue::array(3, 0.4, -0.143845));
    times[5][3] = 4;
    keys[5][3] = AL::ALValue::array(-1.64164, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.244444, 0));
    times[5][4] = 4.73333;
    keys[5][4] = AL::ALValue::array(-1.27326, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[5][5] = 5.46667;
    keys[5][5] = AL::ALValue::array(-1.38218, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[6].arraySetSize(5);
    keys[6].arraySetSize(5);

    times[6][0] = 1.2;
    keys[6][0] = AL::ALValue::array(0.000545389, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.111111, 0));
    times[6][1] = 1.53333;
    keys[6][1] = AL::ALValue::array(0.0120264, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.155556, 0));
    times[6][2] = 2;
    keys[6][2] = AL::ALValue::array(0, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.266667, 0));
    times[6][3] = 2.8;
    keys[6][3] = AL::ALValue::array(0.0120264, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.888889, 0));
    times[6][4] = 5.46667;
    keys[6][4] = AL::ALValue::array(0.0112992, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipPitch");
    times[7].arraySetSize(4);
    keys[7].arraySetSize(4);

    times[7][0] = 0.933333;
    keys[7][0] = AL::ALValue::array(-0.432547, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[7][1] = 1.53333;
    keys[7][1] = AL::ALValue::array(-0.434081, AL::ALValue::array(3, -0.2, 0.00153415), AL::ALValue::array(3, 0.2, -0.00153415));
    times[7][2] = 2.13333;
    keys[7][2] = AL::ALValue::array(-0.642787, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[7][3] = 5.46667;
    keys[7][3] = AL::ALValue::array(-0.434081, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipRoll");
    times[8].arraySetSize(4);
    keys[8].arraySetSize(4);

    times[8][0] = 0.933333;
    keys[8][0] = AL::ALValue::array(0.363599, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[8][1] = 1.53333;
    keys[8][1] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -0.2, 0.0384339), AL::ALValue::array(3, 0.2, -0.0384339));
    times[8][2] = 2.13333;
    keys[8][2] = AL::ALValue::array(-0.039926, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[8][3] = 5.46667;
    keys[8][3] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipYawPitch");
    times[9].arraySetSize(4);
    keys[9].arraySetSize(4);

    times[9][0] = 0.933333;
    keys[9][0] = AL::ALValue::array(-0.29602, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[9][1] = 1.53333;
    keys[9][1] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[9][2] = 2.13333;
    keys[9][2] = AL::ALValue::array(-0.29602, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[9][3] = 5.46667;
    keys[9][3] = AL::ALValue::array(0.00157596, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LKneePitch");
    times[10].arraySetSize(4);
    keys[10].arraySetSize(4);

    times[10][0] = 0.933333;
    keys[10][0] = AL::ALValue::array(0.954107, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[10][1] = 1.53333;
    keys[10][1] = AL::ALValue::array(0.697927, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[10][2] = 2.13333;
    keys[10][2] = AL::ALValue::array(1.43893, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[10][3] = 5.46667;
    keys[10][3] = AL::ALValue::array(0.700996, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[11].arraySetSize(6);
    keys[11].arraySetSize(6);

    times[11][0] = 1.2;
    keys[11][0] = AL::ALValue::array(-0.8918, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][1] = 2;
    keys[11][1] = AL::ALValue::array(1.0038, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][2] = 2.8;
    keys[11][2] = AL::ALValue::array(-0.435699, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.2, 0));
    times[11][3] = 3.4;
    keys[11][3] = AL::ALValue::array(0.153358, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.444444, 0));
    times[11][4] = 4.73333;
    keys[11][4] = AL::ALValue::array(-0.47865, AL::ALValue::array(3, -0.444444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[11][5] = 5.46667;
    keys[11][5] = AL::ALValue::array(1.42198, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[12].arraySetSize(5);
    keys[12].arraySetSize(5);

    times[12][0] = 1.2;
    keys[12][0] = AL::ALValue::array(1.08242, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][1] = 2;
    keys[12][1] = AL::ALValue::array(0.136047, AL::ALValue::array(3, -0.266667, 0.100807), AL::ALValue::array(3, 0.266667, -0.100807));
    times[12][2] = 2.8;
    keys[12][2] = AL::ALValue::array(0.0352401, AL::ALValue::array(3, -0.266667, 0.00507811), AL::ALValue::array(3, 0.644444, -0.0122721));
    times[12][3] = 4.73333;
    keys[12][3] = AL::ALValue::array(0.022968, AL::ALValue::array(3, -0.644444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[12][4] = 5.46667;
    keys[12][4] = AL::ALValue::array(0.322099, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[13].arraySetSize(5);
    keys[13].arraySetSize(5);

    times[13][0] = 1.2;
    keys[13][0] = AL::ALValue::array(-0.0189538, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.111111, 0));
    times[13][1] = 1.53333;
    keys[13][1] = AL::ALValue::array(0.00456004, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.155556, 0));
    times[13][2] = 2;
    keys[13][2] = AL::ALValue::array(-0.0188867, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.266667, 0));
    times[13][3] = 2.8;
    keys[13][3] = AL::ALValue::array(0.00456004, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.888889, 0));
    times[13][4] = 5.46667;
    keys[13][4] = AL::ALValue::array(-0.00310997, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnklePitch");
    times[14].arraySetSize(4);
    keys[14].arraySetSize(4);

    times[14][0] = 0.933333;
    keys[14][0] = AL::ALValue::array(-0.705598, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[14][1] = 1.53333;
    keys[14][1] = AL::ALValue::array(-0.351244, AL::ALValue::array(3, -0.2, -0.00758518), AL::ALValue::array(3, 0.2, 0.00758518));
    times[14][2] = 2.13333;
    keys[14][2] = AL::ALValue::array(-0.343659, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[14][3] = 5.46667;
    keys[14][3] = AL::ALValue::array(-0.351244, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnkleRoll");
    times[15].arraySetSize(4);
    keys[15].arraySetSize(4);

    times[15][0] = 0.933333;
    keys[15][0] = AL::ALValue::array(-0.128814, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[15][1] = 1.53333;
    keys[15][1] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -0.2, -0.0925374), AL::ALValue::array(3, 0.2, 0.0925374));
    times[15][2] = 2.13333;
    keys[15][2] = AL::ALValue::array(0.42641, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[15][3] = 5.46667;
    keys[15][3] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[16].arraySetSize(6);
    keys[16].arraySetSize(6);

    times[16][0] = 0.8;
    keys[16][0] = AL::ALValue::array(1.43332, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.511111, 0));
    times[16][1] = 2.33333;
    keys[16][1] = AL::ALValue::array(0.225931, AL::ALValue::array(3, -0.511111, 0), AL::ALValue::array(3, 0.155556, 0));
    times[16][2] = 2.8;
    keys[16][2] = AL::ALValue::array(0.975665, AL::ALValue::array(3, -0.155556, -0.108346), AL::ALValue::array(3, 0.4, 0.278603));
    times[16][3] = 4;
    keys[16][3] = AL::ALValue::array(1.38678, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.244444, 0));
    times[16][4] = 4.73333;
    keys[16][4] = AL::ALValue::array(0.966462, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[16][5] = 5.46667;
    keys[16][5] = AL::ALValue::array(1.03703, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[17].arraySetSize(6);
    keys[17].arraySetSize(6);

    times[17][0] = 0.8;
    keys[17][0] = AL::ALValue::array(0.922413, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.511111, 0));
    times[17][1] = 2.33333;
    keys[17][1] = AL::ALValue::array(1.02119, AL::ALValue::array(3, -0.511111, -0.0987804), AL::ALValue::array(3, 0.155556, 0.0300636));
    times[17][2] = 2.8;
    keys[17][2] = AL::ALValue::array(1.51152, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.4, 0));
    times[17][3] = 4;
    keys[17][3] = AL::ALValue::array(1.03588, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.244444, 0));
    times[17][4] = 4.73333;
    keys[17][4] = AL::ALValue::array(1.73338, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[17][5] = 5.46667;
    keys[17][5] = AL::ALValue::array(1.38516, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[18].arraySetSize(5);
    keys[18].arraySetSize(5);

    times[18][0] = 0.8;
    keys[18][0] = AL::ALValue::array(0, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.244444, 0));
    times[18][1] = 1.53333;
    keys[18][1] = AL::ALValue::array(0.0102083, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0.266667, 0));
    times[18][2] = 2.33333;
    keys[18][2] = AL::ALValue::array(0.000545389, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.155556, 0));
    times[18][3] = 2.8;
    keys[18][3] = AL::ALValue::array(0.0102083, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.888889, 0));
    times[18][4] = 5.46667;
    keys[18][4] = AL::ALValue::array(0.0102083, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipPitch");
    times[19].arraySetSize(4);
    keys[19].arraySetSize(4);

    times[19][0] = 0.933333;
    keys[19][0] = AL::ALValue::array(-0.642787, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[19][1] = 1.53333;
    keys[19][1] = AL::ALValue::array(-0.437231, AL::ALValue::array(3, -0.2, -0.00468447), AL::ALValue::array(3, 0.2, 0.00468447));
    times[19][2] = 2.13333;
    keys[19][2] = AL::ALValue::array(-0.432547, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[19][3] = 5.46667;
    keys[19][3] = AL::ALValue::array(-0.437231, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipRoll");
    times[20].arraySetSize(4);
    keys[20].arraySetSize(4);

    times[20][0] = 0.933333;
    keys[20][0] = AL::ALValue::array(0.039926, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[20][1] = 1.53333;
    keys[20][1] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.2, 0.039884), AL::ALValue::array(3, 0.2, -0.039884));
    times[20][2] = 2.13333;
    keys[20][2] = AL::ALValue::array(-0.363599, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[20][3] = 5.46667;
    keys[20][3] = AL::ALValue::array(-0.00149204, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RKneePitch");
    times[21].arraySetSize(4);
    keys[21].arraySetSize(4);

    times[21][0] = 0.933333;
    keys[21][0] = AL::ALValue::array(1.43893, AL::ALValue::array(3, -0.311111, 0), AL::ALValue::array(3, 0.2, 0));
    times[21][1] = 1.53333;
    keys[21][1] = AL::ALValue::array(0.696479, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[21][2] = 2.13333;
    keys[21][2] = AL::ALValue::array(0.954107, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 1.11111, 0));
    times[21][3] = 5.46667;
    keys[21][3] = AL::ALValue::array(0.693411, AL::ALValue::array(3, -1.11111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[22].arraySetSize(6);
    keys[22].arraySetSize(6);

    times[22][0] = 0.8;
    keys[22][0] = AL::ALValue::array(1.0038, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.511111, 0));
    times[22][1] = 2.33333;
    keys[22][1] = AL::ALValue::array(-0.8918, AL::ALValue::array(3, -0.511111, 0), AL::ALValue::array(3, 0.155556, 0));
    times[22][2] = 2.8;
    keys[22][2] = AL::ALValue::array(-0.424876, AL::ALValue::array(3, -0.155556, -0.146838), AL::ALValue::array(3, 0.2, 0.188792));
    times[22][3] = 3.4;
    keys[22][3] = AL::ALValue::array(0.115092, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.444444, 0));
    times[22][4] = 4.73333;
    keys[22][4] = AL::ALValue::array(-0.467829, AL::ALValue::array(3, -0.444444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[22][5] = 5.46667;
    keys[22][5] = AL::ALValue::array(1.4328, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[23].arraySetSize(5);
    keys[23].arraySetSize(5);

    times[23][0] = 0.8;
    keys[23][0] = AL::ALValue::array(-0.136047, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.511111, 0));
    times[23][1] = 2.33333;
    keys[23][1] = AL::ALValue::array(-1.08242, AL::ALValue::array(3, -0.511111, 0), AL::ALValue::array(3, 0.155556, 0));
    times[23][2] = 2.8;
    keys[23][2] = AL::ALValue::array(-0.308375, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.644444, 0));
    times[23][3] = 4.73333;
    keys[23][3] = AL::ALValue::array(-0.337522, AL::ALValue::array(3, -0.644444, 0), AL::ALValue::array(3, 0.244444, 0));
    times[23][4] = 5.46667;
    keys[23][4] = AL::ALValue::array(-0.308375, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[24].arraySetSize(5);
    keys[24].arraySetSize(5);

    times[24][0] = 0.8;
    keys[24][0] = AL::ALValue::array(0.0188867, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.244444, 0));
    times[24][1] = 1.53333;
    keys[24][1] = AL::ALValue::array(0.0291041, AL::ALValue::array(3, -0.244444, 0), AL::ALValue::array(3, 0.266667, 0));
    times[24][2] = 2.33333;
    keys[24][2] = AL::ALValue::array(0.0189538, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.155556, 0));
    times[24][3] = 2.8;
    keys[24][3] = AL::ALValue::array(0.0291041, AL::ALValue::array(3, -0.155556, 0), AL::ALValue::array(3, 0.888889, 0));
    times[24][4] = 5.46667;
    keys[24][4] = AL::ALValue::array(0.024502, AL::ALValue::array(3, -0.888889, 0), AL::ALValue::array(3, 0, 0));

    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
    posture.goToPosture("Stand", 0.5);

}

void GuessTheEmotion::sad()
{
    //Set eye color to blue
    eyes.fadeRGB("Eyes", "blue", 0.5);

    //Bezier custom motion imported from Choregraphe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(26);
    times.arraySetSize(26);
    keys.arraySetSize(26);

    names.push_back("HeadPitch");
    times[0].arraySetSize(5);
    keys[0].arraySetSize(5);

    times[0][0] = 1.4;
    keys[0][0] = AL::ALValue::array(0.13495, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[0][1] = 3;
    keys[0][1] = AL::ALValue::array(0.415672, AL::ALValue::array(3, -0.533333, -1.02088e-06), AL::ALValue::array(3, 0.4, 7.65659e-07));
    times[0][2] = 4.2;
    keys[0][2] = AL::ALValue::array(0.415673, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[0][3] = 5.4;
    keys[0][3] = AL::ALValue::array(0.415673, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[0][4] = 6.8;
    keys[0][4] = AL::ALValue::array(0.386526, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(5);
    keys[1].arraySetSize(5);

    times[1][0] = 1.4;
    keys[1][0] = AL::ALValue::array(-0.021518, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[1][1] = 3;
    keys[1][1] = AL::ALValue::array(-0.512398, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[1][2] = 4.2;
    keys[1][2] = AL::ALValue::array(-0.512397, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[1][3] = 5.4;
    keys[1][3] = AL::ALValue::array(-0.512397, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[1][4] = 6.8;
    keys[1][4] = AL::ALValue::array(-0.0383921, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnklePitch");
    times[2].arraySetSize(5);
    keys[2].arraySetSize(5);

    times[2][0] = 1.4;
    keys[2][0] = AL::ALValue::array(-1.18944, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[2][1] = 3;
    keys[2][1] = AL::ALValue::array(-1.18889, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[2][2] = 4.2;
    keys[2][2] = AL::ALValue::array(-1.18889, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[2][3] = 5.4;
    keys[2][3] = AL::ALValue::array(-1.18889, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[2][4] = 6.8;
    keys[2][4] = AL::ALValue::array(-1.18889, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnkleRoll");
    times[3].arraySetSize(5);
    keys[3].arraySetSize(5);

    times[3][0] = 1.4;
    keys[3][0] = AL::ALValue::array(0.0750158, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[3][1] = 3;
    keys[3][1] = AL::ALValue::array(0.0767419, AL::ALValue::array(3, -0.533333, -2.2193e-08), AL::ALValue::array(3, 0.4, 1.66448e-08));
    times[3][2] = 4.2;
    keys[3][2] = AL::ALValue::array(0.0767419, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[3][3] = 5.4;
    keys[3][3] = AL::ALValue::array(0.0767419, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[3][4] = 6.8;
    keys[3][4] = AL::ALValue::array(0.0767419, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[4].arraySetSize(5);
    keys[4].arraySetSize(5);

    times[4][0] = 1.4;
    keys[4][0] = AL::ALValue::array(-1.04461, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[4][1] = 3;
    keys[4][1] = AL::ALValue::array(-0.97865, AL::ALValue::array(3, -0.533333, -5.32632e-07), AL::ALValue::array(3, 0.4, 3.99474e-07));
    times[4][2] = 4.2;
    keys[4][2] = AL::ALValue::array(-0.97865, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[4][3] = 5.4;
    keys[4][3] = AL::ALValue::array(-0.97865, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[4][4] = 6.8;
    keys[4][4] = AL::ALValue::array(-1.03081, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[5].arraySetSize(5);
    keys[5].arraySetSize(5);

    times[5][0] = 1.4;
    keys[5][0] = AL::ALValue::array(-0.81613, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[5][1] = 3;
    keys[5][1] = AL::ALValue::array(-0.875956, AL::ALValue::array(3, -0.533333, 2.66316e-07), AL::ALValue::array(3, 0.4, -1.99737e-07));
    times[5][2] = 4.2;
    keys[5][2] = AL::ALValue::array(-0.875956, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[5][3] = 5.4;
    keys[5][3] = AL::ALValue::array(-0.875956, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[5][4] = 6.8;
    keys[5][4] = AL::ALValue::array(-0.865218, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[6].arraySetSize(5);
    keys[6].arraySetSize(5);

    times[6][0] = 1.4;
    keys[6][0] = AL::ALValue::array(0.0184, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[6][1] = 3;
    keys[6][1] = AL::ALValue::array(0.0316, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[6][2] = 4.2;
    keys[6][2] = AL::ALValue::array(0.0316, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[6][3] = 5.4;
    keys[6][3] = AL::ALValue::array(0.0316, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[6][4] = 6.8;
    keys[6][4] = AL::ALValue::array(0.0316, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipPitch");
    times[7].arraySetSize(5);
    keys[7].arraySetSize(5);

    times[7][0] = 1.4;
    keys[7][0] = AL::ALValue::array(-0.707132, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[7][1] = 3;
    keys[7][1] = AL::ALValue::array(-0.676452, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[7][2] = 4.2;
    keys[7][2] = AL::ALValue::array(-0.676453, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[7][3] = 5.4;
    keys[7][3] = AL::ALValue::array(-0.676453, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[7][4] = 6.8;
    keys[7][4] = AL::ALValue::array(-0.707132, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipRoll");
    times[8].arraySetSize(5);
    keys[8].arraySetSize(5);

    times[8][0] = 1.4;
    keys[8][0] = AL::ALValue::array(-0.08126, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[8][1] = 3;
    keys[8][1] = AL::ALValue::array(-0.07359, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[8][2] = 4.2;
    keys[8][2] = AL::ALValue::array(-0.0735901, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[8][3] = 5.4;
    keys[8][3] = AL::ALValue::array(-0.0735901, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[8][4] = 6.8;
    keys[8][4] = AL::ALValue::array(-0.07359, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipYawPitch");
    times[9].arraySetSize(5);
    keys[9].arraySetSize(5);

    times[9][0] = 1.4;
    keys[9][0] = AL::ALValue::array(-0.246932, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[9][1] = 3;
    keys[9][1] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.533333, -7.54562e-07), AL::ALValue::array(3, 0.4, 5.65922e-07));
    times[9][2] = 4.2;
    keys[9][2] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[9][3] = 5.4;
    keys[9][3] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[9][4] = 6.8;
    keys[9][4] = AL::ALValue::array(-0.243864, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LKneePitch");
    times[10].arraySetSize(5);
    keys[10].arraySetSize(5);

    times[10][0] = 1.4;
    keys[10][0] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[10][1] = 3;
    keys[10][1] = AL::ALValue::array(2.10921, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[10][2] = 4.2;
    keys[10][2] = AL::ALValue::array(2.10921, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[10][3] = 5.4;
    keys[10][3] = AL::ALValue::array(2.10921, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[10][4] = 6.8;
    keys[10][4] = AL::ALValue::array(2.10921, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[11].arraySetSize(5);
    keys[11].arraySetSize(5);

    times[11][0] = 1.4;
    keys[11][0] = AL::ALValue::array(1.38363, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[11][1] = 3;
    keys[11][1] = AL::ALValue::array(1.23636, AL::ALValue::array(3, -0.533333, 7.10176e-07), AL::ALValue::array(3, 0.4, -5.32632e-07));
    times[11][2] = 4.2;
    keys[11][2] = AL::ALValue::array(1.23636, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[11][3] = 5.4;
    keys[11][3] = AL::ALValue::array(1.23636, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[11][4] = 6.8;
    keys[11][4] = AL::ALValue::array(1.34374, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[12].arraySetSize(5);
    keys[12].arraySetSize(5);

    times[12][0] = 1.4;
    keys[12][0] = AL::ALValue::array(0.16563, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[12][1] = 3;
    keys[12][1] = AL::ALValue::array(-0.019984, AL::ALValue::array(3, -0.533333, 1.66448e-08), AL::ALValue::array(3, 0.4, -1.24836e-08));
    times[12][2] = 4.2;
    keys[12][2] = AL::ALValue::array(-0.019984, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[12][3] = 5.4;
    keys[12][3] = AL::ALValue::array(-0.019984, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[12][4] = 6.8;
    keys[12][4] = AL::ALValue::array(0.18097, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[13].arraySetSize(5);
    keys[13].arraySetSize(5);

    times[13][0] = 1.4;
    keys[13][0] = AL::ALValue::array(0.128814, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[13][1] = 3;
    keys[13][1] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[13][2] = 4.2;
    keys[13][2] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[13][3] = 5.4;
    keys[13][3] = AL::ALValue::array(-0.147306, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[13][4] = 6.8;
    keys[13][4] = AL::ALValue::array(0.115008, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnklePitch");
    times[14].arraySetSize(5);
    keys[14].arraySetSize(5);

    times[14][0] = 1.4;
    keys[14][0] = AL::ALValue::array(-1.1863, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[14][1] = 3;
    keys[14][1] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.533333, -1.77544e-07), AL::ALValue::array(3, 0.4, 1.33158e-07));
    times[14][2] = 4.2;
    keys[14][2] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[14][3] = 5.4;
    keys[14][3] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[14][4] = 6.8;
    keys[14][4] = AL::ALValue::array(-1.18421, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnkleRoll");
    times[15].arraySetSize(5);
    keys[15].arraySetSize(5);

    times[15][0] = 1.4;
    keys[15][0] = AL::ALValue::array(-0.076022, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[15][1] = 3;
    keys[15][1] = AL::ALValue::array(-0.0689881, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[15][2] = 4.2;
    keys[15][2] = AL::ALValue::array(-0.0689882, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[15][3] = 5.4;
    keys[15][3] = AL::ALValue::array(-0.0689882, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[15][4] = 6.8;
    keys[15][4] = AL::ALValue::array(-0.0689881, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[16].arraySetSize(5);
    keys[16].arraySetSize(5);

    times[16][0] = 1.4;
    keys[16][0] = AL::ALValue::array(1.04316, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[16][1] = 3;
    keys[16][1] = AL::ALValue::array(1.31468, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[16][2] = 4.2;
    keys[16][2] = AL::ALValue::array(1.31468, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[16][3] = 5.4;
    keys[16][3] = AL::ALValue::array(1.31468, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[16][4] = 6.8;
    keys[16][4] = AL::ALValue::array(1.04163, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[17].arraySetSize(5);
    keys[17].arraySetSize(5);

    times[17][0] = 1.4;
    keys[17][0] = AL::ALValue::array(0.762356, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[17][1] = 3;
    keys[17][1] = AL::ALValue::array(0.361982, AL::ALValue::array(3, -0.533333, 1.10965e-06), AL::ALValue::array(3, 0.4, -8.32238e-07));
    times[17][2] = 4.2;
    keys[17][2] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[17][3] = 5.4;
    keys[17][3] = AL::ALValue::array(0.361981, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[17][4] = 6.8;
    keys[17][4] = AL::ALValue::array(0.766958, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[18].arraySetSize(5);
    keys[18].arraySetSize(5);

    times[18][0] = 1.4;
    keys[18][0] = AL::ALValue::array(0.0216, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[18][1] = 3;
    keys[18][1] = AL::ALValue::array(0.0391999, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[18][2] = 4.2;
    keys[18][2] = AL::ALValue::array(0.0391999, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[18][3] = 5.4;
    keys[18][3] = AL::ALValue::array(0.0391999, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[18][4] = 6.8;
    keys[18][4] = AL::ALValue::array(0.0391999, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipPitch");
    times[19].arraySetSize(5);
    keys[19].arraySetSize(5);

    times[19][0] = 1.4;
    keys[19][0] = AL::ALValue::array(-0.70875, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[19][1] = 3;
    keys[19][1] = AL::ALValue::array(-0.682672, AL::ALValue::array(3, -0.533333, -6.21404e-07), AL::ALValue::array(3, 0.4, 4.66053e-07));
    times[19][2] = 4.2;
    keys[19][2] = AL::ALValue::array(-0.682672, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[19][3] = 5.4;
    keys[19][3] = AL::ALValue::array(-0.682672, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[19][4] = 6.8;
    keys[19][4] = AL::ALValue::array(-0.70108, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipRoll");
    times[20].arraySetSize(5);
    keys[20].arraySetSize(5);

    times[20][0] = 1.4;
    keys[20][0] = AL::ALValue::array(0.0752079, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[20][1] = 3;
    keys[20][1] = AL::ALValue::array(0.0644701, AL::ALValue::array(3, -0.533333, 4.4386e-08), AL::ALValue::array(3, 0.4, -3.32895e-08));
    times[20][2] = 4.2;
    keys[20][2] = AL::ALValue::array(0.06447, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[20][3] = 5.4;
    keys[20][3] = AL::ALValue::array(0.06447, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[20][4] = 6.8;
    keys[20][4] = AL::ALValue::array(0.0752079, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipYawPitch");
    times[21].arraySetSize(5);
    keys[21].arraySetSize(5);

    times[21][0] = 1.4;
    keys[21][0] = AL::ALValue::array(-0.246932, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[21][1] = 3;
    keys[21][1] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.533333, -7.54562e-07), AL::ALValue::array(3, 0.4, 5.65922e-07));
    times[21][2] = 4.2;
    keys[21][2] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[21][3] = 5.4;
    keys[21][3] = AL::ALValue::array(-0.216252, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[21][4] = 6.8;
    keys[21][4] = AL::ALValue::array(-0.243864, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RKneePitch");
    times[22].arraySetSize(5);
    keys[22].arraySetSize(5);

    times[22][0] = 1.4;
    keys[22][0] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[22][1] = 3;
    keys[22][1] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[22][2] = 4.2;
    keys[22][2] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[22][3] = 5.4;
    keys[22][3] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[22][4] = 6.8;
    keys[22][4] = AL::ALValue::array(2.11255, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[23].arraySetSize(5);
    keys[23].arraySetSize(5);

    times[23][0] = 1.4;
    keys[23][0] = AL::ALValue::array(1.38371, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[23][1] = 3;
    keys[23][1] = AL::ALValue::array(-0.185572, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[23][2] = 4.2;
    keys[23][2] = AL::ALValue::array(-0.185572, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[23][3] = 5.4;
    keys[23][3] = AL::ALValue::array(-0.185572, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[23][4] = 6.8;
    keys[23][4] = AL::ALValue::array(1.33616, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[24].arraySetSize(5);
    keys[24].arraySetSize(5);

    times[24][0] = 1.4;
    keys[24][0] = AL::ALValue::array(-0.150374, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.533333, 0));
    times[24][1] = 3;
    keys[24][1] = AL::ALValue::array(0.049046, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[24][2] = 4.2;
    keys[24][2] = AL::ALValue::array(0.049046, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.4, 0));
    times[24][3] = 5.4;
    keys[24][3] = AL::ALValue::array(0.049046, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.466667, 0));
    times[24][4] = 6.8;
    keys[24][4] = AL::ALValue::array(-0.182588, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[25].arraySetSize(8);
    keys[25].arraySetSize(8);

    times[25][0] = 1.4;
    keys[25][0] = AL::ALValue::array(-0.116626, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0.333333, 0));
    times[25][1] = 2.4;
    keys[25][1] = AL::ALValue::array(1.67726, AL::ALValue::array(3, -0.333333, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][2] = 3;
    keys[25][2] = AL::ALValue::array(-0.129154, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][3] = 3.6;
    keys[25][3] = AL::ALValue::array(1.67726, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][4] = 4.2;
    keys[25][4] = AL::ALValue::array(-0.129154, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][5] = 4.8;
    keys[25][5] = AL::ALValue::array(1.67726, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][6] = 5.4;
    keys[25][6] = AL::ALValue::array(-0.129154, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0.466667, 0));
    times[25][7] = 6.8;
    keys[25][7] = AL::ALValue::array(-0.112024, AL::ALValue::array(3, -0.466667, 0), AL::ALValue::array(3, 0, 0));

    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
    posture.goToPosture("Stand", 0.5);

}

void GuessTheEmotion::scared()
{

    //Set eye color to magenta
    eyes.fadeRGB("Eyes", "magenta", 0.5);


    // Choregraphe bezier export in c++.
    // Add #include <alproxies/almotionproxy.h> at the beginning of this file.
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(25);
    times.arraySetSize(25);
    keys.arraySetSize(25);

    names.push_back("HeadPitch");
    times[0].arraySetSize(14);
    keys[0].arraySetSize(14);

    times[0][0] = 0.4;
    keys[0][0] = AL::ALValue::array(-0.0690719, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][1] = 0.8;
    keys[0][1] = AL::ALValue::array(-0.0690719, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[0][2] = 1.13333;
    keys[0][2] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][3] = 1.53333;
    keys[0][3] = AL::ALValue::array(-0.0690719, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][4] = 1.93333;
    keys[0][4] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[0][5] = 2.26667;
    keys[0][5] = AL::ALValue::array(0.033706, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.111111, 0));
    times[0][6] = 2.6;
    keys[0][6] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][7] = 3;
    keys[0][7] = AL::ALValue::array(-0.0629359, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][8] = 3.4;
    keys[0][8] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[0][9] = 3.73333;
    keys[0][9] = AL::ALValue::array(-0.0629359, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][10] = 4.13333;
    keys[0][10] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][11] = 4.53333;
    keys[0][11] = AL::ALValue::array(-0.0629359, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[0][12] = 4.93333;
    keys[0][12] = AL::ALValue::array(0.0444441, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[0][13] = 5.26667;
    keys[0][13] = AL::ALValue::array(-0.0629359, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(14);
    keys[1].arraySetSize(14);

    times[1][0] = 0.4;
    keys[1][0] = AL::ALValue::array(-0.421891, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][1] = 0.8;
    keys[1][1] = AL::ALValue::array(0.323633, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[1][2] = 1.13333;
    keys[1][2] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][3] = 1.53333;
    keys[1][3] = AL::ALValue::array(0.323633, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][4] = 1.93333;
    keys[1][4] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[1][5] = 2.26667;
    keys[1][5] = AL::ALValue::array(0.621227, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.111111, 0));
    times[1][6] = 2.6;
    keys[1][6] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][7] = 3;
    keys[1][7] = AL::ALValue::array(0.04291, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][8] = 3.4;
    keys[1][8] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[1][9] = 3.73333;
    keys[1][9] = AL::ALValue::array(0.04291, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][10] = 4.13333;
    keys[1][10] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][11] = 4.53333;
    keys[1][11] = AL::ALValue::array(0.04291, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[1][12] = 4.93333;
    keys[1][12] = AL::ALValue::array(-0.458707, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.111111, 0));
    times[1][13] = 5.26667;
    keys[1][13] = AL::ALValue::array(0.04291, AL::ALValue::array(3, -0.111111, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnklePitch");
    times[2].arraySetSize(1);
    keys[2].arraySetSize(1);

    times[2][0] = 1.46667;
    keys[2][0] = AL::ALValue::array(-0.403483, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnkleRoll");
    times[3].arraySetSize(1);
    keys[3].arraySetSize(1);

    times[3][0] = 1.46667;
    keys[3][0] = AL::ALValue::array(-0.23466, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[4].arraySetSize(1);
    keys[4].arraySetSize(1);

    times[4][0] = 1.46667;
    keys[4][0] = AL::ALValue::array(-1.13205, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[5].arraySetSize(1);
    keys[5].arraySetSize(1);

    times[5][0] = 1.46667;
    keys[5][0] = AL::ALValue::array(-0.466378, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[6].arraySetSize(1);
    keys[6].arraySetSize(1);

    times[6][0] = 1.46667;
    keys[6][0] = AL::ALValue::array(0.912024, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipPitch");
    times[7].arraySetSize(1);
    keys[7].arraySetSize(1);

    times[7][0] = 1.46667;
    keys[7][0] = AL::ALValue::array(0.48398, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipRoll");
    times[8].arraySetSize(1);
    keys[8].arraySetSize(1);

    times[8][0] = 1.46667;
    keys[8][0] = AL::ALValue::array(0.303775, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipYawPitch");
    times[9].arraySetSize(1);
    keys[9].arraySetSize(1);

    times[9][0] = 1.46667;
    keys[9][0] = AL::ALValue::array(-0.133416, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LKneePitch");
    times[10].arraySetSize(1);
    keys[10].arraySetSize(1);

    times[10][0] = 1.46667;
    keys[10][0] = AL::ALValue::array(0.285283, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[11].arraySetSize(1);
    keys[11].arraySetSize(1);

    times[11][0] = 1.46667;
    keys[11][0] = AL::ALValue::array(-0.0767419, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[12].arraySetSize(1);
    keys[12].arraySetSize(1);

    times[12][0] = 1.46667;
    keys[12][0] = AL::ALValue::array(0.075124, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[13].arraySetSize(1);
    keys[13].arraySetSize(1);

    times[13][0] = 1.46667;
    keys[13][0] = AL::ALValue::array(0.737812, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnklePitch");
    times[14].arraySetSize(1);
    keys[14].arraySetSize(1);

    times[14][0] = 1.46667;
    keys[14][0] = AL::ALValue::array(-0.516916, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnkleRoll");
    times[15].arraySetSize(1);
    keys[15].arraySetSize(1);

    times[15][0] = 1.46667;
    keys[15][0] = AL::ALValue::array(0.11049, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[16].arraySetSize(1);
    keys[16].arraySetSize(1);

    times[16][0] = 1.46667;
    keys[16][0] = AL::ALValue::array(0.673468, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[17].arraySetSize(1);
    keys[17].arraySetSize(1);

    times[17][0] = 1.46667;
    keys[17][0] = AL::ALValue::array(0.621227, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[18].arraySetSize(1);
    keys[18].arraySetSize(1);

    times[18][0] = 1.46667;
    keys[18][0] = AL::ALValue::array(0.99966, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipPitch");
    times[19].arraySetSize(1);
    keys[19].arraySetSize(1);

    times[19][0] = 1.46667;
    keys[19][0] = AL::ALValue::array(0.483168, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipRoll");
    times[20].arraySetSize(1);
    keys[20].arraySetSize(1);

    times[20][0] = 1.46667;
    keys[20][0] = AL::ALValue::array(-0.176367, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RKneePitch");
    times[21].arraySetSize(1);
    keys[21].arraySetSize(1);

    times[21][0] = 1.46667;
    keys[21][0] = AL::ALValue::array(0.40962, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[22].arraySetSize(1);
    keys[22].arraySetSize(1);

    times[22][0] = 1.46667;
    keys[22][0] = AL::ALValue::array(0.108956, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[23].arraySetSize(1);
    keys[23].arraySetSize(1);

    times[23][0] = 1.46667;
    keys[23][0] = AL::ALValue::array(-0.0245859, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[24].arraySetSize(1);
    keys[24].arraySetSize(1);

    times[24][0] = 1.46667;
    keys[24][0] = AL::ALValue::array(-1.45888, AL::ALValue::array(3, -0.488889, 0), AL::ALValue::array(3, 0, 0));


    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);


    posture.goToPosture("Stand", 0.5);

}

void GuessTheEmotion::angry()
{
    //Set eye color to red
    eyes.fadeRGB("Eyes", "red", 0.5);


    //Bezier custom motion imported from Choregraphe
    std::vector<std::string> names;
    AL::ALValue times, keys;
    names.reserve(26);
    times.arraySetSize(26);
    keys.arraySetSize(26);

    names.push_back("HeadPitch");
    times[0].arraySetSize(7);
    keys[0].arraySetSize(7);

    times[0][0] = 1.8;
    keys[0][0] = AL::ALValue::array(-0.00924587, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[0][1] = 2.6;
    keys[0][1] = AL::ALValue::array(0.195477, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[0][2] = 3.4;
    keys[0][2] = AL::ALValue::array(-0.00924586, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[0][3] = 4.2;
    keys[0][3] = AL::ALValue::array(0.193732, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[0][4] = 5;
    keys[0][4] = AL::ALValue::array(-0.00924586, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.4, 0));
    times[0][5] = 6.2;
    keys[0][5] = AL::ALValue::array(-0.00924586, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[0][6] = 6.8;
    keys[0][6] = AL::ALValue::array(0.0199001, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("HeadYaw");
    times[1].arraySetSize(7);
    keys[1].arraySetSize(7);

    times[1][0] = 1.8;
    keys[1][0] = AL::ALValue::array(-0.01845, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[1][1] = 2.6;
    keys[1][1] = AL::ALValue::array(-0.500909, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[1][2] = 3.4;
    keys[1][2] = AL::ALValue::array(-0.01845, AL::ALValue::array(3, -0.266667, -0.226311), AL::ALValue::array(3, 0.266667, 0.226311));
    times[1][3] = 4.2;
    keys[1][3] = AL::ALValue::array(0.856957, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[1][4] = 5;
    keys[1][4] = AL::ALValue::array(-0.01845, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.4, 0));
    times[1][5] = 6.2;
    keys[1][5] = AL::ALValue::array(-0.01845, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[1][6] = 6.8;
    keys[1][6] = AL::ALValue::array(0.00149202, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnklePitch");
    times[2].arraySetSize(5);
    keys[2].arraySetSize(5);

    times[2][0] = 1.8;
    keys[2][0] = AL::ALValue::array(-0.346726, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[2][1] = 3.4;
    keys[2][1] = AL::ALValue::array(-0.346725, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[2][2] = 5;
    keys[2][2] = AL::ALValue::array(-0.346725, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[2][3] = 6.2;
    keys[2][3] = AL::ALValue::array(-0.346725, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[2][4] = 6.8;
    keys[2][4] = AL::ALValue::array(-0.346726, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LAnkleRoll");
    times[3].arraySetSize(5);
    keys[3].arraySetSize(5);

    times[3][0] = 1.8;
    keys[3][0] = AL::ALValue::array(-0.00302601, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[3][1] = 3.4;
    keys[3][1] = AL::ALValue::array(-0.00302602, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[3][2] = 5;
    keys[3][2] = AL::ALValue::array(-0.00302602, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[3][3] = 6.2;
    keys[3][3] = AL::ALValue::array(-0.00302602, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[3][4] = 6.8;
    keys[3][4] = AL::ALValue::array(-0.00302601, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowRoll");
    times[4].arraySetSize(9);
    keys[4].arraySetSize(9);

    times[4][0] = 1.8;
    keys[4][0] = AL::ALValue::array(-1.03234, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][1] = 2.6;
    keys[4][1] = AL::ALValue::array(-0.0767945, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][2] = 3.4;
    keys[4][2] = AL::ALValue::array(-1.03234, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][3] = 4.2;
    keys[4][3] = AL::ALValue::array(-0.0767945, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[4][4] = 5;
    keys[4][4] = AL::ALValue::array(-1.03234, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.133333, 0));
    times[4][5] = 5.4;
    keys[4][5] = AL::ALValue::array(-0.0349066, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[4][6] = 5.8;
    keys[4][6] = AL::ALValue::array(-1.54462, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[4][7] = 6.2;
    keys[4][7] = AL::ALValue::array(-1.03234, AL::ALValue::array(3, -0.133333, -0.0194303), AL::ALValue::array(3, 0.2, 0.0291455));
    times[4][8] = 6.8;
    keys[4][8] = AL::ALValue::array(-1.00319, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LElbowYaw");
    times[5].arraySetSize(5);
    keys[5].arraySetSize(5);

    times[5][0] = 1.8;
    keys[5][0] = AL::ALValue::array(-1.70892, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[5][1] = 3.4;
    keys[5][1] = AL::ALValue::array(-1.70892, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[5][2] = 5;
    keys[5][2] = AL::ALValue::array(-1.70892, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[5][3] = 6.2;
    keys[5][3] = AL::ALValue::array(-1.70892, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[5][4] = 6.8;
    keys[5][4] = AL::ALValue::array(-1.41286, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHand");
    times[6].arraySetSize(9);
    keys[6].arraySetSize(9);

    times[6][0] = 1.8;
    keys[6][0] = AL::ALValue::array(0.2624, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[6][1] = 2.6;
    keys[6][1] = AL::ALValue::array(1, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[6][2] = 3.4;
    keys[6][2] = AL::ALValue::array(0.2624, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[6][3] = 4.2;
    keys[6][3] = AL::ALValue::array(1, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[6][4] = 5;
    keys[6][4] = AL::ALValue::array(0.2624, AL::ALValue::array(3, -0.266667, 0.222222), AL::ALValue::array(3, 0.133333, -0.111111));
    times[6][5] = 5.4;
    keys[6][5] = AL::ALValue::array(0, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[6][6] = 5.8;
    keys[6][6] = AL::ALValue::array(0, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[6][7] = 6.2;
    keys[6][7] = AL::ALValue::array(0.2624, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[6][8] = 6.8;
    keys[6][8] = AL::ALValue::array(0.2496, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipPitch");
    times[7].arraySetSize(5);
    keys[7].arraySetSize(5);

    times[7][0] = 1.8;
    keys[7][0] = AL::ALValue::array(-0.44175, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[7][1] = 3.4;
    keys[7][1] = AL::ALValue::array(-0.44175, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[7][2] = 5;
    keys[7][2] = AL::ALValue::array(-0.44175, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[7][3] = 6.2;
    keys[7][3] = AL::ALValue::array(-0.44175, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[7][4] = 6.8;
    keys[7][4] = AL::ALValue::array(-0.447886, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipRoll");
    times[8].arraySetSize(5);
    keys[8].arraySetSize(5);

    times[8][0] = 1.8;
    keys[8][0] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[8][1] = 3.4;
    keys[8][1] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[8][2] = 5;
    keys[8][2] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[8][3] = 6.2;
    keys[8][3] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[8][4] = 6.8;
    keys[8][4] = AL::ALValue::array(4.19617e-05, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LHipYawPitch");
    times[9].arraySetSize(5);
    keys[9].arraySetSize(5);

    times[9][0] = 1.8;
    keys[9][0] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[9][1] = 3.4;
    keys[9][1] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[9][2] = 5;
    keys[9][2] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[9][3] = 6.2;
    keys[9][3] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[9][4] = 6.8;
    keys[9][4] = AL::ALValue::array(-0.00916195, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LKneePitch");
    times[10].arraySetSize(5);
    keys[10].arraySetSize(5);

    times[10][0] = 1.8;
    keys[10][0] = AL::ALValue::array(0.697928, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[10][1] = 3.4;
    keys[10][1] = AL::ALValue::array(0.697927, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[10][2] = 5;
    keys[10][2] = AL::ALValue::array(0.697927, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[10][3] = 6.2;
    keys[10][3] = AL::ALValue::array(0.697927, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[10][4] = 6.8;
    keys[10][4] = AL::ALValue::array(0.693326, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderPitch");
    times[11].arraySetSize(9);
    keys[11].arraySetSize(9);

    times[11][0] = 1.8;
    keys[11][0] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][1] = 2.6;
    keys[11][1] = AL::ALValue::array(1.1781, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][2] = 3.4;
    keys[11][2] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][3] = 4.2;
    keys[11][3] = AL::ALValue::array(1.1781, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[11][4] = 5;
    keys[11][4] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.133333, 0));
    times[11][5] = 5.4;
    keys[11][5] = AL::ALValue::array(-0.453786, AL::ALValue::array(3, -0.133333, -0.0654498), AL::ALValue::array(3, 0.133333, 0.0654498));
    times[11][6] = 5.8;
    keys[11][6] = AL::ALValue::array(-0.272271, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[11][7] = 6.2;
    keys[11][7] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[11][8] = 6.8;
    keys[11][8] = AL::ALValue::array(1.39283, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LShoulderRoll");
    times[12].arraySetSize(8);
    keys[12].arraySetSize(8);

    times[12][0] = 1.8;
    keys[12][0] = AL::ALValue::array(-0.289968, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][1] = 2.6;
    keys[12][1] = AL::ALValue::array(0.47473, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][2] = 3.4;
    keys[12][2] = AL::ALValue::array(-0.289967, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][3] = 4.2;
    keys[12][3] = AL::ALValue::array(0.47473, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][4] = 5;
    keys[12][4] = AL::ALValue::array(-0.289967, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[12][5] = 5.8;
    keys[12][5] = AL::ALValue::array(-0.221657, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.133333, 0));
    times[12][6] = 6.2;
    keys[12][6] = AL::ALValue::array(-0.289967, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[12][7] = 6.8;
    keys[12][7] = AL::ALValue::array(0.26534, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("LWristYaw");
    times[13].arraySetSize(5);
    keys[13].arraySetSize(5);

    times[13][0] = 1.8;
    keys[13][0] = AL::ALValue::array(-0.216336, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[13][1] = 3.4;
    keys[13][1] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[13][2] = 5;
    keys[13][2] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[13][3] = 6.2;
    keys[13][3] = AL::ALValue::array(-0.216335, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[13][4] = 6.8;
    keys[13][4] = AL::ALValue::array(-0.023052, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnklePitch");
    times[14].arraySetSize(5);
    keys[14].arraySetSize(5);

    times[14][0] = 1.8;
    keys[14][0] = AL::ALValue::array(-0.352778, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[14][1] = 3.4;
    keys[14][1] = AL::ALValue::array(-0.352778, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[14][2] = 5;
    keys[14][2] = AL::ALValue::array(-0.352778, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[14][3] = 6.2;
    keys[14][3] = AL::ALValue::array(-0.352778, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[14][4] = 6.8;
    keys[14][4] = AL::ALValue::array(-0.352778, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RAnkleRoll");
    times[15].arraySetSize(5);
    keys[15].arraySetSize(5);

    times[15][0] = 1.8;
    keys[15][0] = AL::ALValue::array(0.0061779, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[15][1] = 3.4;
    keys[15][1] = AL::ALValue::array(0.00617791, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[15][2] = 5;
    keys[15][2] = AL::ALValue::array(0.00617791, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[15][3] = 6.2;
    keys[15][3] = AL::ALValue::array(0.00617791, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[15][4] = 6.8;
    keys[15][4] = AL::ALValue::array(0.00464392, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowRoll");
    times[16].arraySetSize(9);
    keys[16].arraySetSize(9);

    times[16][0] = 1.8;
    keys[16][0] = AL::ALValue::array(1.04316, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[16][1] = 2.6;
    keys[16][1] = AL::ALValue::array(0.0767945, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[16][2] = 3.4;
    keys[16][2] = AL::ALValue::array(1.04316, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[16][3] = 4.2;
    keys[16][3] = AL::ALValue::array(0.0767945, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[16][4] = 5;
    keys[16][4] = AL::ALValue::array(1.04316, AL::ALValue::array(3, -0.266667, -0.301748), AL::ALValue::array(3, 0.133333, 0.150874));
    times[16][5] = 5.4;
    keys[16][5] = AL::ALValue::array(1.43466, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[16][6] = 5.8;
    keys[16][6] = AL::ALValue::array(0.120428, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[16][7] = 6.2;
    keys[16][7] = AL::ALValue::array(1.04316, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[16][8] = 6.8;
    keys[16][8] = AL::ALValue::array(1.00788, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RElbowYaw");
    times[17].arraySetSize(5);
    keys[17].arraySetSize(5);

    times[17][0] = 1.8;
    keys[17][0] = AL::ALValue::array(1.69963, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[17][1] = 3.4;
    keys[17][1] = AL::ALValue::array(1.69963, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[17][2] = 5;
    keys[17][2] = AL::ALValue::array(1.69963, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[17][3] = 6.2;
    keys[17][3] = AL::ALValue::array(1.69963, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[17][4] = 6.8;
    keys[17][4] = AL::ALValue::array(1.4051, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHand");
    times[18].arraySetSize(9);
    keys[18].arraySetSize(9);

    times[18][0] = 1.8;
    keys[18][0] = AL::ALValue::array(0.2668, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[18][1] = 2.6;
    keys[18][1] = AL::ALValue::array(1, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[18][2] = 3.4;
    keys[18][2] = AL::ALValue::array(0.2668, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[18][3] = 4.2;
    keys[18][3] = AL::ALValue::array(1, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[18][4] = 5;
    keys[18][4] = AL::ALValue::array(0.2668, AL::ALValue::array(3, -0.266667, 0.222222), AL::ALValue::array(3, 0.133333, -0.111111));
    times[18][5] = 5.4;
    keys[18][5] = AL::ALValue::array(0, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[18][6] = 5.8;
    keys[18][6] = AL::ALValue::array(0, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[18][7] = 6.2;
    keys[18][7] = AL::ALValue::array(0.2668, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[18][8] = 6.8;
    keys[18][8] = AL::ALValue::array(0.2584, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipPitch");
    times[19].arraySetSize(5);
    keys[19].arraySetSize(5);

    times[19][0] = 1.8;
    keys[19][0] = AL::ALValue::array(-0.452572, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[19][1] = 3.4;
    keys[19][1] = AL::ALValue::array(-0.452573, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[19][2] = 5;
    keys[19][2] = AL::ALValue::array(-0.452573, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[19][3] = 6.2;
    keys[19][3] = AL::ALValue::array(-0.452573, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[19][4] = 6.8;
    keys[19][4] = AL::ALValue::array(-0.451038, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipRoll");
    times[20].arraySetSize(5);
    keys[20].arraySetSize(5);

    times[20][0] = 1.8;
    keys[20][0] = AL::ALValue::array(0.00157595, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[20][1] = 3.4;
    keys[20][1] = AL::ALValue::array(0.00157595, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[20][2] = 5;
    keys[20][2] = AL::ALValue::array(0.00157595, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[20][3] = 6.2;
    keys[20][3] = AL::ALValue::array(0.00157595, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[20][4] = 6.8;
    keys[20][4] = AL::ALValue::array(0.00157595, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RHipYawPitch");
    times[21].arraySetSize(5);
    keys[21].arraySetSize(5);

    times[21][0] = 1.8;
    keys[21][0] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[21][1] = 3.4;
    keys[21][1] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[21][2] = 5;
    keys[21][2] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[21][3] = 6.2;
    keys[21][3] = AL::ALValue::array(-0.00762796, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[21][4] = 6.8;
    keys[21][4] = AL::ALValue::array(-0.00916195, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RKneePitch");
    times[22].arraySetSize(5);
    keys[22].arraySetSize(5);

    times[22][0] = 1.8;
    keys[22][0] = AL::ALValue::array(0.698012, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[22][1] = 3.4;
    keys[22][1] = AL::ALValue::array(0.698011, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[22][2] = 5;
    keys[22][2] = AL::ALValue::array(0.698011, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[22][3] = 6.2;
    keys[22][3] = AL::ALValue::array(0.698011, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[22][4] = 6.8;
    keys[22][4] = AL::ALValue::array(0.699546, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderPitch");
    times[23].arraySetSize(9);
    keys[23].arraySetSize(9);

    times[23][0] = 1.8;
    keys[23][0] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[23][1] = 2.6;
    keys[23][1] = AL::ALValue::array(1.1781, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[23][2] = 3.4;
    keys[23][2] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[23][3] = 4.2;
    keys[23][3] = AL::ALValue::array(1.1781, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[23][4] = 5;
    keys[23][4] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.133333, 0));
    times[23][5] = 5.4;
    keys[23][5] = AL::ALValue::array(-0.453786, AL::ALValue::array(3, -0.133333, -0.0654498), AL::ALValue::array(3, 0.133333, 0.0654498));
    times[23][6] = 5.8;
    keys[23][6] = AL::ALValue::array(-0.272271, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[23][7] = 6.2;
    keys[23][7] = AL::ALValue::array(-0.66497, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[23][8] = 6.8;
    keys[23][8] = AL::ALValue::array(1.39905, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RShoulderRoll");
    times[24].arraySetSize(9);
    keys[24].arraySetSize(9);

    times[24][0] = 1.8;
    keys[24][0] = AL::ALValue::array(0.299088, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.266667, 0));
    times[24][1] = 2.6;
    keys[24][1] = AL::ALValue::array(-0.47473, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[24][2] = 3.4;
    keys[24][2] = AL::ALValue::array(0.299088, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[24][3] = 4.2;
    keys[24][3] = AL::ALValue::array(-0.47473, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.266667, 0));
    times[24][4] = 5;
    keys[24][4] = AL::ALValue::array(0.299088, AL::ALValue::array(3, -0.266667, 0), AL::ALValue::array(3, 0.133333, 0));
    times[24][5] = 5.4;
    keys[24][5] = AL::ALValue::array(0.158825, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.133333, 0));
    times[24][6] = 5.8;
    keys[24][6] = AL::ALValue::array(0.221657, AL::ALValue::array(3, -0.133333, -0.0233772), AL::ALValue::array(3, 0.133333, 0.0233772));
    times[24][7] = 6.2;
    keys[24][7] = AL::ALValue::array(0.299088, AL::ALValue::array(3, -0.133333, 0), AL::ALValue::array(3, 0.2, 0));
    times[24][8] = 6.8;
    keys[24][8] = AL::ALValue::array(-0.259288, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));

    names.push_back("RWristYaw");
    times[25].arraySetSize(5);
    keys[25].arraySetSize(5);

    times[25][0] = 1.8;
    keys[25][0] = AL::ALValue::array(0.228524, AL::ALValue::array(3, -0.6, 0), AL::ALValue::array(3, 0.533333, 0));
    times[25][1] = 3.4;
    keys[25][1] = AL::ALValue::array(0.228525, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.533333, 0));
    times[25][2] = 5;
    keys[25][2] = AL::ALValue::array(0.228525, AL::ALValue::array(3, -0.533333, 0), AL::ALValue::array(3, 0.4, 0));
    times[25][3] = 6.2;
    keys[25][3] = AL::ALValue::array(0.228525, AL::ALValue::array(3, -0.4, 0), AL::ALValue::array(3, 0.2, 0));
    times[25][4] = 6.8;
    keys[25][4] = AL::ALValue::array(0.02757, AL::ALValue::array(3, -0.2, 0), AL::ALValue::array(3, 0, 0));


    getParentBroker()->getMotionProxy()->angleInterpolationBezier(names, times, keys);
    posture.goToPosture("Stand", 0.5);

}

void GuessTheEmotion::say(const std::string phrase)
{
    speech.say(phrase);
}

void GuessTheEmotion::animatedSay(const std::string phrase)
{
    animated.say(phrase, "contextual");
}

void GuessTheEmotion::ledsOff()
{
    eyes.fadeRGB("Eyes", "white", 0.5);
}

void GuessTheEmotion::stand()
{
    posture.goToPosture("Stand", 0.5);
}



void GuessTheEmotion::faceDetection()
{
    memory.subscribeToEvent("FaceDetected", "GuessTheEmotion", "onFaceDetected");
    std::cout << "Subscribed to Face Detection Event: Waiting to See a Face" << std::endl;
}

void GuessTheEmotion::onFaceDetected()
{
    /* Thread-safe Precaution */
    AL::ALCriticalSection section(mutex);

    try
    {
        memory.unsubscribeToEvent("FaceDetected", "GuessTheEmotion");

    }
    catch(const AL::ALError& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;

    }


    std::cout << "Face Detected" << std::endl;


    this->greeting();

    asr.setLanguage("English");
    std::vector<std::string> wordlist;
    wordlist.push_back("yes");
    wordlist.push_back("no");
    asr.setVocabulary(wordlist, false);
    memory.subscribeToEvent("WordRecognized", "GuessTheEmotion","onWordRecognized");
    asr.pause(false);
    bool checker = false;
    while(checker == false)
    {

        while(asrCheck == 0)
        {
            qi::os::sleep(1);
        }

        if(asrCheck == 2)
        {
            checker = true;
            this->start =true;

        }

        else if(asrCheck == 3)
        {
            // speech.say("Sorry I did not understand you, could you please repeat?")
            std::cout << "Sorry I did not understand, please repeat" << std::endl;
            asrCheck = 0;

        }

        else if(asrCheck == 1)
        {
            checker = true;
            this->start = true;
        }
    }

}

void GuessTheEmotion::onWordRecognized(const std::string& name, const AL::ALValue& val, const std::string& myName)
{
    /* Thread-safe Precaution */
    // AL::ALCriticalSection section(mutex);


    /* Parser for speech recognition */
    for(unsigned int i = 0; i < val.getSize()/2 ; ++i)
    {
        std::cout << "word recognized: " << val[i*2].toString()
                     << " with confidence: " << (float)val[i*2+1] << std::endl;

        if((std::string)val[i*2] == "yes" && (float)val[i*2+1] > 0.5)
        {
            asrCheck = 1;
        }
        else if((std::string)val[i*2] == "no" && (float)val[i*2+1] > 0.5)
        {
            asrCheck = 2;
        }
        else if((std::string)val[i*2] == "happy" && (float)val[i*2+1] > 0.4)
        {
            this->emotionCheck =1;
            emotionCheck = 1;
        }
        else if((std::string)val[i*2] == "sad" && (float)val[i*2+1] > 0.4)
        {
            this->emotionCheck = 2;
            emotionCheck = 2;
        }
        else if((std::string)val[i*2] == "scared" && (float)val[i*2+1] > 0.4)
        {
            this->emotionCheck = 3;
            emotionCheck = 3;
        }
        else if((std::string)val[i*2] == "angry" && (float)val[i*2+1] > 0.4)
        {
            this->emotionCheck = 4;
            emotionCheck = 4;
        }
        else
        {
            asrCheck = 3;
            emotionCheck = 5;
        }

        qi::os::msleep(300);

    }

}

void GuessTheEmotion::rightBumperTouch()
{
    memory.subscribeToEvent("RightBumperPressed", "GuessTheEmotion", "onRightBumperTouch");
    std::cout << "Subscribed to Right Bumper Touch Event" << std::endl;
}

void GuessTheEmotion::onRightBumperTouch()
{
    /* Thread-safe Precaution */
    AL::ALCriticalSection section(mutex);

    memory.unsubscribeToEvent("RightBumperPressed", "GuessTheEmotion");
    std::cout << "Right Bumper Touch Detected" << std::endl;
    this->start = true;
}


