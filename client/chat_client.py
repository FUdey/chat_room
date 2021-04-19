#!python
# -*- coding: UTF-8 -*-

from tkinter import ttk
from tkinter import scrolledtext
from tkinter import *
import time
from client_service import ClientService, ctl_type
import configparser
import threading

LARGE_FONT= ("Verdana", 12)

cs = ClientService()
global_user_id = ""
global_pin = ""
global_room_id = ""
global_room_list = {}


class Application(Tk):
    def __init__(self):
        
        super().__init__()

        self.wm_title("Chat room")
        self.geometry('505x400')
        
        container = Frame(self)
        container.pack(side="top", fill="both", expand = True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}
        for page in (LoginPage, LobbyPage, RoomPage):
            frame = page(container, self)
            self.frames[page] = frame
            frame.grid(row=0, column=0, sticky="nsew") 

        self.show_frame(LoginPage)

        
    def show_frame(self, cont):
        frame = self.frames[cont]
        frame.tkraise()
        frame.data_init()

        
class LoginPage(Frame):
    def __init__(self, parent, root):
        super().__init__(parent)
        label = Label(self, text="Welcome to chat room\nplease sign in or sign up", font=LARGE_FONT)
        label.pack(side="top",pady=20,padx=10)
        self.root = root

        self.user_id = ""
        self.pin = ""
        self.user_id_label = Label(self, text="id:")
        self.user_id_hint = Label(self, text="")
        self.user_id_input = Entry(self, width=10)
        self.pin_label = Label(self, text="pin:")
        self.pin_input = Entry(self, width=10)
                    
        self.button_login = ttk.Button(self, text="login", command=self.login)
        
        self.user_id_hint.pack(side='top',pady=10,padx=10)
        self.user_id_label.pack(side='top',pady=10,padx=10,anchor="s")
        self.user_id_input.pack(side='top',pady=10,padx=10,anchor="n")
        self.pin_label.pack(side='top',pady=10,padx=10)
        self.pin_input.pack(side='top',pady=10,padx=10)
        self.button_login.pack(side='bottom',pady=10,padx=10)
    
    def login(self):
        global global_user_id, global_pin, global_room_id
        self.user_id = self.user_id_input.get()
        self.pin = self.pin_input.get() + "raw"
        print("try login:",self.user_id,self.pin)
        if self.user_id != "":
            res = cs.request_and_recv({
                "ctl":ctl_type['LOGIN'],
                "user_id":self.user_id,
                "pin":self.pin
            })
        else:
            self.user_id_hint['text'] = "id can not be empty"
            return

        if res["status"] == 1:
            global_user_id = self.user_id
            global_pin = self.pin
            print("global:",global_user_id,global_pin)
            if res["data"] is not None:
                global_room_id = res["data"]["room_id"]
            self.root.show_frame(LobbyPage)
        else:
            self.user_id_hint['text'] = res["hint"]
            return
    
    def data_init(self):
        global global_user_id, global_pin
        self.user_id = global_user_id
        self.pin = global_pin
        

