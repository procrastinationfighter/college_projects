# Bid scheduling
This patch adds a new function ```int setbid(int bid)``` to the unistd library. After calling the function, the current process will change its scheduling algorithm.
Every process that called this function with ```bid``` different from zero will now be scheduled with the "lowest unique bid" algorithm. 
The processes can return to normal scheduling after calling ```setbid(0)```. All future children of this process remain unaffected.
