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

#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <qi/os.hpp>
#include <time.h>

#include <alvalue/alvalue.h>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;


int main(int argc, char* argv[])
{

  /* Command Line Option Parser */

  // Default IP and port definitions
  int pport = 9559;
  string pip = "172.16.1.30";

  // Command line arguments must be either 1(default IP/port), 3(Single IP/Port Value), or 5(Both IP/Port Values)
  if (argc != 1 && argc != 3 && argc != 5)
  {
    cerr << "Wrong number of arguments!" << endl;
    cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << endl;
    exit(2);
  }

  // If only one argument is given, check if it's a port or IP, and use the default value for the other
  if (argc == 3)
  {
    if (string(argv[1]) == "--pip")
      pip = argv[2];
    else if (string(argv[1]) == "--pport")
      pport = atoi(argv[2]);
    else
    {
      cerr << "Wrong number of arguments!" << endl;
      cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << endl;
      exit(2);
    }
  }

  // If both IP and port values are given, determine which is which and assign the values
  if (argc == 5)
  {
    if (string(argv[1]) == "--pport"
        && string(argv[3]) == "--pip")
    {
      pport = atoi(argv[2]);
      pip = argv[4];
    }
    else if (string(argv[3]) == "--pport"
             && string(argv[1]) == "--pip")
    {
      pport = atoi(argv[4]);
      pip = argv[2];
    }
    else
    {
      cerr << "Wrong number of arguments!" << endl;
      cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << endl;
      exit(2);
    }
  }

  // Need this to for SOAP serialization of floats to work
  setlocale(LC_NUMERIC, "C");

  // Name definition of broker
  const string brokerName = "mybroker";
  // Look for a free port
  int brokerPort = 0;
  // Listen to the port of the broker
  const string brokerIp = "0.0.0.0";


  /* Broker Definition*/
  boost::shared_ptr<AL::ALBroker> broker;
  try
  {
    broker = AL::ALBroker::createBroker(
        brokerName,
        brokerIp,
        brokerPort,
        pip,
        pport,
        0
      );
  }
  catch(...)
  {
    cerr << "Fail to connect broker to: "
              << pip
              << ":"
              << pport
              << endl;

    AL::ALBrokerManager::getInstance()->killAllBroker();
    AL::ALBrokerManager::kill();

    return 1;
  }

  // Deal with ALBrokerManager singleton (add your broker into NAOqi)
  AL::ALBrokerManager::setInstance(broker->fBrokerManager.lock());
  AL::ALBrokerManager::getInstance()->addBroker(broker);

  /* Creating the module for displaying various emotions */
  AL::ALModule::createModule<GuessTheEmotion>(broker, "GuessTheEmotion");
  GuessTheEmotion state(broker, "GuessTheEmotion");

  /*Necessary for rand function to produce different results each time the program runs */
  srand (time(NULL));


  /*Over-arching Sequence Loop */
  bool repeat = true;
  while(repeat == true)
  {
      /* Emotion Game Initialization */
      state.asrPause();
      state.faceDetection();
      if(state.getStartState() == true)
      {
          state.setStartState(false);
      }

      /* Loop while waiting for a game to begin */

      while(state.start == false)
      {
          // cout << state.start << endl;   Tester for exiting loop
          qi::os::sleep(1);
      }

      /* Game sequence initializing */

      if(state.asrCheck == 1)
      {

           state.asrPause();
           state.ledsOff();
           cout << "This is when the game should begin" << endl;
           bool repeatgame = true;
           // state.say("Great, let's get started.");
           state.animatedSay("Great, let's get started.");
           qi::os::sleep(1);
           // state.say("I'm going to show you an emotion, and it's your job to guess what it is.");
           state.animatedSay("I'm going to show you an emotion, and it's your job to guess what it is.");
           qi::os::sleep(1);
           state.animatedSay("The four options are happy, sad, scared, and angry.");
           // state.say("The four options are happy, sad, scared, and angry.");
           qi::os::sleep(1);


           /* Game Sequence */
           while(repeatgame == true)
           {
                //state.say("Let's get started. Here is the first emotion.");
                state.animatedSay("Let's get started. Here is the first emotion.");
                std::cout << "Displaying first emotion" << endl;
                qi::os::sleep(1);


                std::vector<int> emotionlist;
                emotionlist.push_back(1);
                emotionlist.push_back(2);
                emotionlist.push_back(3);
                emotionlist.push_back(4);

                state.emotionCheck = 0;

                /*Pick the first emotion at random and remove it from the emotion list */
                int firstemotion = 1 + ( std::rand() % ( 4 - 1 + 1));
                emotionlist.erase(std::remove(emotionlist.begin(), emotionlist.end(), firstemotion), emotionlist.end());
                std::cout << emotionlist << std::endl;
                std::cout << firstemotion << std::endl;

                qi::os::sleep(1);

                if(firstemotion == 1)
                {
                    state.happy();
                }
                else if(firstemotion == 2)
                {
                    state.sad();
                }
                else if(firstemotion == 3)
                {
                    state.scared();
                }
                else if(firstemotion == 4)
                {
                    state.angry();
                }

                state.ledsOff();
                state.animatedSay("Okay, guess which emotion I just displayed.");
                // state.say("Okay, guess which emotion I just displayed.");
                std::cout << "Guess the emotion" << endl;
                state.asrGame();

                bool repeatone = true;
                while(repeatone == true)
                {

                    if(firstemotion == state.emotionCheck)
                    {
                        state.asrPause();
                        state.ledsOff();
                        // state.say("You got it right! Let's do the next emotion");
                        state.animatedSay("You got it right! Let's do the next emotion");
                        std::cout << "Correct. Next Emotion." << endl;
                        state.emotionCheck = 0;
                        state.asrPause();
                        repeatone = false;
                    }
                    else if(state.emotionCheck == 5)
                    {
                        // state.animatedSay("Sorry, I didn't understand you. Please repeat your answer.");
                        state.say("Sorry, I didn't understand you. Please repeat your answer.");
                        std::cout << state.emotionCheck << endl;
                        state.emotionCheck = 0;
                    }
                    else if(firstemotion != state.emotionCheck && state.emotionCheck != 0)
                    {
                        // state.animatedSay("Sorry, that is incorrect. Please try a different answer.");
                        state.say("Sorry, that is incorrect. Please try a different answer.");
                        std::cout << "Sorry that is incorrect. Please try a different emotion." << endl;
                        state.emotionCheck = 0;
                    }
                }

                qi::os::sleep(2);
                state.stand();

                std::cout << "Displaying the second emotion." << endl;
                int indextwo = 1 + ( std::rand() % ( 3 - 1 + 1));
                int secondemotion = emotionlist[indextwo-1];
                emotionlist.erase(std::remove(emotionlist.begin(), emotionlist.end(), secondemotion), emotionlist.end());
                std::cout << emotionlist << std::endl;
                std::cout << secondemotion << std::endl;

                qi::os::sleep(1);

                if(secondemotion == 1)
                {
                    state.happy();
                }
                else if(secondemotion == 2)
                {
                    state.sad();
                }
                else if(secondemotion == 3)
                {
                    state.scared();
                }
                else if(secondemotion == 4)
                {
                    state.angry();
                }

                state.ledsOff();
                // state.say("Okay, guess which emotion I just displayed.");
                state.animatedSay("Okay, guess which emotion I just displayed.");
                std::cout << "Guess the emotion" << endl;
                state.asrGame();

                bool repeattwo = true;
                while(repeattwo == true)
                {

                    if(secondemotion == state.emotionCheck)
                    {
                        state.asrPause();
                        state.ledsOff();
                        // state.say("You got it right! Let's do the next emotion");
                        state.animatedSay("You got it right! Let's do the next emotion");
                        std::cout << "Correct. Next Emotion." << endl;
                        state.emotionCheck = 0;
                        repeattwo = false;
                    }
                    else if(state.emotionCheck == 5)
                    {
                        state.say("Sorry, I didn't understand you.");
                        //state.animatedSay("Sorry, I didn't understand you.");
                        std::cout << state.emotionCheck << endl;
                        state.emotionCheck = 0;
                    }
                    else if(secondemotion != state.emotionCheck && state.emotionCheck != 0)
                    {
                        //state.animatedSay("Sorry, that is incorrect. Please try a different answer.");
                        state.say("Sorry, that is incorrect. Please try a different answer.");
                        std::cout << "Sorry that is incorrect. Please try a different emotion." << endl;
                        state.emotionCheck = 0;
                    }
                }

                std::cout << "Displaying the third emotion." << endl;
                int indexthree = 1 + ( std::rand() % ( 2 - 1 + 1));
                int thirdemotion = emotionlist[indexthree-1];
                emotionlist.erase(std::remove(emotionlist.begin(), emotionlist.end(), thirdemotion), emotionlist.end());
                std::cout << emotionlist << std::endl;
                std::cout << thirdemotion << std::endl;

                qi::os::sleep(1);

                if(thirdemotion == 1)
                {
                    state.happy();
                }
                else if(thirdemotion == 2)
                {
                    state.sad();
                }
                else if(thirdemotion == 3)
                {
                    state.scared();
                }
                else if(thirdemotion == 4)
                {
                    state.angry();
                }

                state.ledsOff();
                // state.say("Okay, guess which emotion I just displayed.");
                state.animatedSay("Okay, guess which emotion I just displayed.");
                std::cout << "Guess the emotion" << endl;
                state.asrGame();

                bool repeatthree = true;
                while(repeatthree == true)
                {

                    if(thirdemotion == state.emotionCheck)
                    {
                        state.asrPause();
                        state.ledsOff();
                        // state.say("You got it right! Let's do the next emotion");
                        state.animatedSay("You got it right! Let's do the next emotion");
                        std::cout << "Correct. Next Emotion." << endl;
                        state.emotionCheck = 0;
                        repeatthree = false;
                    }
                    else if(state.emotionCheck == 5)
                    {
                        state.say("Sorry, I didn't understand you.");
                        //state.animatedSay("Sorry, I didn't understand you.");
                        std::cout << state.emotionCheck << endl;
                        state.emotionCheck = 0;
                    }
                    else if(thirdemotion != state.emotionCheck && state.emotionCheck != 0)
                    {
                        state.say("Sorry, that is incorrect. Please try a different answer.");
                        //state.animatedSay("Sorry, that is incorrect. Please try a different answer.");
                        std::cout << "Sorry that is incorrect. Please try a different emotion." << endl;
                        state.emotionCheck = 0;
                    }
                }

                qi::os::sleep(2);
                state.stand();

                std::cout << "Displaying the fourth emotion." << endl;
                int fourthemotion = emotionlist[0];
                emotionlist.erase(std::remove(emotionlist.begin(), emotionlist.end(), fourthemotion), emotionlist.end());
                std::cout << emotionlist << std::endl;
                std::cout << fourthemotion << std::endl;

                qi::os::sleep(1);

                if(fourthemotion == 1)
                {
                    state.happy();
                }
                else if(fourthemotion == 2)
                {
                    state.sad();
                }
                else if(fourthemotion == 3)
                {
                    state.scared();
                }
                else if(fourthemotion == 4)
                {
                    state.angry();
                }

                state.ledsOff();
                // state.say("Okay, guess which emotion I just displayed.");
                state.animatedSay("Okay, guess which emotion I just displayed.");
                std::cout << "Guess the emotion" << endl;
                state.asrGame();

                bool repeatfour = true;
                while(repeatfour == true)
                {

                    if(fourthemotion == state.emotionCheck)
                    {
                        state.asrPause();
                        state.ledsOff();
                        std::cout << "Correct." << endl;
                        state.emotionCheck = 0;
                        repeatfour = false;
                    }
                    else if(state.emotionCheck == 5)
                    {
                        state.say("Sorry, I didn't understand you.");
                        // state.animatedSay("Sorry, I didn't understand you.");
                        std::cout << state.emotionCheck << endl;
                        state.emotionCheck = 0;
                    }
                    else if(fourthemotion != state.emotionCheck && state.emotionCheck != 0)
                    {
                        state.say("Sorry, that is incorrect. Please try a different answer.");
                        // state.animatedSay("Sorry, that is incorrect. Please try a different answer.");
                        std::cout << "Sorry that is incorrect. Please try a different emotion." << endl;
                        state.emotionCheck = 0;
                    }

                 }

           qi::os::sleep(2);
           state.stand();
           state.speech.post.say("Congratulations, you guessed them all right! Thanks for playing. Would you like to play again? Say yes or no.");
           std::cout << "Congratulations, you won! Would you like to play again?" << endl;
           state.cheer();
           state.asrCheck = 0;
           state.asrAgain();

           bool thelastboolean = true;
           while(thelastboolean == true)
           {
               if(state.asrCheck == 1)
               {
                   thelastboolean = false;
               }
               else if(state.asrCheck == 2)
               {
                   repeatgame = false;
                   thelastboolean = false;
                   repeat = false;
               }
           }

           state.asrPause();
           state.ledsOff();
           state.asrCheck = 0;
           state.stand();
           qi::os::sleep(2);
           }


           state.asrPause();
           state.asrCheck = 0;
           state.stand();
           state.speech.post.say("That was fun! Bye for now.");
           state.wave();
           state.rest();
           qi::os::sleep(8);

      }

      else if(state.asrCheck == 2)
      {
          state.asrPause();
          state.ledsOff();
          state.asrCheck = 0;
          state.stand();
          // state.say("That's too bad, maybe some other time then.");
          state.animatedSay("That's too bad, maybe some other time then.");
          state.rest();
          qi::os::sleep(10);
      }

  }

  return 0;
}