class LobbyPage(Frame):
    def __init__(self, parent, root):
        super().__init__(parent)
        self.root = root
        self.room_id = ""
        
        self.label = Label(self, text="create or join  a chat room", font=LARGE_FONT)
        self.label.pack(pady=10,padx=10)

        self.button_enter = ttk.Button(self, text="enter", command=self.enter)
        self.room_label = Label(self, text="room:")
        self.room_input = Entry(self, width=10)
        # get room list
        
        self.scr = scrolledtext.ScrolledText(self, width=50, height=13,state='normal')
        self.scr.pack(side='top',pady=10,padx=10)
        
        self.scr['state'] = 'disabled'
        self.button_enter.pack(side='bottom',pady=10,padx=10)
        self.room_input.pack(side='bottom',pady=10,padx=10)
        self.room_label.pack(side='bottom',pady=10,padx=10)

    def data_init(self):
        global global_user_id, global_pin, global_room_id, global_room_list
        print("global:",global_user_id,global_pin,global_room_id,global_room_list)
        res = cs.request_and_recv({
            "ctl":ctl_type['GET_ROOMLIST'],
            "user_id":global_user_id,
            "pin":global_pin
        })
        if res["status"] == 1:
            global_room_list = res["data"]
            self.room_id = global_room_id
            self.scr['state'] = 'normal'
            self.scr.delete('0.0', END)
            self.scr.insert(END,"exist room:\n")
            if global_room_list is not None:
                for room_id in global_room_list:
                    self.scr.insert(END,"    %s\n" % room_id)
            self.scr['state'] = 'disabled'

            self.room_input.delete(0, END)
            self.room_input.insert(0, global_room_id)
            self.room_label['text'] = "room:"
            # auto enter last room
            if self.room_id != "":
                print("[debug] auto into room")
                self.enter()
        else:
            self.room_label['text'] = res["hint"]+"\nroom:"
        

    def enter(self):
        global global_user_id, global_pin, global_room_id, global_room_list
        self.room_id = self.room_input.get()
        if self.room_id != "":
            res = cs.request_and_recv({
                "ctl":ctl_type['ENTER'],
                "user_id":global_user_id,
                "pin":global_pin,
                "room_id":self.room_id
            })
        else:
            self.room_label['text'] = "room id can not be empty"+"\nroom:"
            return

        if res["status"] == 1:
            global_room_id = self.room_id
            global_room_list = res["data"]
            self.root.show_frame(RoomPage)
        else:
            self.room_label['text'] = res["hint"]+"\nroom:"
            return
        

