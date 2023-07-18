#!/bin/bash
# --http-address=0.0.0.0 --http-port=8080 --deploy-path=/hello --docroot=.
# --http-address=0.0.0.0 --http-port=80 --docroot=.

mkdir -p ext/wt-4.10.0/build
pushd ext
pushd wt-4.10.0
pushd build

if [[ "$OSTYPE" == "msys" ]]; then

cmake .. --fresh -DCMAKE_INSTALL_PREFIX=C:/wt_install -DBOOST_PREFIX=N:/nostr_client_relay/ext/boost_1_82_0
cmake --build . --parallel 9
cmake --install . --config Debug

elif [[ "$OSTYPE" == "darwin"* ]]; then

cmake .. -DCMAKE_INSTALL_PREFIX=/Users/pvn/wt_install -DBOOST_PREFIX=/Users/pvn/git/nostr_client_relay/ext/boost_1_82_0 -DINSTALL_EXAMPLES=ON
cmake --build . --parallel 9
make install

elif [[ "$OSTYPE" == "linux-gnu"* ]]; then

u="$USER"
if [[ "$u" != "root" ]] ;
then

cmake .. -DCMAKE_INSTALL_PREFIX=/home/pvn/wt_install -DBOOST_PREFIX=/home/pvn/git/nostr_client_relay/ext/boost_1_82_0 -DINSTALL_EXAMPLES=ON
cmake --build . --parallel 9
make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/pvn/git/nostr_client_relay/ext/boost_1_82_0/stage/lib:/home/pvn/wt_install/lib
/home/pvn/wt_install/lib/Wt/examples/hello/hello.wt --http-address=0.0.0.0 --http-port=8080 --docroot=.
 
else

cmake .. -DCMAKE_INSTALL_PREFIX=/root/wt_install -DBOOST_PREFIX=/root/git/nostr_client_relay/ext/boost_1_82_0 \
-DBUILD_EXAMPLES=OFF -DENABLE_HARU=OFF -DENABLE_PANGO=OFF -DENABLE_POSTGRES=OFF -DENABLE_FIREBIRD=OFF -DENABLE_MYSQL=OFF -DENABLE_MSSQLSERVER=OFF \
-DENABLE_QT4=OFF -DENABLE_QT5=OFF -DENABLE_QT6=OFF -DENABLE_LIBWTTEST=OFF -DENABLE_LIBWTDBO=OFF -DENABLE_OPENGL=OFF
cmake --build . --parallel 9
make install
 
fi

fi

popd 
popd 
popd 


