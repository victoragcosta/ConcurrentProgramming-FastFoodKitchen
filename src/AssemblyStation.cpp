#include "AssemblyStation.hpp"
#include "Griddle.hpp"

#include <pthread.h>
#include <unistd.h>

namespace AssemblyStation
{
  /* Constants */
  const int assemblingTime = 3;
  const int burgersPerBatch = 1;
  int maxWorkers;

  /* Assembling control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int burgers = 0;
  int workers = 0;
}

void AssemblyStation::initAssemblyStations(int nAssemblyStations)
{
  maxWorkers = nAssemblyStations;
}

bool AssemblyStation::makeBurgers()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if(workers < maxWorkers && Griddle::getBurgerMeats(burgersPerBatch)) {
    workers++;
    canDo = true;
  }
  pthread_mutex_unlock(&mutex);

  if(!canDo)
    return false;

  sleep(assemblingTime);
  pthread_mutex_lock(&mutex);
  workers--;
  burgers += burgersPerBatch;
  pthread_mutex_unlock(&mutex);

  return true;
}

bool AssemblyStation::getBurgers(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if(burgers >= n) {
    burgers -= n;
    canDo = true;
  }
  pthread_mutex_unlock(&mutex);

  if(!canDo)
    return false;

  return true;
}
