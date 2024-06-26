#
# Copyright (C) 2021-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

# disable optimizations for ults
if(MSVC)
  string(REPLACE "/O2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE "/O2" "/Od" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
  string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASEINTERNAL ${CMAKE_CXX_FLAGS_RELEASEINTERNAL})
  string(REPLACE "/O2" "/Od" CMAKE_C_FLAGS_RELEASEINTERNAL ${CMAKE_C_FLAGS_RELEASEINTERNAL})
else()
  string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_upper)

  string(REGEX REPLACE "-O[1-3]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REGEX REPLACE "-O[1-3]" "" CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_upper} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_upper}})
  #disable fortify source as this requires optimization to be on
  string(REPLACE "-Wp,-D_FORTIFY_SOURCE=2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "-D_FORTIFY_SOURCE=2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
  set(CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASEINTERNAL} -O0")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0")
  set(CMAKE_C_FLAGS_RELEASEINTERNAL "${CMAKE_C_FLAGS_RELEASEINTERNAL} -O0")
endif()
