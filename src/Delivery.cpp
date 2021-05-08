#include "Delivery.hpp"
#include "AssemblyStation.hpp"
#include "Fries.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <queue>
#include <random>
#include <unistd.h>
#include <pthread.h>

namespace Delivery
{
  /* Constants */
  const int assemblingTime = 3;
  int maxWorkers;

  /* Delivery and order control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t customerWaitLock = PTHREAD_MUTEX_INITIALIZER;
  int workers = 0;

  /* Orders queue */
  typedef struct
  {
    int fries;
    int burgers;
  } order_t;

  std::queue<order_t> ordersQueue;
}

std::vector<pthread_t> Delivery::initDelivery(int nDelivery, int nCustomers)
{
  maxWorkers = nDelivery;

  srand(time(NULL));

  // This makes so that any other lock waits for the unlock before going.
  // This also only allows only one to be woken up, which is the desired behavior.
  pthread_mutex_lock(&customerWaitLock);

  pthread_t thread;
  int *id;
  std::vector<pthread_t> threads;

  for (int i = 0; i < nCustomers; i++)
  {
    id = new int;
    *id = i;
    pthread_create(&thread, NULL, Customer, (void *)id);
    threads.push_back(thread);
  }

  return threads;
}

bool Delivery::deliverOrder()
{
  bool canDo = false;
  order_t front;

  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&Fries::Salting::mutex);
  pthread_mutex_lock(&AssemblyStation::mutex);
  front = ordersQueue.front();

  if (workers < maxWorkers && Fries::Salting::portions >= front.fries && AssemblyStation::burgers >= front.burgers)
  {
    ordersQueue.pop();
    workers++;
    Fries::Salting::portions -= front.fries;
    AssemblyStation::burgers -= front.burgers;
    canDo = true;
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&Fries::Salting::mutex);
  pthread_mutex_unlock(&AssemblyStation::mutex);

  if (!canDo)
    return false;

  sleep(assemblingTime);

  // Allows one customer to order more.
  pthread_mutex_unlock(&customerWaitLock);

  pthread_mutex_lock(&mutex);
  workers--;
  pthread_mutex_unlock(&mutex);

  return true;
}

void *Delivery::Customer(void *args)
{
  order_t order;
  std::stringstream out;
  int id = *(int *)args;
  delete (int *)args;

  out << "Customer of id " << id << " instantiated" << std::endl;
  std::cout << out.str();
  out.str("");

  while (true)
  {

    /* Think of an order */
    order.burgers = 0;
    order.fries = 0;
    while (order.burgers <= 0 && order.fries <= 0)
    {
      order.burgers = random() % 6;
      order.fries = std::max(order.burgers + ((int)random() % 5) - 2, 0);
    }

    pthread_mutex_lock(&mutex);
    ordersQueue.push(order);
    pthread_mutex_unlock(&mutex);

    // The customer waits for a delivery to be made before ordering more.
    pthread_mutex_lock(&customerWaitLock);
  }
}
