/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"

void beforeSleep(void *userPtr) {
	SafeTrafficLight *light = (SafeTrafficLight*)userPtr;
	unlock(&light->lightMutex);
}

/* Acquire lightMutex locks and set previousLightState before
 * the car potentially intiates a switch from red to green.
*/
void afterSleep(void *userPtr) {
	SafeTrafficLight *light = (SafeTrafficLight*)userPtr;
	lock(&light->lightMutex);
	light->previousLightState = getLightState(&light->base);
}

/* Determines if the light is green for a car.
 * Must be called with the lightMutex held.
*/
int green(Car *car, SafeTrafficLight *light) {
	int state = getLightState(&light->base);

	int pos = car->position;

	if ( state == NORTH_SOUTH && (pos == NORTH || pos == SOUTH) ) {
		return 1;
	}
	if ( state == EAST_WEST && (pos == EAST || pos == WEST) ) {
		return 1;
	}
	return 0;
}

/* Calls actTrafficLight and returns true if the car can act.
 * Returns false only if the car cannot act because it is turning left when
 * there are cars coming straight from the other direction.
 * Must be called with lightMutex held.
*/
int canAct(Car *car, SafeTrafficLight *light) {
	int action = car->action;
	if (action == STRAIGHT || action == RIGHT_TURN) {
		actTrafficLight(car, &light->base, beforeSleep, afterSleep, (void*)(light));
		return 1;
	}

	if (getStraightCount(&light->base, getOppositePosition(car->position)) == 0) {
		actTrafficLight(car, &light->base, beforeSleep, afterSleep, (void*)(light));
		return 1;
	}

	return 0;
}

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	light->previousLightState = getLightState(&light->base);
	initMutex(&light->lightMutex);
	for (int i = 0; i < TRAFFIC_LIGHT_LANE_COUNT; i++) {
		initMutex(&light->laneMutex[i]);
		initConditionVariable(&light->laneCV[i]);
		initQueue(&light->laneQueue[i]);
	}

	initConditionVariable(&light->trafficCV);
	initConditionVariable(&light->actCV);
}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	destroyMutex(&light->lightMutex);
	for (int i = 0; i < TRAFFIC_LIGHT_LANE_COUNT; i++) {
		destroyMutex(&light->laneMutex[i]);
		destroyCV(&light->laneCV[i]);
		destroyQueue(&light->laneQueue[i]);
	}

	destroyCV(&light->trafficCV);
	destroyCV(&light->actCV);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// === Initialization ===

	int laneIndex = getLaneIndexLight(car);
	pthread_mutex_t *laneMutex = &light->laneMutex[laneIndex];
	pthread_cond_t *laneCV = &light->laneCV[laneIndex];
	Queue *laneQueue = &light->laneQueue[laneIndex];
	EntryLane* lane = getLaneLight(car, &light->base);

	pthread_mutex_t *lightMutex = &light->lightMutex;
	pthread_cond_t *trafficCV = &light->trafficCV;
	pthread_cond_t *actCV = &light->actCV;


	// === Logic ===

	// Enter the lane, record order
	lock(laneMutex);
		enterQueue(car, laneQueue);
		enterLane(car, lane);
	unlock(laneMutex);

	// Wait for the traffic signal
	lock(lightMutex);
		while ( !green(car, light) ) {
			waitCV(trafficCV, lightMutex);
		}
		enterTrafficLight(car, &light->base);
	unlock(lightMutex);

	// Go through the intersection, while in canAct lightMutex is acquired,
	// actTrafficLight is called, locks are released for nap and reaquired
	// before returning.
	int currentLightState;
	lock(lightMutex);
		while ( !canAct(car, light) ) {
			waitCV(actCV, lightMutex);
		}
		if (getStraightCount(&light->base, getOppositePosition(car->position)) == 0) {
			broadcastCV(actCV);
		}
		currentLightState = getLightState(&light->base);
		if (currentLightState != RED && currentLightState != light->previousLightState) {
			broadcastCV(trafficCV);
		}
	unlock(lightMutex);
	
	// Wait to become first before formally exiting
	lock(laneMutex);
		while ( !first(car, laneQueue) ) {
			waitCV(laneCV, laneMutex);
		}
		exitIntersection(car, lane);
		exitQueue(laneQueue);
		broadcastCV(laneCV);
	unlock(laneMutex);
}
