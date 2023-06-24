#!/bin/bash

mkdir -p build
pushd build
if [[ "$OSTYPE" == "linux-gnu"* ]]; then

cmake .. -DWT_INCLUDE="$HOME/wt_install/include" -DWT_CONFIG_H="$HOME/wt_install/include" -DBUILD_WEB=ON

elif [[ "$OSTYPE" == "darwin"* ]]; then

cmake .. -DWT_INCLUDE="$HOME/wt_install/include" -DWT_CONFIG_H="$HOME/wt_install/include" -DBUILD_WEB=ON

elif [[ "$OSTYPE" == "msys" ]]; then

cmake .. --fresh -DBUILD_STATIC=OFF -DWT_INCLUDE=N:/nostr_client_relay/ext/wt-4.10.0/src -DWT_CONFIG_H=N:/nostr_client_relay/ext/wt-4.10.0/build -DBUILD_WEB=ON
cmake --build .

fi

cmake --build .

popd
pwd

pushd build
pushd web

remote=$(git config --get remote.origin.url)
echo "remote repository: $remote"
sleep 2
if [ "$remote" == "https://github.com/pedro-vicente/nostr_client_relay.git" ]; then
export LD_LIBRARY_PATH="$HOME/github/nostr_client_relay/ext/boost_1_82_0/stage/lib":$LD_LIBRARY_PATH
else
export LD_LIBRARY_PATH="$HOME/git/nostr_client_relay/ext/boost_1_82_0/stage/lib":$LD_LIBRARY_PATH
fi
if [[ "$OSTYPE" == "msys"* ]]; then
./Debug/nostro_web --http-address=0.0.0.0 --http-port=8080  --docroot=.
else
./nostro_web --http-address=0.0.0.0 --http-port=8080  --docroot=.
fi

exit

