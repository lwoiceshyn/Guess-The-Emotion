#ifndef GUESSTHEEMOTION_H
#define GUESSTHEEMOTION_H

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

#include <althread/almutex.h>
#include <althread/alcriticalsection.h>

#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <qi/os.hpp>
#include <time.h>

#include <alvalue/alvalue.h>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

extern bool global_start;

namespace AL
{
  // Forward declaration of ALBroker
  class ALBroker;
}


class GuessTheEmotion : public AL::ALModule
{
public:

    /* Function Prototyping */
    GuessTheEmotion(boost::shared_ptr<AL::ALBroker> broker,
             const std::string &name);

    virtual ~GuessTheEmotion();

    bool getStartState();

    void setStartState(bool startstate);

    bool robotIsWakeUp();

    void wakeUp();

    void stand();

    void asrPause();

    void asrGame();

    void asrAgain();

    void resetEyes();

    void setLanguage();

    void setVocabulary();

    void subscribeToEvent();

    void faceDetection();

    void onFaceDetected();

    void onWordRecognized(const std::string& name, const AL::ALValue& val, const std::string& myName);

    void rightBumperTouch();

    void onRightBumperTouch();

    void greeting();

    void cheer();

    void say(const std::string phrase);

    void happy();

    void sad();

    void scared();

    void angry();

    static bool start;

    static int asrCheck;

    static int emotionCheck;

    AL::ALTextToSpeechProxy speech;





private:
    /* Proxies */
    AL::ALMotionProxy motion;

    AL::ALMemoryProxy memory;
    AL::ALSpeechRecognitionProxy asr;
    AL::ALRobotPostureProxy posture;
    AL::ALLedsProxy eyes;

//Software not up to date for this proxy:    AL::ALWavingDetectionProxy waving;
    boost::shared_ptr<AL::ALMutex> mutex;



};

#endif // GUESSTHEEMOTION_H
