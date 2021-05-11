#include "Griddle.hpp"
#include "Worker.hpp"
#include "AssemblyStation.hpp"
#include "StatusDisplayer.hpp"

#include <pthread.h>
#include <unistd.h>

namespace Griddle
{
  /* Constants and parameters */
  const int griddlingTime = 7; // Time to make burgers' meats
  const int burgersPerBatch = 20; // Amount of burgers' meats griddled each time
  int maxWorkers; // Max workers in this station

  /* Griddling control */
  // Exclusive access to burgersMeat and workers
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  int burgersMeat = 0; // Number of burger meats ready to be used
  int workers = 0; // number of workers in the station
}

void Griddle::initGriddle(int nGriddles)
{
  maxWorkers = nGriddles;
}

bool Griddle::makeBurgerMeats()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  // Check if there is a spot for the worker and if there's storage for the burgers' meat
  if (workers < maxWorkers && burgersMeat + (workers + 1) * burgersPerBatch <= 40)
  {
    workers++; // Signal that there's a new worker
    canDo = true; // Signal that it can be done
    statusDisplayer->updateGriddleWorkers(workers); // update the UI
  }
  pthread_mutex_unlock(&mutex);

  // Do nothing if task undoable
  if (!canDo)
    return false;

  // Griddle some burgers
  sleep(griddlingTime);

  pthread_mutex_lock(&mutex);
  --workers; // stop working
  burgersMeat += burgersPerBatch; // Add burgers produced

  // Update the UI
  statusDisplayer->updateGriddleWorkers(workers);
  statusDisplayer->updateGriddleBurgersMeat(burgersMeat);
  pthread_mutex_unlock(&mutex);

  // Signal that a task may be available
  Worker::broadcastAvailableTasks();

  return true;
}

bool Griddle::getBurgerMeats(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  // Check if there are enough burgers' meats
  if (burgersMeat >= n)
  {
    // There are
    burgersMeat -= n; // Remove them
    canDo = true; // Signal that it has been removed
    statusDisplayer->updateGriddleBurgersMeat(burgersMeat); // Update the ui
  }
  pthread_mutex_unlock(&mutex);

  // Tell the caller what happened
  if (!canDo)
    return false;

  return true;
}
