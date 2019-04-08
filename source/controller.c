#include "controller.h"
#include "elev.h"
#include "fsm.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// Indexes for orderQueue:
// CM<floor> = COMMAND_BUTTON (in elevator) for <floor>
// U<floor> = BUTTON_CALL_UP in <floor>
// D<floor> = BUTTON_CALL_DOWN in <floor>
const int CM0 = 0;
const int CM1 = 1;
const int CM2 = 2;
const int CM3 = 3;
const int U0 = 4;
const int U1 = 5;
const int U2 = 6;
const int D1 = 7;
const int D2 = 8;
const int D3 = 9;


void controller_run(Elev *elev) {
	fsm_init_seq(elev);
	while (1) {
		controller_checkStopBtn(elev);
		controller_btn_listener(elev);
		if (elev->state == STOP) {
			fsm_stop_seq(elev);
		}
		else if (elev->state == IDLE) {
			fsm_idle_seq(elev);
		}
		else if (elev->state == RUN) {
			fsm_run_seq(elev);
		}
		else if (elev->state == WAIT) {
			fsm_wait_seq(elev);
		}
	}
}


void controller_go_to(Elev *elev, int floor) {
	if (elev->state == INIT) {
		while (elev_get_floor_sensor_signal() != floor) {
			elev_set_motor_direction(DIRN_DOWN);
			if (controller_checkStopBtn(elev)) {  // Should be able to stop while initializing
				printf("Unable to initialize: Stop button pressed!\n");
				return;
			}
		}
		elev_set_motor_direction(DIRN_STOP);
		elev->currentFloor = elev_get_floor_sensor_signal();
	} 
	else {
		controller_updateCurrentFloor(elev);

		// Stop elevator if it has reached desired floor
		if (floor == elev->currentFloor && elev_get_floor_sensor_signal() != -1) {
			elev_set_motor_direction(DIRN_STOP);
			elev->state = WAIT;
			elev->dir = DIRN_STOP;
		}

		// Move elevator down
		else if (floor < elev->currentFloor) {
			elev_set_motor_direction(DIRN_DOWN);
			elev->dir = DIRN_DOWN;
			elev->dir_before_stop = DIRN_DOWN;

			// Special case
			// Execute all orders on the way up to third floor (unless new order is in oppsite direction of elevators current direction)
			if (2 <= elev->currentFloor) {
				if (elev->orderQueue[D2] == 1 || elev->orderQueue[CM2] == 1) {
					elev->currentOrder = 2;
				}
			}
			// Execute all orders on the way up to second floor (unless new order is in oppsite direction of elevators current direction)
			if (1 <= elev->currentFloor) {
				if ((elev->orderQueue[D1] == 1 && elev->orderQueue[D2] == 0) || (elev->orderQueue[CM1] == 1 && elev->orderQueue[CM2] == 0)) {
					elev->currentOrder = 1;
				}
			}
		}

		// Move elevator up
		else if (floor > elev->currentFloor) {
			elev_set_motor_direction(DIRN_UP);
			elev->dir = DIRN_UP;
			elev->dir_before_stop = DIRN_UP;
			// Special case
			// Execute all orders on the way down to second floor (unless new order is in oppsite direction of elevators current direction)
			if (1 >= elev->currentFloor) {
				if (elev->orderQueue[U1] == 1 || elev->orderQueue[CM1] == 1) {
					elev->currentOrder = 1;
				}
			}
			// Execute all orders on the way down to third floor (unless new order is in oppsite direction of elevators current direction)
			if (2 >= elev->currentFloor) {
				if ((elev->orderQueue[U2] == 1 && elev->orderQueue[U1] == 0) || (elev->orderQueue[CM2] == 1 && elev->orderQueue[CM1] == 0)) {
					elev->currentOrder = 2;
				}
			}
		}

		// If elevator stops between floors, make sure the elevator is able to complete next order although currentFloor = floor.
		else if (elev_get_floor_sensor_signal() == -1 && elev->dir == DIRN_STOP && floor == elev->currentFloor) {
			if (elev->dir_before_stop == DIRN_DOWN) {
				elev_set_motor_direction(DIRN_UP);
				elev->dir = DIRN_UP;
			}
			else if (elev->dir_before_stop == DIRN_UP) {
				elev_set_motor_direction(DIRN_DOWN);
				elev->dir = DIRN_DOWN;
			}
		}
		//elev->stopBtn = 0; 
	}
}

int controller_checkStopBtn(Elev *elev) {
	if (elev_get_stop_signal()) {
		elev->state = STOP;
		elev->dir = DIRN_STOP;
		return 1;
	}
	return 0;
}

void controller_delay(unsigned int s, Elev *elev) {
	int timestamp = time(NULL);
	while ((time(NULL) - timestamp) <= s) {
		controller_btn_listener(elev); // Add order to orderQueue if btn is pressed while door open
		if (controller_checkStopBtn(elev)) {  // If stop btn is pressed while door is open during WAIT-state
			return;
		}
	}
}

void controller_updateCurrentFloor(Elev *elev) {
	if (elev->currentFloor != elev_get_floor_sensor_signal() && elev_get_floor_sensor_signal() != -1) {
		if (elev->dir == DIRN_DOWN) {
			elev->currentFloor = elev->currentFloor - 1;
		}
		else if (elev->dir == DIRN_UP) {
			elev->currentFloor = elev->currentFloor + 1;
		}
		// Update floor indicator lamps
		elev_set_floor_indicator(elev->currentFloor);
	}
}

void controller_btn_listener(Elev *elev) {
	// Listen for btn press, and add to orderQueue if btn is pressed
	// Check command buttons (inside elevator)
	for (int i=CM0; i<=CM3; i++) {
		if (elev_get_button_signal(BUTTON_COMMAND, i)) {
			elev->orderQueue[i] = 1;
			elev_set_button_lamp(BUTTON_COMMAND, i, 1);	
		}
	}
	
	// Check up-buttons (outside elevator)
	for (int i=U0; i<=U2; i++) {
		if (elev_get_button_signal(BUTTON_CALL_UP, i-4)) {
			elev->orderQueue[i] = 1;	
			elev_set_button_lamp(BUTTON_CALL_UP, i-4, 1);	
		}
	}

	// Check down-buttons (outside elevator)
	for (int i=D1; i<=D3; i++) {
		if (elev_get_button_signal(BUTTON_CALL_DOWN, i-6)) {
			elev->orderQueue[i] = 1;
			elev_set_button_lamp(BUTTON_CALL_DOWN, i-6, 1);	
		}	
	}
}

void controller_turn_off_all_lights(Elev *elev) {
	for (int i=CM0; i<=CM3; i++) {
		elev_set_button_lamp(BUTTON_COMMAND, i, 0);	
	}
	for (int i=U0; i<=U2; i++) {
		elev_set_button_lamp(BUTTON_CALL_UP, i-4, 0);	
	}
	for (int i=D1; i<=D3; i++) {
		elev_set_button_lamp(BUTTON_CALL_DOWN, i-6, 0);		
	}
}

