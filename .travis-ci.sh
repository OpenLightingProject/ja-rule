#!/bin/bash

# This script is triggered from the script section of .travis.yml
# It runs the appropriate commands depending on the task requested.

CPP_LINT_URL="http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py";

COVERITY_SCAN_BUILD_URL="https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh"

if [[ $TASK = 'lint' ]]; then
  # run the lint tool only if it is the requested task
  wget -O cpplint.py $CPP_LINT_URL;
  chmod u+x cpplint.py;
  # We can only do limited lint on the C firmware, as it's not C++
  ./cpplint.py \
    --filter=-legal/copyright,-build/include,-readability/casting \
    --extensions=c \
    Bootloader/firmware/src/*.c \
    $(find common boardcfg tools -name "*.c") \
    firmware/src/*.c
  if [[ $? -ne 0 ]]; then
    exit 1;
  fi;
  # Check everything else, including the firmware headers, more thoroughly
  ./cpplint.py \
    --filter=-legal/copyright,-build/include \
    Bootloader/firmware/src/*.h \
    $(find common boardcfg tools -name "*.h" -type f) \
    firmware/src/*.h \
    tests/{include,lib,tests}/*.{h,cpp}
  if [[ $? -ne 0 ]]; then
    exit 1;
  fi;
elif [[ $TASK = 'check-licences' ]]; then
  # check licences only if it is the requested task
  ./tests/scripts/enforce_licence.py
  if [[ $? -ne 0 ]]; then
    exit 1;
  fi;
elif [[ $TASK = 'doxygen' ]]; then
  # check doxygen only if it is the requested task
  cd tests
  autoreconf -i && ./configure --without-ola
  # the following is a bit of a hack to build the files normally built during
  # the build, so they are present for Doxygen to run against
  #make builtfiles
  # count the number of warnings
  warnings=$(make doxygen-doc 2>&1 >/dev/null | wc -l)
  if [[ $warnings -ne 0 ]]; then
    # print the output for info
    make doxygen-doc
    echo "Found $warnings doxygen warnings"
    exit 1;
  else
    echo "Found $warnings doxygen warnings"
  fi;
elif [[ $TASK = 'coverage' ]]; then
  # Compile with coverage for coveralls
  cd tests
  autoreconf -i && ./configure --enable-gcov && make && make check
elif [[ $TASK = 'coverity' ]]; then
  # Run Coverity Scan unless token is zero length
  # The Coverity Scan script also relies on a number of other COVERITY_SCAN_
  # variables set in .travis.yml
  if [[ ${#COVERITY_SCAN_TOKEN} -ne 0 ]]; then
    curl -s $COVERITY_SCAN_BUILD_URL | bash
  else
    echo "Skipping Coverity Scan as no token found, probably a Pull Request"
  fi;
else
  # Otherwise compile and check as normal
  cd tests
  autoreconf -i && ./configure && make check
fi
