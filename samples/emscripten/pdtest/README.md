# empd

emscripten + libpd

## install

first install and activate the emscripten environment. then

    git clone --recursive https://github.com/claudeha/libpd.git
    cd libpd
    git checkout emscripten
    cd pure-data
    git remote add claudeha https://github.com/claudeha/pure-data.git
    git fetch claudeha
    git checkout claudeha/emscripten
    cd ..
    mkdir build
    cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release
    emmake make
    cd ../samples/emscripten/pdtest
    make

## run

    python -m SimpleHTTPServer 8080 &
    firefox localhost:8080/pdtest.html   # plays automatically
    chromium localhost:8080/index.html   # click to play
