rem RUN TEST SUITE

set PATH=%PATH%;C:\Programme\pd\bin\

pd -lib ..\iemmatrix -path ..\abs -nogui runtests_nogui.pd > runtests.log 2>&1