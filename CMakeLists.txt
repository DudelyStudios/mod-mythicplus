AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/mod_mythicplus.conf.dist")

set_property(
  GLOBAL APPEND PROPERTY ACORE_MODULE_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPCatalogTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPKeyMathTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPModifierLadderTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPVaultTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPScoreTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPCommandTest.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests/MPPartyAndKeyTest.cpp")

set_property(
  GLOBAL APPEND PROPERTY ACORE_MODULE_TEST_INCLUDES
  "${CMAKE_CURRENT_LIST_DIR}/src")
