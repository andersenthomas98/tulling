#pragma once
#include "controller.h"
/** 
* @file 
* @brief Module dedicated for each sequence which is run in an associated state.
* States: STOP, INIT, IDLE, RUN, WAIT. 
*/

/**
* @brief Stop the elevator. Remove all orders from orderQueue. Turn of lights. 
* If elevator in defined floor, open door for three seconds. Switch to next state: IDLE.
*  @param elev Elev struct
*/
void fsm_stop_seq(Elev *elev);

/**
* @brief Initialize hardware and Elev struct, move to floor 0 (first floor).
* Next state: IDLE.
* @param elev Elev struct
* @return 1 if unable to initialize hardware, else return 0.
*/
int fsm_init_seq(Elev *elev);

/**
* @brief Stand still while waiting for new order. When button is pressed, 
* update currentOrder and switch to next state: RUN.
* @param elev Elev struct
*/
void fsm_idle_seq(Elev *elev);


/**
* @brief Move elevator to desired floor. Next state: WAIT
* @param elev Elev struct
*/
void fsm_run_seq(Elev *elev);


/** 
* The elevator has reached the desired floor.
* Open door for three seconds, shut door.
* New state: IDLE / RUN (depending on if there is another order)
* @param elev Elev struct
*/
void fsm_wait_seq(Elev *elev);


/**
* @brief Delete order from orderQueue when the order has been executed, and turn off associated lights
* @param elev Elev struct.
*/
void fsm_delete_order(Elev *elev);

void fsm_update_currentOrder(Elev *elev);