#ifndef MTHREADPOOL_H
#define MTHREADPOOL_H

#include "locker.h"
#include <list>
#include <stdio.h>
#include <exception>
#include <errno.h>
#include <pthread.h>
#include <iostream>

template<class T>
class ThreadPool{
private:
    int thread_number;  //线程池的线程数
    int max_task_number;    //任务队列中的最大任务数

    pthread_t *all_threads;  //线程数组
    std::list<T *> task_queue;  //任务队列
    mutex_locker queue_mutex_locker;    //互斥锁
    sem_locker queue_sem_locker;    //信号量

    bool is_stop;

public:
    ThreadPool(int thread_num=20,int max_task_num=30);
    ~ThreadPool();

    bool append_task(T *task);
    void start();
    void stop();
private:
    static void *worker(void *arg);
    void run();
};


template<class T>
ThreadPool<T>::ThreadPool(int thread_num, int max_task_num):
    thread_number(thread_num),max_task_number(max_task_num),is_stop(false),all_threads(NULL)
{
    if(this->thread_number<=0||this->max_task_number<=0){

//        std::cout<<"threadpool can't init because thread_number=0"
//                   " or max_task_number=0"<<std::endl;
    }
    all_threads=new pthread_t[thread_number];
    if(!all_threads){
//        std::cout<<"threadpool can't init because pthread_t can't new"<<std::endl;
    }
}

template<class T>
ThreadPool<T>::~ThreadPool()
{
    delete []all_threads;
    this->is_stop=true;
}

template<class T>
void ThreadPool<T>::start()
{
    for(int i=0;i<thread_number;i++)
    {
//        std::cout<<"create the "<<i<<"th pthread"<<std::endl;
        if(pthread_create(all_threads+i,NULL,worker,this))
        {
            delete []all_threads;
            throw std::exception();
        }

        if(pthread_detach(all_threads[i]))
        {
            delete []all_threads;
            throw std::exception();
        }
    }
}

template<class T>
void ThreadPool<T>::stop()
{
    is_stop=true;
}

template<class T>
bool ThreadPool<T>::append_task(T *task)
{
    //获取互斥锁
    queue_mutex_locker.mutex_lock();
    //判断队列中任务数是否大于最大任务数
    if(task_queue.size()>(unsigned int)max_task_number)
    {
        queue_mutex_locker.mutex_unlock();
        return false;
    }
    task_queue.push_back(task);
    queue_mutex_locker.mutex_unlock();
    queue_sem_locker.add();
    return true;
}

template<class T>
void *ThreadPool<T>::worker(void *arg)
{
    ThreadPool *pool=(ThreadPool *)arg;
    pool->run();
    return pool;
}

template <class T>
void ThreadPool<T>::run()
{
    while(!is_stop)
    {
        usleep(1000*5);
        //等待任务
        queue_sem_locker.wait();
        if(errno==EINTR)
        {
            printf("errno");
            continue;
        }

        //获取互斥锁
        queue_mutex_locker.mutex_lock();
        //判断任务队列是否为空
        if(task_queue.empty())
        {
            queue_mutex_locker.mutex_unlock();
            continue;
        }

        //获取队头任务并执行
        T *task=task_queue.front();
        task_queue.pop_front();
        queue_mutex_locker.mutex_unlock();//互斥解锁

        if(!task)
            continue;

        task->FuncHandle();

        delete task;
        task=NULL;
    }
//    printf("close %ld\n",(unsigned long)pthread_self());
}

class Task
{
private:
    int TaskID;
public:
    Task(){}
    Task(int id){
        TaskID=id;
    }
    virtual void FuncHandle()
    {
//        std::cout<<"TaskID:"<<TaskID<<std::endl;
    }
};

#endif // MTHREADPOOL_H
