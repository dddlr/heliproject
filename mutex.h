/*
 * mutex.h
 *
 *  ENCE361 HeliProject Group 3
 *  Created on: 18/03/2021
 *  Author: CoppyNawaphanarat, Grant Wong, Will Archer
 *
 *  Purpose:
 *
 *  To prevent the race conditions within the codes, so we won't run into deadlocks.
 *  Therefore, we think that all variables within the ISRs requires a mutex assistance.
 *
 *  To make this works:
 *
 *  - All mutexes must be a static type within their own modules.
 *  - Only ISRs should be using lock and unlock for their task.
 *  - Only Non-ISRs should be using mutex_wait until the ISRs unlocks after finishing their tasks.
 *
 *  If this works as we expected, it should be greatly reduce the risk of accessing a variable,
 *  such as the buffer data, while it is being changed.
 *
 */

#ifndef MUTEX_H_
#define MUTEX_H_

/**
 * Define the mutex type. Using "volatile" for arise in hardware access (memory-mapped I/O),
 * where writing to memory or reading from is used to communicate with peripheral devices.
 * Basically, the value may change between different accesses, even if it does not appear to be modified.
 *
 * For more information: https://en.wikipedia.org/wiki/Volatile_(computer_programming)
 */
typedef volatile bool Mutex;

/**
 * Creates a lock within a mutex.
 */
#define mutex_lock(mutex) (mutex = true)

/**
 * Unlocks the lock within a mutex.
 */
#define mutex_unlock(mutex) (mutex = false)

/**
 * Waiting state for the lock to be release
 */
#define mutex_wait(mutex) while (mutex) continue


#endif /* MUTEX_H_ */
