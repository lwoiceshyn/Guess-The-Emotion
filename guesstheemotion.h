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
#include <alproxies/alanimatedspeechproxy.h>

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


namespace AL
{
  // Forward declaration of ALBroker
  class ALBroker;
}


class GuessTheEmotion : public AL::ALModule
{
public:

    /* Constructors */

    GuessTheEmotion(boost::shared_ptr<AL::ALBroker> broker,
             const std::string &name);

    virtual ~GuessTheEmotion();

    /* Get & Set */

    bool getStartState();

    void setStartState(bool startstate);

    bool robotIsWakeUp();

    /* Event Handling */

    void subscribeToEvent();

    void faceDetection();

    void onFaceDetected();

    void rightBumperTouch();

    void onRightBumperTouch();


    /* Speech Recognition */

    void asrPause();

    void asrGame();

    void asrAgain();

    void setLanguage();

    void onWordRecognized(const std::string& name, const AL::ALValue& val, const std::string& myName);

    void setVocabulary();


    /* Behaviors */

    void happy();

    void sad();

    void scared();

    void angry();

    void greeting();

    void cheer();

    void rest();

    void wave();

    void stand();

    void say(const std::string phrase);

    void animatedSay(const std::string phrase);

    void postSay(const std::string phrase);

    void ledsOff();

    void wakeUp();

    void resetEyes();


    /* Static Variables */
    static bool start;

    static int asrCheck;

    static int emotionCheck;

    AL::ALTextToSpeechProxy speech;


private:

    /* Proxies */
    AL::ALMotionProxy motion;
    AL::ALMemoryProxy memory;
    AL::ALSpeechRecognitionProxy asr;
    AL::ALAnimatedSpeechProxy animated;
    AL::ALRobotPostureProxy posture;
    AL::ALLedsProxy eyes;
    boost::shared_ptr<AL::ALMutex> mutex;



};

#endif // GUESSTHEEMOTION_H
