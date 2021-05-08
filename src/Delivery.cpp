#include "Delivery.hpp"
#include "Griddle.hpp"
#include "AssemblyStation.hpp"
#include "Fries.hpp"
#include "Worker.hpp"

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
  double avgTimePerBurger, avgTimePerFries;
  int maxWorkers;

  /* Delivery and order control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t customerWaitLock = PTHREAD_MUTEX_INITIALIZER;
  int workers = 0;

  /* Orders queue */
  int totalFries = 0;
  int totalBurgers = 0;
  std::queue<order_t> ordersQueue;
}

std::vector<pthread_t> Delivery::initDelivery(int nDelivery, int nCustomers)
{
  avgTimePerBurger = (double)Griddle::griddlingTime / (double)Griddle::burgersPerBatch;
  avgTimePerBurger += (double)AssemblyStation::assemblingTime / (double)AssemblyStation::burgersPerBatch;

  avgTimePerFries = (double)Fries::DeepFriers::setupTime;
  avgTimePerFries += (double)Fries::DeepFriers::fryTime;
  avgTimePerFries /= (double)Fries::DeepFriers::friesPerBatch;
  avgTimePerFries *= (double)Fries::Salting::friesPerPortion;
  avgTimePerFries += (double)Fries::Salting::saltingTime;

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
    AssemblyStation::burgers -= front.burgers;
    Fries::Salting::portions -= front.fries;
    totalBurgers -= front.burgers;
    totalFries -= front.fries;
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
  if (--workers == maxWorkers - 1)
    Worker::broadcastAvailableTasks();
  pthread_mutex_unlock(&mutex);

  return true;
}

void Delivery::placeOrder(order_t order)
{
  pthread_mutex_lock(&mutex);
  ordersQueue.push(order);
  totalBurgers += order.burgers;
  totalFries += order.fries;
  pthread_mutex_unlock(&mutex);

  Worker::broadcastAvailableTasks();
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

    placeOrder(order);
    std::stringstream out;
    out << "Customer[" << id << "]: Pedi " << order.fries << " fritas e " << order.burgers << " hambúrgueres." << std::endl;
    std::cout << out.str();
    out.str("");

    // The customer waits for a delivery to be made before ordering more.
    pthread_mutex_lock(&customerWaitLock);
  }
}

std::vector<Delivery::priority_type_t> Delivery::getPriority()
{
  std::vector<priority_type_t> priorities;
  order_t front;
  int fries, burgers, neededFriesForOrder, neededBurgersForOrder;

  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&Fries::Salting::mutex);
  pthread_mutex_lock(&AssemblyStation::mutex);

  /* If there's nothing to be done, make workers wait */
  if (totalBurgers != 0 || totalFries != 0)
  {
    burgers = AssemblyStation::burgers;
    fries = Fries::Salting::portions;
    front = ordersQueue.front();
    neededBurgersForOrder = std::max(0, front.burgers - burgers);
    neededFriesForOrder = std::max(0, front.fries - fries);

    if (neededBurgersForOrder == 0 && neededBurgersForOrder == 0)
    {
      priorities.push_back(DELIVERY);
      if (totalBurgers * avgTimePerBurger <= totalFries * avgTimePerFries)
      {
        priorities.push_back(FRIES);
        priorities.push_back(BURGERS);
      }
      else
      {
        priorities.push_back(BURGERS);
        priorities.push_back(FRIES);
      }
    }
    else
    {
      if (neededBurgersForOrder * avgTimePerBurger <= neededFriesForOrder * avgTimePerFries)
      {
        priorities.push_back(FRIES);
        priorities.push_back(BURGERS);
      }
      else
      {
        priorities.push_back(BURGERS);
        priorities.push_back(FRIES);
      }
      priorities.push_back(DELIVERY);
    }
  }

  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&Fries::Salting::mutex);
  pthread_mutex_unlock(&AssemblyStation::mutex);

  return priorities;
}