class RoomPage(Frame):
    def __init__(self, parent, root):
        super().__init__(parent)
        self.root = root
        self.frmLT = Frame(self, width = 500, height = 270, bg = 'white')
        self.frmLC = Frame(self, width = 500, height = 50, bg = 'white')
        self.frmLB = Frame(self, width = 500, height = 30)
        # self.frmRT = Frame(self, width = 200, height = 500)

        self.txtMsgList = Text(self.frmLT)
        self.txtMsgList.tag_config('midgreen', foreground = '#008C00')
        self.txtMsgList.tag_config('midblue', foreground = '#0000CD')
        self.txtMsg = Text(self.frmLC)
        self.txtMsg.bind("<KeyPress-Up>", self.send_msg_event)

        self.button_exit = ttk.Button(self.frmLB, text="exit", width = 8, command= self.exit_room)
        self.button_lock = ttk.Button(self.frmLB, text="lock", width = 8, command= self.set_lock)
        self.button_send = ttk.Button(self.frmLB, text="send", width = 8, command = self.send)
        self.button_cancel = ttk.Button(self.frmLB, text="cancel", width = 8, command = self.cancel_msg)


        self.frmLT.grid(row = 0, column = 0, columnspan = 2, padx = 1, pady = 3)
        self.frmLC.grid(row = 1, column = 0, columnspan = 2, padx = 1, pady = 3)
        self.frmLB.grid(row = 2, column = 0, columnspan = 2)
        # self.frmRT.grid(row = 0, column = 2, rowspan = 3, padx =2, pady = 3)

        self.frmLT.grid_propagate(0)
        self.frmLC.grid_propagate(0)
        self.frmLB.grid_propagate(0)
        # self.frmRT.grid_propagate(0)

        self.button_exit.grid(row = 2, column = 3, padx =2, pady = 3)
        self.button_lock.grid(row = 2, column = 4, padx =2, pady = 3)
        self.button_send.grid(row = 2, column = 1, padx =2, pady = 3)
        self.button_cancel.grid(row = 2, column = 2, padx =2, pady = 3)
        # lblImage.grid()
        self.txtMsgList.grid()
        self.txtMsg.grid()
        self.listening = False
    
    def data_init(self):
        global global_user_id, global_pin, global_room_id, global_room_list
        if global_room_id != "":
            self.is_locked = global_room_list[global_room_id]["is_locked"]
            self.owner = global_room_list[global_room_id]["owner"]
            if self.is_locked:
                self.button_lock["text"] = "unlock"
            else:
                self.button_lock["text"] = "lock"
            if self.owner != global_user_id:
                self.button_lock["state"] = "disabled"
            else:
                self.button_lock["state"] = "normal"
            self.txtMsgList["state"] = "normal"
            self.txtMsgList.delete('0.0', END)
            self.show_text("[SYSTEM] %s into room %s." % (global_user_id, global_room_id),"")
            self.txtMsgList["state"] = "disabled"

            # start listen thread
            self.text_buffer = []
            self.add_listen_thread()
            

    def send(self):
        global global_user_id, global_pin, global_room_id
        if self.txtMsg.get('0.0', END) != "":
            res = cs.request({
                "ctl":ctl_type['SEND'],
                "user_id":global_user_id,
                "pin":global_pin,
                "room_id":global_room_id,
                "text":self.txtMsg.get('0.0', END)
            })
        else:
            return

        if res is not None and res["status"] == 1:
            self.txtMsg.delete('0.0', END)
        else:
            return
    
    def remove_read_thread(self):
        self.listening = False
        self.listen_thread.join()
        self.update_thread.join()
        print("[info] stop listening")


    def add_listen_thread(self):
        global global_user_id, global_pin, global_room_id
        self.listening = True
        self.listen_thread=threading.Thread(target=self.listen)  
        self.listen_thread.setDaemon(True)  
        self.listen_thread.start()
        self.update_thread=threading.Thread(target=self.update_msg_list)  
        self.update_thread.setDaemon(True)  
        self.update_thread.start()
        print("[info] start updateing")

    def listen(self):    
        global global_user_id, global_pin, global_room_id, cs
        while self.listening:
            self.text_buffer.extend(cs.auto_recv())
    
    def update_msg_list(self):    
        global global_user_id, global_pin, global_room_id, cs
        while self.listening:
            time.sleep(0.1)
            if len(self.text_buffer) != 0:
                for text_msg in self.text_buffer:
                    self.show_msg(text_msg)
                self.text_buffer = []
        

    def set_lock(self):
        global global_user_id, global_pin, global_room_id
        if global_room_id != "":
            res = cs.request({
                "ctl":ctl_type['SET_LOCK'],
                "user_id":global_user_id,
                "pin":global_pin,
                "room_id":global_room_id,
                "lock":not self.is_locked,
            })
        else:
            return

        if res is not None and res["status"] == 1:
            self.is_locked = not self.is_locked
            if self.is_locked:
                self.button_lock['text'] = "unlock"
            else:
                self.button_lock['text'] = "lock"
        else:
            return

    def exit_room(self):
        global global_user_id, global_pin, global_room_id
        res = cs.request({
            "ctl":ctl_type['EXIT'],
            "user_id":global_user_id,
            "pin":global_pin,
            "room_id":global_room_id,
        })

        if res is not None and res["status"] == 1:
            global_room_id = ""
            self.remove_read_thread()
            self.root.show_frame(LobbyPage)
        else:
            return

    def show_msg(self, text_msg):
        global global_user_id, global_room_id
        if text_msg['room_id'] != global_room_id:
            print("[error] recv other room's msg, server error")
            return

        user_stat = "%s: %s" % (text_msg["user_id"], time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()))
        if text_msg['user_id'] == global_user_id:
            self.show_text(user_stat, text_msg["text"], "midgreen")
        else:
            self.show_text(user_stat, text_msg["text"])
        

        
    def show_text(self, stat_str, text, color="midblue"):
        self.txtMsgList["state"] = "normal"
        self.txtMsgList.insert(END, stat_str+"\n", color)
        self.txtMsgList.insert(END, text)
        self.txtMsgList["state"] = "disabled"

    def cancel_msg(self):
        self.txtMsg.delete('0.0', END)

    def send_msg_event(self,event):
        if event.keysym =='Up':
            self.send()

if __name__ == '__main__':
    config = configparser.ConfigParser()
    config.read('config\\client_conf.ini', encoding='GB18030')
    host = config.get('server', 'host')
    port = config.getint('server', 'port')
    cs.set_server(host, port)
    app = Application()
    app.mainloop()