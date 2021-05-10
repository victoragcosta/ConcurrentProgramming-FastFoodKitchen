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
  /* Constants */
  const int setupTime = 2, fryTime = 5, friesPerBatch = 120;
  int maxFriers;

  /* Deep friers cycle control */
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
  int available = 0; //!< Helps workers know if there are free deep friers
  int requested = 0; //!< Helps signal deep friers to work

  /* Unsalted fries control */
  pthread_mutex_t mutexUnsaltedFries = PTHREAD_MUTEX_INITIALIZER;
  int unsaltedFries = 0;
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
    pthread_mutex_lock(&mutex);
    /* Show that the deep frier is free */
    if (++available == 1)
      Worker::broadcastAvailableTasks();

    statusDisplayer->updateDeepFrierAvailable(available);

    out << "DeepFrier[" << id << "]: Estou pronto." << std::endl;
    if (loggingEnabled)
      logFile << out.str();
    out.str("");

    /* Wait for a request */
    while (requested <= 0 && runThreads)
      pthread_cond_wait(&condition, &mutex);
    requested--;
    pthread_mutex_unlock(&mutex);

    if (!runThreads)
      break;

    /* Fry */
    sleep(fryTime);

    /* Add unsalted fries */
    pthread_mutex_lock(&mutex);
    unsaltedFries += friesPerBatch;
    out << "DeepFrier[" << id << "]: Fritei. Agora temos " << unsaltedFries << " batatas." << std::endl;
    if (loggingEnabled)
      logFile << out.str();
    out.str("");
    if (unsaltedFries >= SaltingStation::friesPerPortion)
      Worker::broadcastAvailableTasks();

    statusDisplayer->updateDeepFrierUnsaltedFries(unsaltedFries);
    pthread_mutex_unlock(&mutex);
  }
  return nullptr;
}

bool DeepFriers::setupDeepFrier()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&mutexUnsaltedFries);
  if (available > 0 && unsaltedFries + (1 + maxFriers - available) * friesPerBatch <= 240)
  {
    canDo = true;
    available--;
    statusDisplayer->updateDeepFrierAvailable(available);
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&mutexUnsaltedFries);

  if (!canDo)
    return false;

  sleep(setupTime);
  pthread_mutex_lock(&mutex);
  requested++;
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&condition);

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
