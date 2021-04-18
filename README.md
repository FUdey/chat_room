# chat_room
## server
-jsoncpp required https://github.com/open-source-parsers/jsoncpp
``` 
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install jsoncpp 
``` 

-edit config/server_config.h to server ip.
-run `make` in chat_room dir.
-run `./chat_server` to start server.
## client

