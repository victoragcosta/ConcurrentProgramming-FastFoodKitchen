#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <pthread.h> // Adds locks, conds and threads
// #include <semaphore.h> // May be needed

#include "Fries.hpp"

using namespace std;

int main(int argc, char *argv[])
{
  vector<pthread_t> threads, aux;

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
