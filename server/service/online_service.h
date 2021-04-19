#ifndef MESSAGE_ONLINE_SERVICE_H_
#define MESSAGE_ONLINE_SERVICE_H_

#include "../message/message.h"
#include "../message/message_adapter.h"
#include <json/json.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;

/* 
    This class is used to process json online request 
 */
class OnlineService
{
private:
    enum class ControlType
    {
        LOGIN = 0,
        ENTER = 1,
        EXIT = 2,
        SEND = 3,
        GET_ROOMLIST = 4,
        SET_LOCK = 5,
        STATUS = 6,
        TEXT = 7,
    };
    struct UserInfo
    {
        string id;
        string pin;
    };
    struct RoomInfo
    {
        string id;
        string owner;
        bool is_locked;
        unordered_set<string> members;
    };
    Json::Reader json_reader_;
    unordered_map<string, UserInfo> user_map_;
    unordered_map<string, RoomInfo> room_map_;
    unordered_map<string, unique_ptr<MessageAdapter>> c2s_adapter_map_;

    MessageAdapter main_adapter_;

    bool parseJson(string json_str, Json::Value *json_value);

    bool isUserVaild(string user_id, string pin);

    bool sendFailResponse(string user_id, string text);

    bool sendFailResponse(int fd, string text);

    bool sendSuccessResponse(string user_id, const Json::Value &extra_data);

    bool sendSuccessResponse(string user_id);

    bool printCurrentStat();

public:
    OnlineService();

    bool process(string json_requset, int fd);
};

#endif // MESSAGE_ONLINE_SERVICE_H_
