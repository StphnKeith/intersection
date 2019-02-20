/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"

/* Takes a Car pointer car and a SafeStopSign pointer sign and determines if car
 * can move through sign without colliding.
 * If the car's path is available it is recorded in the sign's quadrant field.
*/
int pathAvailable(Car *car, SafeStopSign *sign) {
    int path[3];
    int pathLength = getStopSignRequiredQuadrants(car, path);

    for (int i = 0; i < pathLength; i++) {
        if (sign->quadrants[path[i]] == 1) {
            return 0;
        }
    }

    // If it gets here the car can move and must claim the path
    for (int i = 0; i < pathLength; i++) {
        sign->quadrants[path[i]] = 1;
    }

    return 1;
}

/* Takes a Car pointer car and a SafeStopSign pointer sign and de-selects
 * the quadrants of sign that car passed through.
*/
void clearPath(Car *car, SafeStopSign *sign) {
    int path[3];
    int pathLength = getStopSignRequiredQuadrants(car, path);
    for (int i = 0; i < pathLength; i++) {
        sign->quadrants[path[i]] = 0;
    }
}

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

    initMutex(&sign->pathMutex);
    initConditionVariable(&sign->pathCV);
    for (int i = 0; i < QUADRANT_COUNT; i++) {
        initMutex(&sign->laneMutex[i]);
        initConditionVariable(&sign->laneCV[i]);
        initQueue(&sign->laneQueue[i]);
        sign->quadrants[i] = 0;
    }
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

    destroyMutex(&sign->pathMutex);
    destroyCV(&sign->pathCV);
    for (int i = 0; i < QUADRANT_COUNT; i++) {
        destroyMutex(&sign->laneMutex[i]);
        destroyCV(&sign->laneCV[i]);
        destroyQueue(&sign->laneQueue[i]);
    }
}

void runStopSignCar(Car* car, SafeStopSign* sign) {

    // === Initialization ===

    int laneIndex = getLaneIndex(car);
    pthread_mutex_t *pathMutex = &sign->pathMutex;
    pthread_cond_t *pathCV = &sign->pathCV;
    pthread_mutex_t *laneMutex = &sign->laneMutex[laneIndex];
    pthread_cond_t *laneCV = &sign->laneCV[laneIndex];
    Queue *laneQueue = &sign->laneQueue[laneIndex];
    EntryLane* lane = getLane(car, &sign->base);


    // === Logic ===

    // Enter lane, record order, wait until first
    lock(laneMutex);
        enterQueue(car, laneQueue);
    	enterLane(car, lane);
        while (!first(car, laneQueue)) {
            waitCV(laneCV, laneMutex);
        }
    unlock(laneMutex);

    // Wait until the path is clear
    lock(pathMutex);
        while (!pathAvailable(car, sign)) {
            waitCV(pathCV, pathMutex);
        }
    unlock(pathMutex);

	goThroughStopSign(car, &sign->base);

    // Record the path has been cleared, signal other cars
    lock(pathMutex);
        clearPath(car, sign);
        broadcastCV(pathCV);
    unlock(pathMutex);

    // Exit intersection, signal that a new car is first
    lock(laneMutex);
        exitIntersection(car, lane);
        exitQueue(laneQueue);
        broadcastCV(laneCV);
    unlock(laneMutex);
}
