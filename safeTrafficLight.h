#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe traffic light submission code.
*/
#include "car.h"
#include "trafficLight.h"
#include "myCommon.h"

/**
* @brief Structure that you can modify as part of your solution to implement
* proper synchronization for the traffic light intersection.
*
* This is basically a wrapper around TrafficLight, since you are not allowed to 
* modify or directly access members of TrafficLight.
*/
typedef struct _SafeTrafficLight {

	/**
	* @brief The underlying light.
	*
	* You are not allowed to modify the underlying traffic light or directly
	* access its members. All interactions must be done through the functions
	* you have been provided.
	*/
	TrafficLight base;

	// Mutex for modifying traffic light data.
    pthread_mutex_t lightMutex;

    // Condition Variable for signalling that the light is green.
    pthread_cond_t trafficCV;

    // Condition Variable for signalling that left turning cars can act.
    pthread_cond_t actCV;

    // Mutex's, CV's, and Queues for maintaining enter and exit order.
    pthread_mutex_t laneMutex[TRAFFIC_LIGHT_LANE_COUNT];
    pthread_cond_t laneCV[TRAFFIC_LIGHT_LANE_COUNT];
    Queue laneQueue[TRAFFIC_LIGHT_LANE_COUNT];

    int previousLightState;
} SafeTrafficLight;

/**
* @brief Initializes the safe traffic light.
*
* @param light pointer to the instance of SafeTrafficLight to be initialized.
* @param eastWest total number of cars moving east-west.
* @param northSouth total number of cars moving north-south.
*/
void initSafeTrafficLight(SafeTrafficLight* light, int eastWest, int northSouth);

/**
* @brief Destroys the safe traffic light.
*
* @param light pointer to the instance of SafeTrafficLight to be destroyed.
*/
void destroySafeTrafficLight(SafeTrafficLight* light);

/**
* @brief Runs a car-thread in a traffic light scenario.
*
* @param car pointer to the car.
* @param light pointer to the traffic light intersection.
*/
void runTrafficLightCar(Car* car, SafeTrafficLight* light);
