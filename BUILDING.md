## Dependencies

- CMake [https://cmake.org/]
- libsecp256k1 [https://github.com/bitcoin-core/secp256k1]
- OpenSSL [https://www.openssl.org/] (Windows build with ext/openssl-3.0.5/build.bat)
- Asio [https://think-async.com/Asio/AsioStandalone.html] 
- Simple-WebSocket-Server [https://gitlab.com/eidheim/Simple-WebSocket-Server] 
- JSON Modern C++ [https://json.nlohmann.me/] 

### For Web build

- Boost [https://www.boost.org/]
- Wt [https://www.webtoolkit.eu/wt]

## For GUI desktop build

- WxWidgets [https://www.wxwidgets.org/]

## For iOS

- Qt [https://www.qt.io/]

## Install dependencies

To install dependencies on Mac and Linux

Mac

``` cmd 
brew install cmake
brew install openssl@3.0
brew link openssl@3.0
```

Linux

``` cmd 
sudo apt-get install cmake
sudo apt-get install build-essential libgtk-3-dev 
sudo apt-get install libssl-dev 
```

# Building

All dependencies except boost, wxWidgets and Qt are included in the repository. To clone a copy with Boost, do at a shell prompt

=======

### For GUI desktop build

- WxWidgets [https://www.wxwidgets.org/]

## Building

All dependencies except boost are included in the repository. To clone a copy with Boost, do at a shell prompt

```
 git clone --recurse-submodules https://github.com/pedro-vicente/nostr_client_relay
```

Or without Boost (Boost clone can be done later)

```
 git clone  https://github.com/pedro-vicente/nostr_client_relay
```

The CMake build utility is used. There are 4 modes of build.

- command line (default)
- web 
- desktop 
- mobile 

Default is command line only. To build the source code, use on a shell

The CMake build utility is used. There are 3 modes of build. Default is command line only. To build with defaults, use
on a shell

```
cd nostr_client_relay
cmake -S . -B build
cd build
cmake --build . 

```

This builds the software on folder named 'build'. There you can find the executables and your code editor of choice
generated projects, like projects for Microsof Visual Studio (in Windows).

## :warning: Building dependency libraries

Dependencies boost, Wt, WxWidgets are needed for desktop and web build. Build with bash shell scripts

## Building for web and desktop

To enable desktop and web builds, define at command line 

```
cmake -DBUILD_WEB=ON -DBUILD_GUI=ON

```

but these require extra parameters to locate dependencies. Use the bash script to build and see examples 

``` cmd
./build.cmake.sh
```

## Install dependencies

To install dependencies on Mac and Linux

Mac

``` cmd 
brew install cmake
brew install openssl
```

Linux

``` cmd 
sudo apt-get install cmake
sudo apt-get install build-essential libgtk-3-dev 
sudo apt-get install libssl-dev 
```

For web, dependencies boost and wt are needed, build with bash shell scripts


``` cmd
./build.boost.sh
./build.wt.sh

./build.widgets.sh
```

## Building for web 

To enable web build, use 

```
cmake -DBUILD_WEB=ON

```

Use the bash script to build 

``` cmd
./build.desktop.sh 
```

To clone Boost individually, do

```

### Boost

All dependencies except Boost are included. To clone Boost individually, do


```
git clone -b boost-1.82.0 --recursive --depth=1 https://github.com/boostorg/boost.git ext/boost_1_82_0
```

## Building for desktop

To clone WxWidgets, do

```
git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git ext/wxWidgets-3.2.2.1
```

To enable desktop build, use 

```
cmake -DBUILD_DESKTOP=ON

```

or build with bash shell script

``` cmd
./build.desktop.sh 
```

## Building for iOS

To enable mobile build, use 

```
cmake -DBUILD_MOBILE=ON

```

or build with bash shell script (requires a Qt cross compile iOS builds on MacOS) 

``` cmd
./build.mobile.sh
```

