#include "Worker.hpp"
#include "Delivery.hpp"
#include "AssemblyStation.hpp"
#include "Griddle.hpp"
#include "SaltingStation.hpp"
#include "DeepFrier.hpp"

#include <vector>
#include <pthread.h>
#include <sstream>
#include <iostream>
#include <fstream>

extern bool runThreads;
extern bool loggingEnabled;
extern std::ofstream logFile;

namespace Worker
{
  // so the condition can be used
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  // used to sleep when there are no tasks
  pthread_cond_t waitForTask = PTHREAD_COND_INITIALIZER;
}

std::vector<pthread_t> Worker::initWorkers(int nWorkers)
{
  pthread_t thread;
  int *id;
  std::vector<pthread_t> threads;

  for (int i = 0; i < nWorkers; i++)
  {
    id = new int;
    *id = i;
    pthread_create(&thread, NULL, Worker, (void *)id);
    threads.push_back(thread);
  }

  return threads;
}

void Worker::broadcastAvailableTasks()
{
  // Wake all workers to try to do some task
  pthread_cond_broadcast(&waitForTask);
}

void *Worker::Worker(void *args)
{
  std::vector<Delivery::priority_type_t> priorities;
  bool doneSomething;

  std::ostringstream out;
  int id = *(int *)args;
  delete (int *)args;

  out << "Worker of id " << id << " instantiated" << std::endl;
  if (loggingEnabled)
    logFile << out.str();
  out.str("");

  // Do tasks while possible
  while (runThreads)
  {
    doneSomething = false;
    priorities = Delivery::getPriority(); // check what tasks to do and in which order

    // Attempt each priority until something is done
    for (auto priority : priorities)
    {
      switch (priority)
      {
      case Delivery::DELIVERY:
        if ((doneSomething = Delivery::deliverOrder()))
          out << "Worker[" << id << "]: Entreguei pedido." << std::endl;
        break;

      case Delivery::BURGERS:
        if ((doneSomething = AssemblyStation::makeBurgers()))
          out << "Worker[" << id << "]: Finalizei um hambúrguer." << std::endl;

        if (!doneSomething)
        {
          if ((doneSomething = Griddle::makeBurgerMeats()))
            out << "Worker[" << id << "]: Fritei hambúrgueres." << std::endl;
        }
        break;

      case Delivery::FRIES:
        if ((doneSomething = SaltingStation::saltFries()))
          out << "Worker[" << id << "]: Fiz uma porção de batatas." << std::endl;

        if (!doneSomething)
        {
          if ((doneSomething = DeepFriers::setupDeepFrier()))
            out << "Worker[" << id << "]: Coloquei batatas para fritar." << std::endl;
        }
        break;
      }

      if (doneSomething)
      {
        if (loggingEnabled)
          logFile << out.str();
        out.str("");
        break;
      }
    }
    pthread_mutex_lock(&mutex);
    // If there was nothing to do, sleep
    if (!doneSomething)
    {
      out << "Worker[" << id << "]: Sem tarefas para fazer, vou esperar por uma." << std::endl;
      if (loggingEnabled)
        logFile << out.str();
      out.str("");

      // Wait for a task
      pthread_cond_wait(&waitForTask, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }
  return nullptr;
}
