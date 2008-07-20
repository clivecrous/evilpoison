INCLUDE_DIRECTORIES(../src)

ADD_EXECUTABLE(tests tests.c
  check_bind.c ../src/bind.c
  check_command.c ../src/command.c
  check_dictionary.c ../src/dictionary.c
  check_settings.c ../src/settings.c)

TARGET_LINK_LIBRARIES(tests check ${X11_LIBRARIES})
