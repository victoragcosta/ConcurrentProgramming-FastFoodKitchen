#include "Delivery.hpp"
#include "AssemblyStation.hpp"
#include "Fries.hpp"

#include <pthread.h>
#include <queue>
#include <unistd.h>

namespace Delivery
{
  /* Constants */
  const int assemblingTime = 3;
  int maxWorkers;

  /* Delivery and order control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int workers = 0;

  /* Orders queue */
  typedef struct {
    int fries;
    int burgers;
  } order_t;

  std::queue<order_t> ordersQueue;
}

void Delivery::initDelivery(int nDelivery)
{
  maxWorkers = nDelivery;
}

bool Delivery::deliverOrder()
{
  bool canDo = false;
  order_t front;

  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&Fries::Salting::mutex);
  pthread_mutex_lock(&AssemblyStation::mutex);
  front = ordersQueue.front();

  if(workers < maxWorkers
    && Fries::Salting::portions >= front.fries
    && AssemblyStation::burgers >= front.burgers) {
    ordersQueue.pop();
    workers++;
    Fries::Salting::portions -= front.fries;
    AssemblyStation::burgers -= front.burgers;
    canDo = true;
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&Fries::Salting::mutex);
  pthread_mutex_unlock(&AssemblyStation::mutex);

  if(!canDo)
    return false;

  sleep(assemblingTime);

  pthread_mutex_lock(&mutex);
  workers--;
  pthread_mutex_unlock(&mutex);

  return true;
}
