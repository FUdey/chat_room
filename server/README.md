# chat_room
## server

### envrionment
- jsoncpp required https://github.com/open-source-parsers/jsoncpp
``` 
# install jsoncpp
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install jsoncpp 
``` 

- copy jsoncpp to path
```
cp ./vcpkg/installed/x64-linux/include/json /usr/local/include/
cp ./vcpkg/installed/x64-linux/lib/libjsoncpp.a /usr/local/lib/

```

- add path in /etc/profile
```
    export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

- run `source /etc/profile`

### to run chat room server
- edit config/server_config.h to server ip.
- run `make` in server dir.
- run `./chat_server` to start server.

