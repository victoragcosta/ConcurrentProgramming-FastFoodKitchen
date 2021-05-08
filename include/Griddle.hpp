#ifndef GRIDDLE_HPP_
#define GRIDDLE_HPP_

namespace Griddle
{
  extern const int griddlingTime;
  extern const int burgersPerBatch;

  void initGriddle(int nGriddles);

  bool makeBurgerMeats();

  bool getBurgerMeats(int n);
}

#endif /* GRIDDLE_HPP_ */
