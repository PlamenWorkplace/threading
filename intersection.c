/*
 * Operating Systems (2INC0) Practical Assignment
 * Threading
 *
 * Intersection Part [REPLACE WITH PART NUMBER]
 * 
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 * STUDENT_NAME_3 (STUDENT_NR_3)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "arrivals.h"
#include "intersection_time.h"
#include "input.h"

// TODO: Global variables: mutexes, data structures, etc...
#define TRAFFIC_LIGHTS 10
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m7 = PTHREAD_MUTEX_INITIALIZER;

/* 
 * curr_car_arrivals[][][]
 *
 * A 3D array that stores the arrivals that have occurred
 * The first two indices determine the entry lane: first index is Side, second index is Direction.
 * curr_arrivals[s][d] returns an array of all arrivals for the entry lane on side s for direction d,
 *   ordered in the same order as they arrived
 */
static Car_Arrival curr_car_arrivals[4][4][20];

/*
 * car_sem[][]
 *
 * A 2D array that defines a semaphore for each entry lane,
 *   which are used to signal the corresponding traffic light that a car has arrived
 * The two indices determine the entry lane: first index is Side, second index is Direction
 */
static sem_t car_sem[4][4];

/*
 * supply_cars()
 *
 * A function for supplying car arrivals to the intersection
 * This should be executed by a separate thread
 */
static void *supply_cars(void *arg)
{
  int t = 0;
  int num_curr_arrivals[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

  // for every arrival in the list
  for (int i = 0; i < sizeof(input_car_arrivals)/sizeof(Car_Arrival); i++)
  {
    // get the next arrival in the list
    Car_Arrival arrival = input_car_arrivals[i];
    // wait until this arrival is supposed to arrive
    sleep(arrival.time - t);
    t = arrival.time;
    // store the new arrival in curr_arrivals
    curr_car_arrivals[arrival.side][arrival.direction][num_curr_arrivals[arrival.side][arrival.direction]] = arrival;
    num_curr_arrivals[arrival.side][arrival.direction] += 1;
    // increment the semaphore for the traffic light that the arrival is for
    sem_post(&car_sem[arrival.side][arrival.direction]);
  }

  return(0);
}

void lockMutexes(Side side, Direction direction)
{
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == LEFT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_lock(&m1);
  }
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_lock(&m2);
  }
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == UTURN) || (side == WEST && direction == RIGHT) )
  {
    pthread_mutex_lock(&m3);
  }
  if ( (side == EAST && direction == STRAIGHT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_lock(&m4);
  }
  if ( (side == EAST && direction == STRAIGHT) || (side == SOUTH && direction == LEFT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_lock(&m5);
  }
  if ( (side == EAST && direction == RIGHT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_lock(&m6);
  }
  if ( (side == SOUTH && direction == RIGHT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_lock(&m7);
  }
}

void unlockMutexes(Side side, Direction direction)
{
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == LEFT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_unlock(&m1);
  }
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_unlock(&m2);
  }
  if ( (side == EAST && direction == LEFT) || (side == SOUTH && direction == UTURN) || (side == WEST && direction == RIGHT) )
  {
    pthread_mutex_unlock(&m3);
  }
  if ( (side == EAST && direction == STRAIGHT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_unlock(&m4);
  }
  if ( (side == EAST && direction == STRAIGHT) || (side == SOUTH && direction == LEFT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_unlock(&m5);
  }
  if ( (side == EAST && direction == RIGHT) || (side == SOUTH && direction == STRAIGHT) || (side == WEST && direction == LEFT) )
  {
    pthread_mutex_unlock(&m6);
  }
  if ( (side == SOUTH && direction == RIGHT) || (side == WEST && direction == STRAIGHT) )
  {
    pthread_mutex_unlock(&m7);
  }
}

/*
 * manage_light(void* arg)
 *
 * A function that implements the behaviour of a traffic light
 */
static void *manage_light(void *arg)
{
  Car_Arrival *args = (Car_Arrival *) arg;
  Side side = args->side;
  Direction direction = args->direction;
  free(args);  // Free the allocated memory

  int num_curr_arrival = 0;
  while (true)
  {
    sem_wait(&car_sem[side][direction]);
    lockMutexes(side, direction);
    printf("traffic light %d %d turns green at time %d for car %d\n", side, direction, get_time_passed(), curr_car_arrivals[side][direction][num_curr_arrival].id);
    sleep(CROSS_TIME);
    printf("traffic light %d %d turns red at time %d\n", side, direction, get_time_passed());
    unlockMutexes(side, direction);
    num_curr_arrival += 1;
  }

  return(0);
}

void createTrafficLightThreads(pthread_t my_threads[])
{
  int entry_lanes[TRAFFIC_LIGHTS][2] = { 
    {EAST, LEFT}, {EAST, STRAIGHT}, {EAST, RIGHT},
    {SOUTH, LEFT}, {SOUTH, STRAIGHT}, {SOUTH, RIGHT}, {SOUTH, UTURN}, 
    {WEST, LEFT}, {WEST, STRAIGHT}, {WEST, RIGHT} 
  };

  for (int i = 0; i < TRAFFIC_LIGHTS; i++)
  {
    Car_Arrival *args = malloc(sizeof(Car_Arrival)); // New allocation for each thread

    if (!args) 
    {
      fprintf(stderr, "Error: Memory allocation failed!\n");
      exit(EXIT_FAILURE);
    }

    args->side = entry_lanes[i][0];
    args->direction = entry_lanes[i][1];

    if (pthread_create(&my_threads[i], NULL, manage_light, args) != 0) 
    {
      fprintf(stderr, "Error: Could not create thread %d!\n", i);
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char * argv[])
{
  // create semaphores to wait/signal for arrivals
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_init(&car_sem[i][j], 0, 0);
    }
  }

  // start the timer
  start_time();

  pthread_t my_threads[TRAFFIC_LIGHTS + 1];
  
  // create a thread per traffic light that executes manage_light
  createTrafficLightThreads(my_threads);

  // create a thread that executes supply_cars()
  if (pthread_create(&my_threads[TRAFFIC_LIGHTS], NULL, supply_cars, NULL) != 0) 
  {
    fprintf(stderr, "Error: Could not create thread %d!\n", TRAFFIC_LIGHTS);
    exit(EXIT_FAILURE);
  }

  // wait for all threads to finish
  sleep_until_arrival(END_TIME);
  for (int i = 0; i < TRAFFIC_LIGHTS + 1; i++)
  {
    if (pthread_cancel(my_threads[i]) != 0) 
    {
      fprintf(stderr, "Error: Could not cancel thread! %d\n", i);
    }
    
    if (pthread_join(my_threads[i], NULL) != 0) 
    {
      fprintf(stderr, "Error: Could not join thread %d!\n", i);
    } 
  }

  // destroy semaphores
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_destroy(&car_sem[i][j]);
    }
  }
}