## Qthreads - A pthreads-like threading library implementation.

This project is a simplistic implementation of the threading library pthreads, the provided makefile builds the project and 
returns a static library which can be linked to by other programs. 

### Linking to QThreads:
Any program can be linked to the Qthreads library by usig the **_-lqthread_** option with gcc

### Data Structures and API's Provided
The following data structures and APIs are provided by the implementation:

**Data Structures Provided**
* qthread_t
* qthread_mutex_t
* qthread_cond_t

**API's for threads mangement**
* qthread_create
* qthread_accept
* qthread_usleep
* qthread_attr_init

**API's for mutexes**
* qthread_mutex_init
* qthread_mutex_lock
* qthread_mutex_unlock

**API's for Conditional Variables**
* qthread_cond_init
* qthread_cond_wait
* qthread_cond_signal

The api's and data structures are very similar to the implementation of the pthreads library and work almost equivalently. 

## Testing
We have tested our library against the dining philosopher's problem, a sample multithreaded web-server and several other multithreading programs written using the qthreads library. 

To test the library use the **_make test_** phony from the makefiles. 

