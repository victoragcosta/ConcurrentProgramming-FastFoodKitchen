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

namespace DeepFriers
{
  /* Constants and parameters */
  // Time to start the DeepFrier
  // Time to fry fries
  // Amount of fries generated
  const int setupTime = 2, fryTime = 5, friesPerBatch = 120;
  int maxFriers; // Number of DeepFriers in the restaurant

  /* Deep friers cycle control */
  // Protects available and requested from non-exclusive access
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  // Allows the DeepFrier to sleep while waiting for someone to set it up
  pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
  int available = 0; // Helps workers know if there are free deep friers
  int requested = 0; // Helps signal deep friers to work

  /* Unsalted fries control */

  // Protects unsaltedFries from non-exclusive access
  pthread_mutex_t mutexUnsaltedFries = PTHREAD_MUTEX_INITIALIZER;
  int unsaltedFries = 0; // Keeps track of how many unsalted fries are available
}

std::vector<pthread_t> DeepFriers::initDeepFriers(int nDeepFriers)
{
  /* Setup constants */
  maxFriers = nDeepFriers;

  /* Start Threads */
  pthread_t thread;
  int *id;
  std::vector<pthread_t> threads;

  for (int i = 0; i < nDeepFriers; i++)
  {
    id = new int;
    *id = i;
    pthread_create(&thread, NULL, DeepFrier, (void *)id);
    threads.push_back(thread);
  }

  return threads;
}

void *DeepFriers::DeepFrier(void *args)
{
  int id = *(int *)args;
  delete (int *)args; // Prevent memory leaks

  std::ostringstream out;
  out << "Deep frier of id " << id << " instantiated" << std::endl;
  if (loggingEnabled)
    logFile << out.str();
  out.str("");

  while (runThreads)
  {
    pthread_mutex_lock(&mutex); // exclusive access to requested and available variables
    /* Show that the deep frier is free */
    if (++available == 1)
      Worker::broadcastAvailableTasks();

    statusDisplayer->updateDeepFrierAvailable(available); // update the UI

    out << "DeepFrier[" << id << "]: Estou pronto." << std::endl;
    if (loggingEnabled)
      logFile << out.str();
    out.str("");

    /* Wait for a request to fry sleeping */
    while (requested <= 0 && runThreads)
      pthread_cond_wait(&condition, &mutex);
    requested--; // Shows that it is taking care of the frying
    pthread_mutex_unlock(&mutex);

    // If closing the program, simply stop
    if (!runThreads)
      break;

    /* Fry */
    sleep(fryTime);

    pthread_mutex_lock(&mutexUnsaltedFries); // exclusive access to unsalted fries
    /* Add unsalted fries */
    unsaltedFries += friesPerBatch;

    out << "DeepFrier[" << id << "]: Fritei. Agora temos " << unsaltedFries << " batatas." << std::endl;
    if (loggingEnabled)
      logFile << out.str();
    out.str("");

    // Signals workers about new tasks that can be done
    Worker::broadcastAvailableTasks();

    statusDisplayer->updateDeepFrierUnsaltedFries(unsaltedFries); // update the UI
    pthread_mutex_unlock(&mutexUnsaltedFries);
  }
  return nullptr;
}

bool DeepFriers::setupDeepFrier()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&mutexUnsaltedFries);
  // Checks if there are DeepFriers and not filled the unsalted fries storage.
  if (available > 0 && unsaltedFries + (1 + maxFriers - available) * friesPerBatch <= 240)
  {
    canDo = true; // Signal that the task can be done
    available--; // Use a DeepFrier
    statusDisplayer->updateDeepFrierAvailable(available); // update the UI
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&mutexUnsaltedFries);

  // If not possible to do, do nothing
  if (!canDo)
    return false;

  // Setup DeepFrier
  sleep(setupTime);

  pthread_mutex_lock(&mutex);
  requested++; // Request the work of a DeepFrier
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&condition); // Wake up a DeepFrier to fry

  return true;
}

bool DeepFriers::getUnsaltedFries(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutexUnsaltedFries);
  if (unsaltedFries >= n)
  {
    canDo = true;
    unsaltedFries -= n;
    statusDisplayer->updateDeepFrierUnsaltedFries(unsaltedFries);
  }
  pthread_mutex_unlock(&mutexUnsaltedFries);
  return canDo;
}
