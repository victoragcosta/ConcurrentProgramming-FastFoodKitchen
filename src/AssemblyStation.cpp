#include "AssemblyStation.hpp"
#include "Griddle.hpp"
#include "Worker.hpp"
#include "StatusDisplayer.hpp"

#include <pthread.h>
#include <unistd.h>

namespace AssemblyStation
{
  /* Constants and parameters */
  const int assemblingTime = 3; // time to assemble a burger.
  const int burgersPerBatch = 1; // number of burgers created after that time.
  int maxWorkers; // number of workers that can work on this station at the same time.

  /* Assembling control */
  // Protects the burgers and workers variables from non-exclusive access.
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int burgers = 0; // counts how many burgers are completed.
  int workers = 0; // keeps track of the number of workers in the station.
}

void AssemblyStation::initAssemblyStations(int nAssemblyStations)
{
  maxWorkers = nAssemblyStations;
}

bool AssemblyStation::makeBurgers()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex); // exclusive access to workers variable
  // Checks if there is space for one more worker and if there are enough burger
  // meats to do the job.
  if (workers < maxWorkers && Griddle::getBurgerMeats(burgersPerBatch))
  {
    // Subtracts burgers in the condition
    statusDisplayer->updateAssemblyStationWorkers(++workers); // Update the UI
    canDo = true; // Signals that the task can be done
  }
  pthread_mutex_unlock(&mutex);

  // If the task can't be done, do nothing
  if (!canDo)
    return false;

  // Do the task
  sleep(assemblingTime);

  pthread_mutex_lock(&mutex); // exclusive access to workers and burgers variables
  burgers += burgersPerBatch; // Adds the produced burgers

  // update the UI and signals that the worker has stopped
  statusDisplayer->updateAssemblyStationWorkers(--workers);
  statusDisplayer->updateAssemblyStationBurgers(burgers);
  pthread_mutex_unlock(&mutex);

  // Signals that a task may be available to do.
  Worker::broadcastAvailableTasks();
  return true;
}

bool AssemblyStation::getBurgers(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex); // Exclusive access to burgers variable
  if (burgers >= n) // Checks if there are enough burgers
  {
    // There are enough burgers
    burgers -= n; // Remove them
    statusDisplayer->updateAssemblyStationBurgers(burgers); // Update the UI
    canDo = true; // Signal that it is possible to take burgers
  }
  pthread_mutex_unlock(&mutex);

  // Tell if the burgers were taken
  if (!canDo)
    return false;

  return true;
}
