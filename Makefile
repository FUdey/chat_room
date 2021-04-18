OBJS = ./out/chat_server.o ./out/online_service.o ./out/message.o ./out/message_adapter.o
chat_server: $(OBJS)
	g++ -std=c++11 $(OBJS) -o chat_server `mysql_config --cflags --libs` -ljsoncpp
./out/chat_server.o: chat_server.cc ./message/message.h ./message/message_adapter.h ./service/online_service.h ./config/server_config.h
	g++ -std=c++11 -c chat_server.cc -o ./out/chat_server.o
./out/online_service.o: ./service/online_service.cc ./message/protocol.h ./message/message_adapter.h ./message/message.h
	g++ -std=c++11 -c ./service/online_service.cc -o ./out/online_service.o
./out/message.o: ./message/message.cc ./message/protocol.h
	g++ -std=c++11 -c ./message/message.cc -o ./out/message.o
./out/message_adapter.o: ./message/message_adapter.cc ./message/protocol.h ./message/message.h
	g++ -std=c++11 -c ./message/message_adapter.cc -o ./out/message_adapter.o

clean:
	rm -rf ./out/*.o main