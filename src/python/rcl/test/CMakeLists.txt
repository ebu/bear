# Copyright Institute of Sound and Vibration Research - All rights reserved

add_test(NAME rclpython_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${PYTHON_EXECUTABLE} -m pytest )

include( adjust_test_environment )
adjustPythonTestEnvironment( rclpython_test rclpython )
