#ifndef MPTHREAD_H
#define MPTHREAD_H
#include "locker.h"
#include <pthread.h>

template<class T>
class MThread{
public:
    MThread();
    ~MThread();
    void startThread(T *tp){
        tp=targ;

        pthread_t pid;
        pthread_attr_t pattr;
        pthread_attr_init(&pattr);
        pthread_attr_setscope(&pattr,PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&pattr,PTHREAD_CREATE_DETACHED);

        pthread_create(pid,&pattr,tp->FuncHandle,this);
    }

    void stopThread(){
        thread_status=false;
    }

private:
    T *targ;

    bool thread_status;
//    void lock();
//    void unlock();
};

#endif // MPTHREAD_H
