#ifndef DELIVERY_HPP_
#define DELIVERY_HPP_

#include <vector>
#include <pthread.h>

namespace Delivery
{
  std::vector<pthread_t> initDelivery(int nDelivery, int nCustomers);

  typedef struct
  {
    int fries;
    int burgers;
  } order_t;

  bool deliverOrder();

  void placeOrder(order_t order);

  void *Customer(void *args);

  typedef enum
  {
    FRIES,
    BURGERS,
    DELIVERY,
  } priority_type_t;

  std::vector<priority_type_t> getPriority();
}

#endif /* DELIVERY_HPP_ */
