#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <pthread.h> // Adds locks, conds and threads
// #include <semaphore.h> // May be needed

#include "Fries.hpp"
#include "Griddle.hpp"
#include "AssemblyStation.hpp"
#include "Delivery.hpp"

using namespace std;

int main(int argc, char *argv[])
{
  vector<pthread_t> threads, aux;

  AssemblyStation::initAssemblyStations(2);
  Griddle::initGriddle(2);

  aux = Delivery::initDelivery(2, 2);
  threads.insert(threads.end(), aux.begin(), aux.end());
  aux = Fries::initFries(2, 2);
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
