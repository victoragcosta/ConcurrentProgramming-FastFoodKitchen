#include "Delivery.hpp"
#include "Griddle.hpp"
#include "AssemblyStation.hpp"
#include "SaltingStation.hpp"
#include "DeepFrier.hpp"
#include "Worker.hpp"
#include "StatusDisplayer.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <queue>
#include <random>
#include <unistd.h>
#include <pthread.h>

extern bool runThreads;
extern bool loggingEnabled;
extern std::ofstream logFile;
namespace Delivery
{
  /* Constants and parameters*/
  const int assemblingTime = 3; // Time to get all order items
  // Average time to make a burger and a fry. Used to prioritize tasks
  double avgTimePerBurger, avgTimePerFries;
  int maxWorkers; // Number of workers that can be delivering at the same time

  /* Delivery and order control */
  pthread_mutex_t orderMakingMutex = PTHREAD_MUTEX_INITIALIZER; // Just to use the condition
  pthread_cond_t waitForOrderDelivered = PTHREAD_COND_INITIALIZER; // Sleep until order delivered
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Exclusive access to variables below
  int workers = 0; // People working at this instant in this station

  /* Orders queue */
  int totalFries = 0; // Total fries that are in the queue. Helps prioritize
  int totalBurgers = 0; // Total burgers that are in the queue. Helps prioritize
  std::queue<order_t> ordersQueue; // Orders to make
}

std::vector<pthread_t> Delivery::initDelivery(int nDelivery, int nCustomers)
{
  // Calculate the average times. Helps prioritize
  avgTimePerBurger = (double)Griddle::griddlingTime / (double)Griddle::burgersPerBatch;
  avgTimePerBurger += (double)AssemblyStation::assemblingTime / (double)AssemblyStation::burgersPerBatch;

  avgTimePerFries = (double)DeepFriers::setupTime;
  avgTimePerFries += (double)DeepFriers::fryTime;
  avgTimePerFries /= (double)DeepFriers::friesPerBatch;
  avgTimePerFries *= (double)SaltingStation::friesPerPortion;
  avgTimePerFries += (double)SaltingStation::saltingTime;

  // Sets a limit of workers
  maxWorkers = nDelivery;

  // Help workers think of an order
  srand(time(NULL));

  // Start customer threads
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

  // Gets access to 2 stations with the ready items and this station's variables
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&SaltingStation::mutex);
  pthread_mutex_lock(&AssemblyStation::mutex);
  if (ordersQueue.size() > 0)
  {

    front = ordersQueue.front();

    // Check if the first order can be delivered
    if (workers < maxWorkers && SaltingStation::fries >= front.fries && AssemblyStation::burgers >= front.burgers)
    {
      ++workers; // Start working

      // Show that the order is in assembly and needs no more attention
      ordersQueue.pop();
      // Get resources
      AssemblyStation::burgers -= front.burgers;
      SaltingStation::fries -= front.fries;

      // Keeps track of ordered
      totalBurgers -= front.burgers;
      totalFries -= front.fries;

      // Update UI
      statusDisplayer->updateDeliveryWorkers(workers);
      statusDisplayer->updateAssemblyStationBurgers(AssemblyStation::burgers);
      statusDisplayer->updateSaltingFries(SaltingStation::fries);
      statusDisplayer->updateDeliveryOrdered(totalFries, totalBurgers);
      statusDisplayer->updateDeliveryFirstOrder(ordersQueue);
      canDo = true;
    }
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&SaltingStation::mutex);
  pthread_mutex_unlock(&AssemblyStation::mutex);

  if (!canDo)
    return false;

  // Assemble the order
  sleep(assemblingTime);

  // Deliver the order and
  // allow one customer to order more.
  pthread_cond_signal(&waitForOrderDelivered);

  pthread_mutex_lock(&mutex);
  --workers; // Stop working here

  statusDisplayer->updateDeliveryWorkers(workers); // Update UI
  pthread_mutex_unlock(&mutex);

  // Tell the workers about potential tasks
  Worker::broadcastAvailableTasks();
  return true;
}

void Delivery::placeOrder(order_t order)
{
  pthread_mutex_lock(&mutex); // exclusive access to the queue

  // Place order
  ordersQueue.push(order);

  // Update amounts ordered in total
  totalBurgers += order.burgers;
  totalFries += order.fries;

  // update UI
  if (ordersQueue.size() == 1)
    statusDisplayer->updateDeliveryFirstOrder(ordersQueue);
  statusDisplayer->updateDeliveryOrdered(totalFries, totalBurgers);

  pthread_mutex_unlock(&mutex);

  // Signal about possible new tasks
  Worker::broadcastAvailableTasks();
}

void *Delivery::Customer(void *args)
{
  order_t order;
  std::ostringstream out;
  int id = *(int *)args;
  delete (int *)args;

  out << "Customer of id " << id << " instantiated" << std::endl;
  if (loggingEnabled)
    logFile << out.str();
  out.str("");

  // Until the program stops
  while (runThreads)
  {
    /* Think of an order */
    order.burgers = 0;
    order.fries = 0;
    while (order.burgers <= 0 && order.fries <= 0)
    {
      order.burgers = random() % 6;
      order.fries = std::max(order.burgers + ((int)random() % 5) - 2, 0);
    }

    // Place the order in queue
    placeOrder(order);

    out << "Customer[" << id << "]: Pedi " << order.fries << " fritas e " << order.burgers << " hambúrgueres." << std::endl;
    if (loggingEnabled)
      logFile << out.str();
    out.str("");

    // The customer waits for a delivery to be made before ordering more.
    pthread_mutex_lock(&orderMakingMutex);
    pthread_cond_wait(&waitForOrderDelivered, &orderMakingMutex);
    pthread_mutex_unlock(&orderMakingMutex);
  }

  return nullptr;
}

std::vector<Delivery::priority_type_t> Delivery::getPriority()
{
  std::vector<priority_type_t> priorities;
  order_t front;
  int fries, burgers, neededFriesForOrder, neededBurgersForOrder;

  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&SaltingStation::mutex);
  pthread_mutex_lock(&AssemblyStation::mutex);

  /* If there's nothing to be done, make workers wait */
  if (totalBurgers != 0 || totalFries != 0)
  {
    // Gather data
    burgers = AssemblyStation::burgers;
    fries = SaltingStation::fries;
    front = ordersQueue.front();
    neededBurgersForOrder = std::max(0, front.burgers - burgers);
    neededFriesForOrder = std::max(0, front.fries - fries);

    // If can deliver
    if (neededBurgersForOrder == 0 && neededBurgersForOrder == 0)
    {
      priorities.push_back(DELIVERY); // try to deliver first

      // Then make the one that takes more time to do (burgers or fries)
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
      // Make the one that takes more time to do (burgers or fries)
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
      // Then try to deliver
      priorities.push_back(DELIVERY);
    }
  }

  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&SaltingStation::mutex);
  pthread_mutex_unlock(&AssemblyStation::mutex);

  return priorities;
}
