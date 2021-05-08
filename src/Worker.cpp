#include "Worker.hpp"
#include "Delivery.hpp"
#include "AssemblyStation.hpp"
#include "Griddle.hpp"
#include "Fries.hpp"

#include <vector>
#include <pthread.h>
#include <sstream>
#include <iostream>

namespace Worker
{
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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
  pthread_cond_broadcast(&waitForTask);
}

void *Worker::Worker(void *args)
{
  std::vector<Delivery::priority_type_t> priorities;
  bool doneSomething;

  std::stringstream out;
  int id = *(int *)args;
  delete (int *)args;

  out << "Worker of id " << id << " instantiated" << std::endl;
  std::cout << out.str();
  out.str("");

  while (true)
  {
    doneSomething = false;
    while (!doneSomething)
    {
      priorities = Delivery::getPriority();
      for (auto priority : priorities)
      {
        switch (priority)
        {
        case Delivery::DELIVERY:
          out << "Worker[" << id << "]: Tentando entregar pedido." << std::endl;
          if (doneSomething = Delivery::deliverOrder())
            out << "Worker[" << id << "]: Entreguei pedido." << std::endl;
          break;

        case Delivery::BURGERS:
          out << "Worker[" << id << "]: Tentando finalizar um hambúrguer." << std::endl;
          if (doneSomething = AssemblyStation::makeBurgers())
            out << "Worker[" << id << "]: Finalizei um hambúrguer." << std::endl;

          if (!doneSomething)
          {
            out << "Worker[" << id << "]: Tentando fritar hambúrgueres." << std::endl;
            if (doneSomething = Griddle::makeBurgerMeats())
              out << "Worker[" << id << "]: Fritei hambúrgueres." << std::endl;
          }
          break;

        case Delivery::FRIES:
          out << "Worker[" << id << "]: Tentando fazer uma porção de batatas." << std::endl;
          if (doneSomething = Fries::Salting::saltFries())
            out << "Worker[" << id << "]: Fiz uma porção de batatas." << std::endl;

          if (!doneSomething)
          {
            out << "Worker[" << id << "]: Tentando colocar batatas para fritar." << std::endl;
            if (doneSomething = Fries::DeepFriers::setupDeepFrier())
              out << "Worker[" << id << "]: Coloquei batatas para fritar." << std::endl;
          }
          break;
        }

        if (doneSomething)
        {
          std::cout << out.str();
          out.str("");
          break;
        }
      }
      pthread_mutex_lock(&mutex);
      if (!doneSomething)
      {
        out << "Worker[" << id << "]: Sem tarefas para fazer, vou esperar por uma." << std::endl;
        std::cout << out.str();
        out.str("");
        pthread_cond_wait(&waitForTask, &mutex);
      }
      pthread_mutex_unlock(&mutex);
    }
  }
}
