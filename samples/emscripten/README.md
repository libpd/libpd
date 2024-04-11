# empd

emscripten + libpd

## install

first install and activate the emscripten environment. then

```
git clone --recursive https://github.com/libpd/libpd.git
cd libpd
mkdir build
cd build
emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release
emmake make
cd ../samples/emscripten/pdtest
make
```

## run

due to CORS restrictions, must be served on HTTP(S)
(file:// does not work).  sound requires interaction in browser.

```
python3 -m http.server 8080 &
firefox http://localhost:8080/index.html   # click to play
```
