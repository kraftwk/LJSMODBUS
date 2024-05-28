#pragma once
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define X_ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
//#define X_INVALIDSOCKET(s) ((s) != SOCKET_ERROR)
#define X_ISCONNECTSUCCEED(s) ((s) != SOCKET_ERROR)
//#define X_ISCONNECTSUCCEED(s) ((s) >= 0)


#include <iostream>
#include <Winsock2.h>
#include<bits.h>
#include<string.h>
#include<string>
#include<cmath>

using X_SOCKET = SOCKET;
using namespace std;



using SOCKADDR_IN = struct sockaddr_in;


#define WRITE_REGISTERS 0x10
#define READ_REGISTERS 0x03
#define BAD_CONNECTION -1
#define MAX_MESSAGE_LENGTH 260
#define MAX_MESSAGE_LENGTH_ENRON 65535

typedef union {

    float f;
    struct
    {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;

    } raw;
} myfloat;

struct regwords
{
    uint16_t word1;
    uint16_t word2;
};





class modbus
{
public:
    modbus(string IP, uint16_t port, uint16_t salve_id);

    bool modbus_connect();
    bool modbus_disconnect();
    bool isConnected() const { return _connected; }

    void modbus_close() const;
    void set_slave_id(int id);

    //int read_7000_registers(uint16_t address, uint16_t amount, uint32_t buffer);
    int write_Registers(uint16_t address, uint16_t amount, const uint16_t* value);
    //int write_Enron_Registers(uint16_t address, uint16_t amount, const uint32_t* value);
    int read_registers(uint16_t address, uint16_t amount, uint16_t* buffer);
    int read_Enron_Registers(uint16_t address, uint16_t amount, uint32_t* buffer);




    regwords conv_fl_dw(float float_val);


    bool err{ false };
    bool _connected = false;


    int err_no{};
    string error_msg;



private:
    int btd(string val);
    string getbinary(int n, int i, string usable);
    string retsign(int var);


    uint16_t Port{};
    int _slaveid{};
    int _msg_id{};
    string IP_Address{};



    void modbus_build_request(uint8_t* to_send, uint16_t address, int func) const;
    void modbus_build_Enron_request(uint8_t* to_send_Enron, uint16_t address, int func) const;
    SSIZE_T modbus_send(uint8_t* to_send, size_t length);
    SSIZE_T modbus_send_Enron(uint8_t* to_send, size_t length);
    SSIZE_T modbus_receive(uint8_t* buffer) const;
    SSIZE_T modbus_receive_Enron(uint8_t* buffer) const;
    X_SOCKET _socket{};
    SOCKADDR_IN _server{};
    int write_func(uint16_t address, uint16_t amount, int func, const uint16_t* value);
    int read_func(uint16_t address, uint16_t amount, int func);
    int read_Enron_func(uint16_t address, uint16_t amount, int func);
    WSADATA wsadata;
    void set_bad_connection();

};

inline string modbus::getbinary(int n, int i, string usable = "")
{
    int k;
    for (k = i - 1; k >= 0; k--) {

        if ((n >> k) & 1)
        {
            usable += "1";
        }

        else
        {
            usable += "0";
        }
    }
    return usable;
}

inline string modbus::retsign(int var)
{
    string sign;
    if (var >= 0)
    {
        sign = "0";
    }
    else
    {
        sign = "1";
    }
    return sign;
}

inline int modbus::btd(string val)
{
    int convf, i;
    convf = 0;
    string buf;
    for (i = 0; i < 17; i++)
    {

        buf = val[i];
        if (buf == "1")
        {
            convf += pow(2, 15 - i);
        }

    }



    return convf;
}

inline void modbus::set_slave_id(int id)
{
    _slaveid = id;
}

inline modbus::modbus(string IP, uint16_t port = 502, uint16_t slave_id = 1)
{
    IP_Address = IP;
    Port = port;
    set_slave_id(slave_id);
    err = false;
    err_no = 0;
    _msg_id = 1;
    _connected = false;
    error_msg = "";

}

