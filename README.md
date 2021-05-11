# Concurrent Programming - Fast Food Kitchen

This is a problem invented by @victoragcosta to complete his final assignment in
the concurrent programming class in Universidade de Brasília. This problem tries
to simulate the concurrency of completing tasks in a generic fast food restaurant.

## The problem

There is a fast food restaurant that sells burgers and french fries. This
restaurant has many workers capable of doing everything in the kitchen, but only
one task at a time. This kitchen has a griddle, on which you can fry many burgers'
meat, but it requires the worker's full attention while you do so. This kitchen
has a burger assembly station as well, on which the worker can assemble one burger
at a time and requiring all of their attention. This kitchen also has a deep frier,
in which you can fry many fries and get some unsalted fries that you must salt
before serving. The deep frier has no need to be watched, since it has a timer,
therefore the worker can do more tasks while waiting. Lastly, but not least, this
kitchen has a counter for delivering the orders of the customers, on which the
worker can deliver one order at a time and requiring the worker's full attention
while doing so.

The customers can order any reasonable number of french fries and burgers and it
automatically goes to the order queue (using some fancy apps).

## Compiling and running

Simply execute the comand `make` at the root of the project and it will build
automatically. The binary will be created at `bin/fast-food-kitchen.out` and you
may execute it. You may add `-l` or `--logging` to create a `log.txt` file in the
current work directory. This log will contain messages from all threads indicating
what they are doing.

## Repository

https://github.com/victoragcosta/ConcurrentProgramming-FastFoodKitchen
