#include "Fries.hpp"
#include <unistd.h>
#include <sstream>
#include <iostream>

namespace Fries
{
  namespace DeepFriers
  {
    /* Constants */
    int setupTime, fryTime, friesPerBatch;

    /* Deep friers cycle control */
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
    int available = 0;  //!< Helps workers know if there are free deep friers
    int requested = 0;  //!< Helps signal deep friers to work

    /* Unsalted fries control */
    pthread_mutex_t mutexUnsaltedFries = PTHREAD_MUTEX_INITIALIZER;
    int unsaltedFries = 0;
  }

  namespace Salting
  {
    /* Constants */
    int saltingTime, maxWorkers, friesPerPortion;

    /* Salting control */
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int portions = 0;
    int workers = 0;
  }
}

std::vector<pthread_t> Fries::initFries(int nDeepFriers, int nSalters)
{
  /* Setup constants */
  DeepFriers::setupTime = 2;
  DeepFriers::fryTime = 5;
  DeepFriers::friesPerBatch = 120;
  Salting::saltingTime = 3;
  Salting::friesPerPortion = 40;

  Salting::maxWorkers = 2;

  /* Start Threads */
  pthread_t thread;
  int *id;
  std::vector<pthread_t> threads;

  for (int i = 0; i < nDeepFriers; i++)
  {
    id = new int;
    *id = i;
    pthread_create(&thread, NULL, DeepFriers::DeepFrier, (void *)id);
    threads.push_back(thread);
  }

  return threads;
}


void* Fries::DeepFriers::DeepFrier(void* args)
{
  int id = *(int*)args;
  delete (int*)args; // Prevent memory leaks

  std::stringstream out;
  out << "Deep frier of id " << id << " instantiated" << std::endl;
  std::cout << out.str();
  out.str("");

  while (true)
  {
    pthread_mutex_lock(&mutex);
    /* Show that the deep frier is free */
    available++;
    /* Wait for a request */
    while (requested <= 0)
      pthread_cond_wait(&condition, &mutex);
    requested--;
    pthread_mutex_unlock(&mutex);

    /* Fry */
    sleep(fryTime);

    /* Add unsalted fries */
    pthread_mutex_lock(&mutex);
    unsaltedFries += friesPerBatch;
    pthread_mutex_lock(&mutex);
  }
}

bool Fries::DeepFriers::setupDeepFrier()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if(available > 0) {
    canDo = true;
    available--;
  }
  pthread_mutex_unlock(&mutex);

  if(!canDo)
    return false;

  sleep(setupTime);
  pthread_mutex_lock(&mutex);
  requested++;
  pthread_mutex_unlock(&mutex);

  return true;
}

bool Fries::DeepFriers::getUnsaltedFries(int n)
{
  bool canDo = false;
  pthread_mutex_lock(&mutexUnsaltedFries);
  if(unsaltedFries >= n){
    canDo = true;
    unsaltedFries -= n;
  }
  pthread_mutex_unlock(&mutexUnsaltedFries);
  return canDo;
}


bool Fries::Salting::saltFries()
{
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  /*
    Tries to make a portion.
    If successful the unsalted fries are already subtracted in this function call.
  */
  if(workers < maxWorkers && DeepFriers::getUnsaltedFries(friesPerPortion)){
    portions++;
    canDo = true;
  }
  pthread_mutex_unlock(&mutex);

  if(!canDo)
    return false;

  sleep(saltingTime);

  return true;
}

bool Fries::Salting::getPortions(int n) {
  bool canDo = false;
  pthread_mutex_lock(&mutex);
  if(portions >= n){
    canDo = true;
    portions -= n;
  }
  pthread_mutex_unlock(&mutex);
  return canDo;
}