inline regwords modbus::conv_fl_dw(float float_val)
{
    regwords result;
    string test;
    myfloat var;
    var.f = float_val;
    test = ((retsign(var.f)) + (getbinary(var.raw.exponent, 8)) + (getbinary(var.raw.mantissa, 23)));
    string reg1 = test.substr(0, 16);
    string reg2 = test.substr(16, 32);

    int reg1n, reg2n;
    reg1n = btd(reg1);
    reg2n = btd(reg2);

    result.word1 = reg1n;
    result.word2 = reg2n;




    return result;
}

inline bool modbus::modbus_disconnect()
{
    if (_connected)
    {
        int closesocket(_socket);
        _connected = false;
        cout << "connection closed";
        return false;
    }
    else
    {
        cout << "connection already closed";
        return false;
    }
}

inline bool modbus::modbus_connect()
{
    if (IP_Address.empty() || Port == 0)
    {
        cout << "Missing host and or port\n";
        return false;
    }
    else
    {
        cout << "host and port found\n";
    }
    if (WSAStartup(0x202, &wsadata))
    {
        return false;
    }
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!X_ISVALIDSOCKET(_socket))
    {
        cout << "error opening socket\n";
        WSACleanup();
        return false;
    }
    else
    {
        cout << "socket opened\n";
    }
    const DWORD timeout = 20;

    setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    _server.sin_family = AF_INET;
    _server.sin_addr.s_addr = inet_addr(IP_Address.c_str());
    _server.sin_port = htons(Port);

    if (!X_ISCONNECTSUCCEED(connect(_socket, (SOCKADDR*)&_server, sizeof(_server))))
    {
        cout << "failed to connect\n";
        return false;
    }
    else
    {
        cout << "connected\n";
        _connected = true;
        return true;
    }

}

inline SSIZE_T modbus::modbus_send(uint8_t* to_send, size_t length)
{
    _msg_id++;

    return send(_socket, (const char*)to_send, (size_t)length, 0);
}

inline SSIZE_T modbus::modbus_send_Enron(uint8_t* to_send, size_t length)
{
    _msg_id++;

    return send(_socket, (const char*)to_send, (size_t)length, 0);
}

inline void modbus::modbus_build_request(uint8_t* to_send, uint16_t address, int func) const
{
    to_send[0] = (uint8_t)(_msg_id >> 8u);
    to_send[1] = (uint8_t)(_msg_id & 0x00FFu);
    to_send[2] = 0;
    to_send[3] = 0;
    to_send[4] = 0;
    to_send[6] = (uint8_t)_slaveid;
    to_send[7] = (uint8_t)func;
    to_send[8] = (uint8_t)(address >> 8u);
    to_send[9] = (uint8_t)(address & 0x00FFu);
}

inline void modbus::modbus_build_Enron_request(uint8_t* to_send_Enron, uint16_t address, int func) const
{
    to_send_Enron[0] = (uint8_t)(_msg_id >> 8u);
    to_send_Enron[1] = (uint8_t)(_msg_id & 0x00FFu);
    to_send_Enron[2] = 0;
    to_send_Enron[3] = 0;
    to_send_Enron[4] = 0;
    to_send_Enron[6] = (uint8_t)_slaveid;
    to_send_Enron[7] = (uint8_t)func;
    to_send_Enron[8] = (uint8_t)(address >> 8u);
    to_send_Enron[9] = (uint8_t)(address & 0x00FFu);
}

inline int modbus::write_func(uint16_t address, uint16_t amount, int func, const uint16_t* value)
{
    int status = 0;
    uint8_t* to_send;
    to_send = new uint8_t[13 + 2 * amount];
    modbus_build_request(to_send, address, func);
    to_send[5] = (uint16_t)(7 + 2 * amount);
    to_send[10] = (uint8_t)(amount >> 8u);
    to_send[11] = (uint8_t)(amount & 0x00FFu);
    to_send[12] = (uint8_t)(2 * amount);

    for (int i = 0; i <= amount; i++)
    {
        if (i != amount)
        {
            to_send[14 + 2 * i] = (value[i]);
            to_send[13 + 2 * i] = (value[i] >> 8);

        }


    }
    status = modbus_send((to_send), 13 + 2 * amount);
    delete[] to_send;
    return status;

}

