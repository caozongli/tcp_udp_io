#ifndef __WRAP_H_
#define __WRAP_h_

void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);
void Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
void Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
int Epoll_create(int len);
int Epoll_ctl(int efd, int op, int listenfd, struct epoll_event* tep);
int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int __timeout);
static ssize_t my_read(int fd, char *ptr);
static Readline(int fd, void *vptr, size_t maxlen);

#endif
