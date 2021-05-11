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
  /* Constants and parameters */
  // Time to make a fries serving
  // Amount of unsalted fries it takes to make a serving
  const int saltingTime = 3, friesPerPortion = 40;
  int maxWorkers; // Number of worker spots

  /* Salting control */
  // Exclusive access to the 2 variables below
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int fries = 0; // Number of fries servings
  int workers = 0; // number of workers working in this station
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
    workers++; // Start working
    canDo = true;
    statusDisplayer->updateSaltingWorkers(workers); // Update UI
  }
  pthread_mutex_unlock(&mutex);

  if (!canDo)
    return false;

  sleep(saltingTime);

  pthread_mutex_lock(&mutex);
  --workers; // Stops working
  fries++; // Creates a portion

  // Update UI
  statusDisplayer->updateSaltingWorkers(workers);
  statusDisplayer->updateSaltingFries(fries);
  pthread_mutex_unlock(&mutex);

  // Show that a task may be available
  Worker::broadcastAvailableTasks();
  return true;
}

bool SaltingStation::getPortions(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  // Checks if there are enough fries
  if (fries >= n)
  {
    canDo = true;
    fries -= n; // Gets them
    statusDisplayer->updateSaltingFries(fries); // Update UI
  }
  pthread_mutex_unlock(&mutex);
  return canDo;
}