inline int modbus::read_func(uint16_t address, uint16_t amount, int func)
{
    uint8_t to_send[12];
    modbus_build_request(to_send, address, func);
    to_send[5] = 6;
    to_send[10] = (uint8_t)(amount >> 8u);
    to_send[11] = (uint8_t)(amount & 0x00FFu);
    return modbus_send(to_send, 12);

}

inline int modbus::read_Enron_func(uint16_t address, uint16_t amount, int func)
{
    uint16_t enron_amount;
    enron_amount = amount * 2;
    uint8_t to_send_Enron[12];
    modbus_build_Enron_request(to_send_Enron, address, func);
    to_send_Enron[5] = 6;
    to_send_Enron[10] = (uint8_t)(enron_amount >> 8u);
    to_send_Enron[11] = (uint8_t)(enron_amount & 0x00FFu);
    return modbus_send_Enron(to_send_Enron, 12);

}

inline int modbus::write_Registers(uint16_t address, uint16_t amount, const uint16_t* value)
{
    if (_connected)
    {
        bool sent = true;
        while (sent)
        {
            write_func(address, amount, WRITE_REGISTERS, value);
            uint8_t to_rec[MAX_MESSAGE_LENGTH];
            SSIZE_T k = modbus_receive(to_rec);
            if (k == -1)
            {

                set_bad_connection();
                cout << "bad connection\n";
                //return BAD_CONNECTION;

                modbus_connect();
            }
            else
            {
                sent = false;
            }
        }

        //modbuserror_handle(to_rec, WRITE_REGISTERS);
        if (err)
            return err_no;
        return 0;
    }
    else
    {
        set_bad_connection();
        cout << "bad test connection\n";
        return BAD_CONNECTION;
    }
}

inline int modbus::read_registers(uint16_t address, uint16_t amount, uint16_t* buffer)
{
    if (_connected)
    {
        read_func(address, amount, READ_REGISTERS);
        uint8_t to_rec[MAX_MESSAGE_LENGTH];
        SSIZE_T k = modbus_receive(to_rec);
        if (k == -1)
        {
            set_bad_connection();
            return BAD_CONNECTION;
        }
        if (err)
            return err_no;
        for (auto i = 0; i < amount; i++)
        {
            buffer[i] = ((uint16_t)to_rec[9u + 2u * i]) << 8u;
            buffer[i] += (uint16_t)to_rec[10u + 2u * i];
        }
        return 0;
    }
    else
    {
        set_bad_connection();
        return BAD_CONNECTION;
    }
}

inline int modbus::read_Enron_Registers(uint16_t address, uint16_t amount, uint32_t* buffer)
{
    if (_connected)
    {

        read_Enron_func(address, amount, READ_REGISTERS);
        uint8_t to_rec[MAX_MESSAGE_LENGTH];
        SSIZE_T k = modbus_receive(to_rec);
        if (k == -1)
        {
            set_bad_connection();
            return BAD_CONNECTION;
        }
        if (err)
            return err_no;
        for (auto i = 0; i < (amount * 2); i++)
        {
            buffer[i] = ((uint16_t)to_rec[9u + 2u * i]) << 8u;
            buffer[i] += (uint16_t)to_rec[10u + 2u * i];
        }
        return 0;
    }
    else
    {
        set_bad_connection();
        return BAD_CONNECTION;
    }

}

inline void modbus::set_bad_connection()
{
    err = true;
    error_msg = "BAD CONNECTION";
}

inline SSIZE_T modbus::modbus_receive(uint8_t* buffer) const
{
    Sleep(500);
    return recv(_socket, (char*)buffer, MAX_MESSAGE_LENGTH, 0);
}

inline SSIZE_T modbus::modbus_receive_Enron(uint8_t* buffer) const
{
    Sleep(500);
    return recv(_socket, (char*)buffer, MAX_MESSAGE_LENGTH, 0);
}


