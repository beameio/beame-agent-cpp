# beame-agent-cpp
Background, beame-insta-ssl is a heavy toolkit due to its nodejs dependency. To relativly simplify the the product we started developing a device agent, target spicifically for embedded devices with or without linux. 

This is supposed to be pure CPP 11 with boost, a just a few libs to make development resonable. The project is ment to compile with a cross chain therefore it includes all of its dependencies, i.e. openssl, boost, librapidjson, and libwebscoket of one kind or another. 



## First initiate the repos
git submodule init
git submodule update
git submodule foreach git submodule init
git submodule foreach git submodule update

# Now co
`cd lib/openssl/ 
`Configure linux-x86_64 --prefix=`(cd .. && pwd)`/bin
make install_dev`

# Now co
cd lib/boost/
./boostrap.sh
./bjam install --prefix=../bin/boost/ --with-system --with-date_time --with-random --with-thread link=static runtime-link=shared threading=multi
cmake -DCMAKE_INSTALL_PREFIX=../libsocketio/ -DBOOST_INCLUDEDIR=../bin/boost -DBOOST_LIBRARYDIR=../bin/boost/ DOPENSSL_ROOT_DIR=../bin/ -DOPENSSL_LIBRARIES=../include -DBOOST_VER:STRING=1.63.0 . 
