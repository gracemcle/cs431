## Project 3 Example Codebook

In this codebook, you will be learning about scheduling pthreads using mutex locks from the pthreads library.

### how_sched.c

The first thing I want to show is how threads are scheduled to get access to locked mutexes.
**Key Points**:

- When multiple threads are waiting for the same mutex, there is no guaranteed order for which thread will acquire the lock when it becomes available
- Pthread mutexes get their functionality from the Linux futex (Fast Userspace Mutex) system call
- The wake-up order depends on the kernel scheduler and many other factors including:
  - Thread priorities and "nice" values
  - CPU usage history (virtual runtime in CFS scheduler)
  - Multi-core load balancing
  - Timing of when threads block on the mutex

**What this means for your code:**
- Never assume threads will acquire a mutex in the order they requested it
- If you need specific ordering, implement it at the application level using condition variables or other synchronization primitives
- Even threads with identical priorities can be awakened in unpredictable order

Try running this and see what happens!

#### Exercise 1 black_friday.c :
In the old days of black friday, all order is lost. What was once order is now complete chaos. People are lined up waiting to get into the store by any means necessary.
Queues and reason are abandoned. When all of the shoppers arrive, they are all strong enough to break through the door. 
A single shop is selling TV's at some sort of fantastic sale. There are enough TV's for everyone. However, the sale price of the TV diminishes as each TV is bought. 
That is, the price of the TV will increase. 
Please implement using pthreads:
    - spawn *n* shopper threads
    - create a (`barrier`) to hold the shoppers back from entering the store
    - implement a (`mutex`) to sell a single TV shoppers as they enter the store
    - print the order in which shoppers buy the TV and the price they bought it for
    - end

#### Exercise 2 black_friday.c :
In the following year, the store manager decides to only let one customer in at a time to manage the chaos by hiring a bouncer. 
Now, let's implement a simulation where:
    - *n* shopper threads and one *bouncer* thread are spawned
    - Each shopper must wait for the bouncer thread to open the door
    - The bouncer will let all of the shopper threads in the store by unlocking the (`door`) lock

### runway.c
Let's try to use our mutexes to ensure FIFO ordering. Specifically, we'll look at the situation when we need to lock and unlock certain critical sections in a certain order. Imagine an airstrip thats meant to be under the radar. They are usually short, and planes can either take off or land. Now, imagine that our drug lord who has this tiny little airstrip has multiple airplanes that are all doing the same run, i.e, a plane that takes off first is the first one to land. The next plane can't take off until the first one takes off, and can only land after the first plane has landed. This becomes circular pretty quickly. We can use a mutex lock for each plane, and let the first plane let the next one know when its taken off and landed so the next one can do the same. 
This looks something like 
takeoff: [1] [0]
plane one has taken off, updating locks
takeoff: [0] [1]
plane two has taken off, updating...
takeoff: [1] [0]

landing: [1] [0]
plane one has landed, updating locks
landing: [0] [1]
plane two has landed, updating locks
landing: [1] [0]