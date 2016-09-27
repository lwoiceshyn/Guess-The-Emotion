cmake_minimum_required(VERSION 2.8)
project(guesstheemotion)

find_package(qibuild)

option(EMOTIONS_IS_REMOTE
  "module is compiled as a remote module (ON or OFF)"
  ON)


set(_srcs
  main.cpp
  guesstheemotion.h
  guesstheemotion.cpp
)



if(EMOTIONS_IS_REMOTE)
  add_definitions(" -DEMOTIONS_IS_REMOTE ")
  qi_create_bin(guesstheemotion ${_srcs})
else()
  qi_create_lib(guesstheemotion SHARED ${_srcs} SUBFOLDER naoqi)
endif()



qi_use_lib(guesstheemotion ALCOMMON ALPROXIES ALTHREAD)


