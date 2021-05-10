#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <csignal>
#include <pthread.h> // Adds locks, conds and threads
// #include <semaphore.h> // May be needed

#include "SaltingStation.hpp"
#include "DeepFrier.hpp"
#include "Griddle.hpp"
#include "AssemblyStation.hpp"
#include "Delivery.hpp"
#include "Worker.hpp"
#include "StatusDisplayer.hpp"

using namespace std;

bool runThreads = true;
vector<pthread_t> threads;
ofstream logFile;
bool loggingEnabled = false;
StatusDisplayer *statusDisplayer = nullptr;

void signalHandler(int signum);

int main(int argc, char *argv[])
{
  vector<pthread_t> aux;
  vector<string> args(argv + 1, argv + argc);

  const int nWorkers = 6;
  const int nCustomers = 2;
  const int nDelivery = 2;
  const int nDeepFriers = 2;
  const int nSalters = 2;
  const int nAssemblyStations = 2;
  const int nGriddles = 2;

  loggingEnabled |= find(args.begin(), args.end(), "--logging") != args.end();
  loggingEnabled |= find(args.begin(), args.end(), "-l") != args.end();

  if (loggingEnabled)
    logFile.open("./log.txt", ios::trunc | ios::out);

  signal(SIGINT, signalHandler);
  signal(SIGSEGV, signalHandler);

  statusDisplayer = new StatusDisplayer(nWorkers, nCustomers, nDelivery, nDeepFriers, nSalters, nAssemblyStations, nGriddles);

  AssemblyStation::initAssemblyStations(nAssemblyStations);
  Griddle::initGriddle(nGriddles);
  SaltingStation::initSaltingStations(nSalters);

  aux = DeepFriers::initDeepFriers(nDeepFriers);
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

int numCallsSigint = 0;
void signalHandler(int signum)
{
  if (signum == SIGINT) {
    numCallsSigint++;
    if(numCallsSigint > 1)
      exit(signum);
  }

  if (loggingEnabled)
    logFile.close();

  if (signum == SIGSEGV)
  {
    cout << "Falha de segmentação" << endl;
    statusDisplayer->displayStatus();
    if (statusDisplayer != nullptr)
      delete statusDisplayer;
    exit(signum);
  }

  cout << "Parando threads.\n";
  runThreads = false;

  DeepFriers::setupDeepFrier();
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

  printf("Finalizando programa.\n");
  if (signum == SIGINT)
    exit(0);
  exit(signum);
}
