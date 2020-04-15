/*
 *  pal_thread.h
 *
 *  Multi-threading abstraction layer
 *
 *  Copyright 2013, 2014 Michael Zillgith
 *
 *	This file is taken from libIEC61850.
 */

#ifndef THREAD_HAL_H_
#define THREAD_HAL_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! \addtogroup hal
   *
   *  @{
   */

/**
 * @defgroup HAL_THREAD Threading and synchronization API
 *
 * @{
 */

/** Opaque reference of a Thread instance */
typedef struct sThread* Thread;

/** Qpaque reference of a Semaphore instance */
typedef void* Semaphore;

/** Reference to a function that is called when starting the thread */
typedef void* (*ThreadExecutionFunction) (void*);

/**
 * \brief Create a new Thread instance
 *
 * \param function the entry point of the thread
 * \param parameter a parameter that is passed to the threads start function
 * \param autodestroy the thread is automatically destroyed if the ThreadExecutionFunction has finished.
 *
 * \return the newly created Thread instance
 */
Thread
Thread_create(ThreadExecutionFunction function, void* parameter, bool autodestroy);

/**
 * \brief Start a Thread.
 *
 * This function invokes the start function of the thread. The thread terminates when
 * the start function returns.
 *
 * \param thread the Thread instance to start
 */
void
Thread_start(Thread thread);

/**
 * \brief Destroy a Thread and free all related resources.
 *
 * \param thread the Thread instance to destroy
 */
void
Thread_destroy(Thread thread);

/**
 * \brief Suspend execution of the Thread for the specified number of milliseconds
 */
void
Thread_sleep(int millies);

Semaphore
Semaphore_create(int initialValue);

/* Wait until semaphore value is greater than zero. Then decrease the semaphore value. */
void
Semaphore_wait(Semaphore self);

void
Semaphore_post(Semaphore self);

void
Semaphore_destroy(Semaphore self);

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* THREAD_HAL_H_ */
