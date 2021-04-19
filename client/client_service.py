import socket
import time
import datetime
import socket
import struct
import json

msg_type_to_id = {
    'JSON': 1,
}

ctl_type = {
    "LOGIN": 0,
    "ENTER": 1,
    "EXIT": 2,
    "SEND": 3,
    "GET_ROOMLIST": 4,
    "SET_LOCK": 5,
    "STATUS": 6,
    "TEXT": 7,
}

class ClientService:

    max_wait = 5 # second
    

    def __init__(self):
        self.server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.text_buffer = []
        self.ctl_buffer = []
    def set_server(self, host = '',port = 8888):
        self.server.connect((host, port))

    def encode_json_msg(self, data):
        json_data = json.dumps(data)
        data_bytes = json_data.encode('utf-8')
        data_length = len(data_bytes)
        data_type = msg_type_to_id['JSON']
        
        header = (data_type.to_bytes(1, byteorder = 'big', signed = False) + 
            data_length.to_bytes(4, byteorder = 'big', signed = False))
        
        return header + data_bytes

    def decode_header(self, header_bytes):
        data_type = int.from_bytes(header_bytes[:1], byteorder='big', signed = False)
        data_length = int.from_bytes(header_bytes[1:5], byteorder='big', signed = False)
        return data_type, data_length

    def decode_json_data(self, json_bytes):
        json_data = json_bytes.decode(encoding='UTF-8')
        print("[debug] recv raw json:", json_data)
        data = json.loads(json_data)
        return data
        
    def send_json_msg(self, dict_data):
        byte_data = self.encode_json_msg(dict_data)
        self.server.send(byte_data)

    def recv_json_msg(self):
        header = self.server.recv(5)
        data_type, data_length = self.decode_header(header)
        print("[debug] header:",data_type, data_length)
        byte_data = self.server.recv(data_length)
        dict_data = self.decode_json_data(byte_data)
        return dict_data

    def request(self, data):
        print("SEND: ", data)
        self.send_json_msg(data)

        # check buffer
        for i in range(self.max_wait*10):
            time.sleep(0.1)
            if len(self.ctl_buffer) != 0:
                res = self.ctl_buffer[0]
                self.ctl_buffer = []
                print("RECV in buffer: ", res)
                return res

        print("[error] RECV Failed")
        return None

    def request_and_recv(self, data):
        print("SEND: ", data)
        self.send_json_msg(data)

        # check buffer first
        if len(self.ctl_buffer) != 0:
            res = self.ctl_buffer[0]
            self.ctl_buffer = []
            print("RECV in buffer: ", res)
            return res

        res = self.recv_json_msg()
        while res["ctl"] == ctl_type["TEXT"]:
            print("RECV TEXT: ", res)
            self.text_buffer.append(res)
            res = self.recv_json_msg()
        print("RECV: ", res)
        return res
    
    def auto_recv(self):
        try:
            self.server.settimeout(1)
            while True:
                res = self.recv_json_msg()
                if res["ctl"] == ctl_type["TEXT"]:
                    print("AUTO RECV TEXT: ", res)
                    self.text_buffer.append(res)
                elif res["ctl"] == ctl_type["STATUS"]:
                    print("AUTO RECV STATUS: ", res)
                    self.ctl_buffer.append(res)
                else:
                    print("AUTO RECV UNKNOWN MSG:", res)
        except socket.timeout:
            self.server.settimeout(None)
            # print('[debug] time out, no more msg')
        finally:
            self.server.settimeout(None)
        text_list = self.text_buffer
        self.text_buffer = []
        return text_list

if __name__ == '__main__':
    cs = ClientService()
    data = {
        "key": "value"
    }
    data_b = cs.encode_json_msg(data)
    _,length = cs.decode_header(data_b[:5])
    print("text length: ",length)
    data_d = cs.decode_json_data(data_b[5:5+length])
    print("text data: ",data_d)

    cs.set_server("1.15.224.82", 8888)

    cs.request_and_recv( {
        "ctl": ctl_type['LOGIN'],
        "user_id": "xxx",
        "pin": "1234"
    })

    cs.request_and_recv( {
        "ctl": ctl_type['GET_ROOMLIST'],
        "user_id": "xxx",
        "pin": "1234"
    })

    cs.request_and_recv( {
        "ctl": ctl_type['ENTER'],
        "user_id": "xxx",
        "pin": "1234",
        "room_id": "lobby",
    })

    cs.request_and_recv( {
        "ctl": ctl_type['SEND'],
        "user_id": "xxx",
        "pin": "1234",
        "room_id": "lobby",
    })

    cs.request_and_recv( {
        "ctl": ctl_type['SET_LOCK'],
        "user_id": "xxx",
        "pin": "1234",
        "room_id": "lobby",
        "lock" : True,
    })

    cs.request_and_recv( {
        "ctl": ctl_type['SET_LOCK'],
        "user_id": "xxx",
        "pin": "1234",
        "room_id": "lobby",
        "lock" : False,
    })

    cs.request_and_recv( {
        "ctl": ctl_type['EXIT'],
        "user_id": "xxx",
        "pin": "1234",
        "room_id": "lobby",
    })
    
    cs.server.close()