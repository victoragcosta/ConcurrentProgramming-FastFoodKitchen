#include "Griddle.hpp"
#include "Worker.hpp"
#include "AssemblyStation.hpp"
#include "StatusDisplayer.hpp"

#include <pthread.h>
#include <unistd.h>

namespace Griddle
{
  /* Constants */
  const int griddlingTime = 7;
  const int burgersPerBatch = 20;
  int maxWorkers;

  /* Griddling control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int burgersMeat = 0;
  int workers = 0;
}

void Griddle::initGriddle(int nGriddles)
{
  maxWorkers = nGriddles;
}

bool Griddle::makeBurgerMeats()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if (workers < maxWorkers && burgersMeat + (workers + 1) * burgersPerBatch <= 40)
  {
    workers++;
    canDo = true;
    statusDisplayer->updateGriddleWorkers(workers);
  }
  pthread_mutex_unlock(&mutex);

  if (!canDo)
    return false;

  sleep(griddlingTime);

  pthread_mutex_lock(&mutex);
  if (--workers == maxWorkers - 1)
    Worker::broadcastAvailableTasks();
  burgersMeat += burgersPerBatch;
  if (burgersMeat > AssemblyStation::burgersPerBatch)
    Worker::broadcastAvailableTasks();
  statusDisplayer->updateGriddleWorkers(workers);
  statusDisplayer->updateGriddleBurgersMeat(burgersMeat);
  pthread_mutex_unlock(&mutex);

  return true;
}

bool Griddle::getBurgerMeats(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if (burgersMeat >= n)
  {
    burgersMeat -= n;
    canDo = true;
    statusDisplayer->updateGriddleBurgersMeat(burgersMeat);
  }
  pthread_mutex_unlock(&mutex);

  if (!canDo)
    return false;

  return true;
}
