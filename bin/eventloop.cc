#include "../include/eventloop.hh"
#include "../include/acceptor.hh"
#include "../include/tcpconnection.hh"
#include <unistd.h>
#include <iostream>

namespace yuki
{
    Eventloop::Eventloop(Acceptor &acceptor)
        : epfd_(init_epollfd()), isloop_(false),
          evtlist_(SIZE), acceptor_(acceptor)
    {
        add_epoll_readfd(acceptor_.fd());
    }
    Eventloop::~Eventloop()
    {
        close(epfd_);
    }

    void Eventloop::loop()
    {
        isloop_ = true;
        while (isloop_)
        {
            wait_epollfd();
        }
    }

    void Eventloop::unloop()
    {
        isloop_ = false;
    }

    void Eventloop::set_connection_callback(TcpConnectionCallback &&cb)
    {
        on_connection_ = std::move(cb);
    }

    void Eventloop::set_close_callback(TcpConnectionCallback &&cb)
    {
        on_close_ = std::move(cb);
    }

    void Eventloop::set_message_callback(TcpConnectionCallback &&cb)
    {
        on_message_ = std::move(cb);
    }

    int Eventloop::init_epollfd()
    {
        int fd = epoll_create1(0);
        if (fd < 0)
        {
            perror("epoll_create1");
        }
        return fd;
    }
    void Eventloop::add_epoll_readfd(int fd)
    {
        struct epoll_event epevt;
        epevt.events = EPOLLIN;
        epevt.data.fd = fd;

        int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &epevt);
        if (ret < 0)
        {
            perror("add_epoll_readfd");
        }
    }

    void Eventloop::del_epoll_readfd(int fd)
    {
        struct epoll_event epevt;
        epevt.events = EPOLLIN;
        epevt.data.fd = fd;

        int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &epevt);
        if (ret < 0)
        {
            perror("add_epoll_readfd");
        }
    }

    void Eventloop::wait_epollfd()
    {
        int nready = 0;
        do
        {
            (nready = epoll_wait(epfd_, &*evtlist_.begin(), evtlist_.size(), 5000));
        } while (-1 == nready && errno == EINTR);
        if (-1 == nready)
        {
            perror("epoll_wait");
            return;
        }
        else if (0 == nready)
        {
            std::cout << "epoll_wait timeout\n";
        }
        else
        {
            if (nready == SIZE)
            {
                evtlist_.resize(2 * nready);
            }
            for (int i = 0; i < nready; ++i)
            {
                int fd = evtlist_[i].data.fd;
                if (fd == acceptor_.fd()) //新连接
                {
                    if (evtlist_[i].events & EPOLLIN)
                    {
                        handler_newconnection();
                    }
                }
                else
                {
                    if (evtlist_[i].events & EPOLLIN) //收到新消息
                    {
                        handler_message(fd);
                    }
                }
            }
        }
    }

    void Eventloop::handler_newconnection()
    {
        int peerfd = acceptor_.accept();
        if (peerfd < 0)
        {
            perror("accept");
            return;
        }
        add_epoll_readfd(peerfd); //添加读监听

        //注册回调函数
        TcpConnectionPtr con(new Tcpconnection(peerfd));
        con->set_connection_callback(on_connection_);
        con->set_message_callback(on_message_);
        con->set_close_callback(on_close_);

        conns_[peerfd] = con;

        //处理连接建立的事件
        con->handler_connection_callback();
    }

    void Eventloop::handler_message(int fd)
    {
        auto curr = conns_[fd];
        if (curr != nullptr)
        {
            bool flag = curr->isclosed();
            if (flag)
            {
                curr->handler_close_callback();
                del_epoll_readfd(fd);
                conns_.erase(fd);
            }
            else
            {
                curr->handler_message_callback();
            }
        }
        else
        {
            std::cout << "this connection is not exist\n";
        }
    }

} // namespace yuki
