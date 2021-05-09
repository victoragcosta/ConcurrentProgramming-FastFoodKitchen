#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <csignal>
#include <pthread.h> // Adds locks, conds and threads
// #include <semaphore.h> // May be needed

#include "Fries.hpp"
#include "Griddle.hpp"
#include "AssemblyStation.hpp"
#include "Delivery.hpp"
#include "Worker.hpp"
#include "StatusDisplayer.hpp"

using namespace std;

bool runThreads = true;
vector<pthread_t> threads;
ostringstream logString;
StatusDisplayer *statusDisplayer = nullptr;

void signalHandler(int signum)
{
  cout << "Parando threads.\n";
  runThreads = false;

  Fries::DeepFriers::setupDeepFrier();
  pthread_cond_broadcast(&Delivery::waitForOrderDelivered);
  Worker::broadcastAvailableTasks();

  for (size_t i = 0; i < threads.size(); i++)
  {
    cout << "Parando thread " << i << "/" << threads.size() << "." << endl;
    pthread_cancel(threads[i]);
    if (pthread_join(threads[i], NULL))
    {
      printf("\n ERROR joining thread");
      exit(1);
    }
    cout << i << "/" << threads.size() << " threads paradas." << endl;
  }

  if (statusDisplayer != nullptr)
    delete statusDisplayer;

  cout << "==================================================" << endl;
  cout << logString.str() << endl;
  cout << "==================================================" << endl;

  printf("Finalizando programa.\n");
  if (signum == SIGINT)
    exit(0);
  exit(signum);
}

int main(int argc, char *argv[])
{
  vector<pthread_t> aux;

  const int nWorkers = 6;
  const int nCustomers = 2;
  const int nDelivery = 2;
  const int nDeepFriers = 2;
  const int nSalters = 2;
  const int nAssemblyStations = 2;
  const int nGriddles = 2;

  signal(SIGINT, signalHandler);

  statusDisplayer = new StatusDisplayer(nWorkers, nCustomers, nDelivery, nDeepFriers, nSalters, nAssemblyStations, nGriddles);

  AssemblyStation::initAssemblyStations(nAssemblyStations);
  Griddle::initGriddle(nGriddles);

  aux = Fries::initFries(nDeepFriers, nSalters);
  threads.insert(threads.end(), aux.begin(), aux.end());

  aux = Worker::initWorkers(nWorkers);
  threads.insert(threads.end(), aux.begin(), aux.end());

  aux = Delivery::initDelivery(nDelivery, nCustomers);
  threads.insert(threads.end(), aux.begin(), aux.end());

  for (size_t i = 0; i < threads.size(); i++)
  {
    if (pthread_join(threads[i], NULL))
    {
      printf("\n ERROR joining thread");
      exit(1);
    }
  }
  printf("Finalizando programa.\n");
  return 0;
}
