#include "SaltingStation.hpp"
#include "DeepFrier.hpp"
#include "Worker.hpp"
#include "StatusDisplayer.hpp"

#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>

extern bool runThreads;
extern bool loggingEnabled;
extern std::ofstream logFile;
namespace SaltingStation
{
  /* Constants */
  const int saltingTime = 3, friesPerPortion = 40;
  int maxWorkers;

  /* Salting control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int fries = 0;
  int workers = 0;
}



void SaltingStation::initSaltingStations(int nSalters)
{
  maxWorkers = nSalters;
}

bool SaltingStation::saltFries()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  /*
    Tries to make a portion.
    If successful the unsalted fries are already subtracted in this function call.
  */
  if (workers < maxWorkers && DeepFriers::getUnsaltedFries(friesPerPortion))
  {
    workers++;
    fries++;
    canDo = true;
    statusDisplayer->updateSaltingFries(fries);
    statusDisplayer->updateSaltingWorkers(workers);
  }
  pthread_mutex_unlock(&mutex);

  if (!canDo)
    return false;

  sleep(saltingTime);

  pthread_mutex_lock(&mutex);
  --workers;
  statusDisplayer->updateSaltingWorkers(workers);
  pthread_mutex_unlock(&mutex);

  Worker::broadcastAvailableTasks();
  return true;
}

bool SaltingStation::getPortions(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if (fries >= n)
  {
    canDo = true;
    fries -= n;
    statusDisplayer->updateSaltingFries(fries);
  }
  pthread_mutex_unlock(&mutex);
  return canDo;
}
