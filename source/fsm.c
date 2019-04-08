#include "fsm.h"
#include "elev.h"
#include "controller.h"
#include <stdio.h>




void fsm_stop_seq(Elev *elev) {
//	elev->stopBtn = 1;
	elev_set_motor_direction(DIRN_STOP);
	elev_set_stop_lamp(1);
	printf("Stop btn pressed\n");
	
	// Remove all orders in orderQueue
	for (int i=0; i<=9; i++) {
		elev->orderQueue[i] = 0;	
	}

	controller_turn_off_all_lights(elev);
	
	// If elevator at floor, open door (D.3)
	if (elev_get_floor_sensor_signal() != -1) {
		elev_set_door_open_lamp(1);
		while (elev_get_stop_signal()) {}
		elev_set_stop_lamp(0);
		controller_delay(3, elev);
		elev_set_door_open_lamp(0);
	} else {
		while (elev_get_stop_signal()) {}
		elev_set_stop_lamp(0);
	}
	elev->currentOrder = -1;
	elev->state = IDLE;
}

int fsm_init_seq(Elev *elev) {
	elev->state = INIT;
	//elev->stopBtn = 0;
	// Initialize hardware
	printf("Initializing...");
	if (!elev_init()) {
		printf("Unable to initialize elevator hardware!\n");
		return 1;
	}

	// Kj�r til 1. etasje
	controller_go_to(elev, 0);
	elev->currentFloor = 0;
	elev->currentOrder = -1;
	elev->state = IDLE;
	return 0;
}

void fsm_delete_order(Elev *elev) {
	if (elev->currentOrder == elev_get_floor_sensor_signal()) {
		// Delete order in first floor
		if (elev->currentOrder == 0 || elev->currentOrder == 4) {
			elev->orderQueue[0] = 0;
			elev->orderQueue[4] = 0;
			elev_set_button_lamp(BUTTON_COMMAND, 0, 0);
			elev_set_button_lamp(BUTTON_CALL_UP, 0, 0);
		}
		// ... second floor
		else if ((elev->currentOrder == 1 || elev->currentOrder == 5 || elev->currentOrder == 7)) {
			elev->orderQueue[1] = 0;
			elev->orderQueue[5] = 0;
			elev->orderQueue[7] = 0;
			elev_set_button_lamp(BUTTON_COMMAND, 1, 0);
			elev_set_button_lamp(BUTTON_CALL_UP, 1, 0);
			elev_set_button_lamp(BUTTON_CALL_DOWN, 1, 0);
		}
		// ... third floor
		else if ((elev->currentOrder == 2 || elev->currentOrder == 6 || elev->currentOrder == 8)) {
			elev->orderQueue[2] = 0;
			elev->orderQueue[6] = 0;
			elev->orderQueue[8] = 0;
			elev_set_button_lamp(BUTTON_COMMAND, 2, 0);
			elev_set_button_lamp(BUTTON_CALL_UP, 2, 0);
			elev_set_button_lamp(BUTTON_CALL_DOWN, 2, 0);
		}
		// ... fourth floor
		else if (elev->currentOrder == 3 || elev->currentOrder == 9) {
			elev->orderQueue[3] = 0;
			elev->orderQueue[9] = 0;
			elev_set_button_lamp(BUTTON_COMMAND, 3, 0);
			elev_set_button_lamp(BUTTON_CALL_DOWN, 3, 0);
		}
		elev->currentOrder = -1;
	}
}

void fsm_wait_seq(Elev *elev) {
	elev_set_motor_direction(DIRN_STOP);
	
	// Remove executed order from orderQueue
	fsm_delete_order(elev);
	
	// Open and close doors
	elev_set_door_open_lamp(1);
	controller_delay(3, elev);
	elev_set_door_open_lamp(0);

	// Check if there are more orders to be executed
	for (int i=0; i<=9; i++) {
		if (elev->orderQueue[i] == 1) {
			// First floor
			if ((i == 0 || i == 4) && (elev->currentOrder == -1)) {
				elev->currentOrder = 0;
			}
			// Second floor
			else if ((i == 1 || i == 5 || i == 7) && (elev->currentOrder == -1)) {
				elev->currentOrder = 1;
			}
			// Third floor
			else if ((i == 2 || i == 6 || i == 8) && (elev->currentOrder == -1 || elev->currentOrder == 2) ) {
				elev->currentOrder = 2;
			}
			// Fourth floor
			else if ((i == 3 || i == 9) && (elev->currentOrder == -1 || elev->currentOrder == 3)) {
				elev->currentOrder = 3;
			}
			elev->state = RUN;	
		} 
	}
	
	// No more orders
	if (elev->currentOrder == -1) {
		elev->state = IDLE;
	}
}

void fsm_run_seq(Elev *elev) {
	for (int i=0; i<=9; i++) {
		if (elev->orderQueue[i] == 1) {
			// First floor
			if ((i == 0 || i == 4) && (elev->currentOrder == 0 || elev->currentOrder == -1)) {
				controller_go_to(elev, 0);
			}
			// Second floor
			else if ((i == 1 || i == 5 || i == 7) && (elev->currentOrder == 1 || elev->currentOrder == -1)) {
				controller_go_to(elev, 1);
			}
			// Third floor
			else if ((i == 2 || i == 6 || i == 8) && (elev->currentOrder == 2 || elev->currentOrder == -1)) {
				controller_go_to(elev, 2);
			}
			// Fourth floor
			else if ((i == 3 || i == 9) && (elev->currentOrder == 3 || elev->currentOrder == -1)) {
				controller_go_to(elev, 3);
			}	
		}
	}
}


void fsm_idle_seq(Elev *elev) {
	elev_set_motor_direction(DIRN_STOP);
	//controller_btn_listener(elev);
	for (int i=0; i<=9; i++) {
		if (elev->orderQueue[i] == 1) {
			// First floor
			if (i == 0 || i == 4) {
				elev->currentOrder = 0;
			}
			// Second floor
			else if (i == 1 || i == 5 || i == 7) {
				elev->currentOrder = 1;
			}
			// Third floor
			else if (i == 2 || i == 6 || i == 8) {
				elev->currentOrder = 2;
			}
			// Fourth floor
			else if (i == 3 || i == 9) {
				elev->currentOrder = 3;
			}
			elev->state = RUN;
		} 
	}

}



/*

void fsm_idle_seq(Elev *elev) {
	elev_set_motor_direction(DIRN_STOP);
	controller_btn_listener(elev);
	for (int i=0; i<=9; i++) {
		if (elev->orderQueue[i] == 1) {
			elev->state = RUN;
		} 
	}
}

*/