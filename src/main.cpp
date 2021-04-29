#include <iostream>
#include <iomanip>
#include <string>
#include <pthread.h> // Adds locks, conds and threads
// #include <semaphore.h> // May be needed

using namespace std;

void *Thread(void *arg) {
  int id = *(int *)arg;
  cout << "Meu id é " + to_string(id) + "\n";
  return arg;
}

int main(int argc, char *argv[])
{
  const int size = 10;

  pthread_t threads[size];

  int *id;
  for (int i = 0; i < size; i++)
  {
    id = (int *)malloc(sizeof(int));
    *id = i;
    pthread_create(&threads[i], NULL, Thread, (void *)id);
  }

  for (int i = 0; i < size; i++)
  {
    if (pthread_join(threads[i], NULL))
    {
      printf("\n ERROR joining thread");
      exit(1);
    }
  }
  printf("Bye!\n");
  return 0;
}
