/*
 * @Author: cell_lin 1335954640@qq.com
 * @Date: 2022-09-14 22:54:22
 * @LastEditors: cell_lin 1335954640@qq.com
 * @LastEditTime: 2022-09-15 20:20:27
 * @FilePath: /Linux/search/src/bin/test.cc
 * @Description:
 *
 * Copyright (c) 2022 by cell_lin 1335954640@qq.com, All Rights Reserved.
 */
#include "../include/acceptor.hh"
#include "../include/tcpconnection.hh"
#include "../include/eventloop.hh"
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace yuki;

void onConnection(const TcpConnectionPtr &con)
{
    cout << con->to_string() << " has connect！" << endl;
}

void onMessage(const TcpConnectionPtr &con)
{
    std::string msg = con->recive();
    cout << ">>recv client " << msg << endl;
    con->send(msg);
}

void onClose(const TcpConnectionPtr &con)
{
    cout << con->to_string() << " has closed！" << endl;
}

void test()
{
    Acceptor acceptor("127.0.0.1", 8888);
    acceptor.ready(); //此时处于监听状态

    Eventloop loop(acceptor);
    loop.set_connection_callback(std::move(onConnection));
    loop.set_message_callback(std::move(onMessage));
    loop.set_close_callback(std::move(onClose));
    loop.loop();
}

int main(int argc, char **argv)
{
    test();
    return 0;
}
