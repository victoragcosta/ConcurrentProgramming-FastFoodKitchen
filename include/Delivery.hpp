#ifndef DELIVERY_HPP_
#define DELIVERY_HPP_

#include <vector>
#include <pthread.h>

namespace Delivery
{
  extern const int assemblingTime;

  extern pthread_cond_t waitForOrderDelivered;

  // Initializes Customer threads preparing for the simulation and set some parameters.
  // Returns a vector with all threads created.
  std::vector<pthread_t> initDelivery(int nDelivery, int nCustomers);

  // Stores an order of fries and burgers.
  typedef struct
  {
    int fries;
    int burgers;
  } order_t;

  // Attempts to deliver an order. If possible, subtracts all resources needed,
  // waits assemblingTime and then returns true. If not possible, return false.
  // Wakes up a Customer thread.
  bool deliverOrder();

  // Places an order in the queue.
  void placeOrder(order_t order);

  // Thread that will think of an order, place the order, wait for the delivery
  // and then repeat.
  void *Customer(void *args);

  // Enumerates tasks that the Worker threads can prioritize.
  typedef enum
  {
    FRIES,
    BURGERS,
    DELIVERY,
  } priority_type_t;

  // Returns a vector with all the tasks that the Worker should try to do by
  // order of priority.
  std::vector<priority_type_t> getPriority();
}

#endif /* DELIVERY_HPP_ */
