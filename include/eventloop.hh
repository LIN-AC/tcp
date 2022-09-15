/*
 * @Author: cell_lin 1335954640@qq.com
 * @Date: 2022-09-15 16:28:41
 * @LastEditors: cell_lin 1335954640@qq.com
 * @LastEditTime: 2022-09-15 21:00:21
 * @FilePath: /Linux/search/src/include/eventloop.hh
 * @Description:
 *
 * Copyright (c) 2022 by cell_lin 1335954640@qq.com, All Rights Reserved.
 */
#pragma once

#include <sys/epoll.h>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace yuki
{
    class Acceptor;
    class Tcpconnection;

    const int SIZE = 1024;
    using TcpConnectionPtr = std::shared_ptr<Tcpconnection>;
    using TcpConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    class Eventloop
    {
    private:
        int epfd_; // epollfd
        std::vector<struct epoll_event> evtlist_;
        bool isloop_;
        Acceptor &acceptor_;
        std::map<int, TcpConnectionPtr> conns_; //已连接

        TcpConnectionCallback on_connection_;
        TcpConnectionCallback on_message_;
        TcpConnectionCallback on_close_;

    public:
        Eventloop(Acceptor &);
        ~Eventloop();
        void loop();
        void unloop();

        //设置三个回调函数
        void set_connection_callback(TcpConnectionCallback &&);
        void set_message_callback(TcpConnectionCallback &&);
        void set_close_callback(TcpConnectionCallback &&);

    private:
        int init_epollfd();            //初始化epollfd
        void add_epoll_readfd(int fd); //对fd添加读监听
        void del_epoll_readfd(int fd); //对fd删除读监听

        void wait_epollfd();
        void handler_newconnection();
        void handler_message(int fd);
    };
} // namespace yuki
