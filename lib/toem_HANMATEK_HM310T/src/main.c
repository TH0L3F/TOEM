#include "../inc/main.h"
#include <stdint.h> //uint16_t, uint8_t
#include <stdio.h>
#include <stdbool.h> //bool
#include <stdlib.h> //malloc
#include <modbus/modbus.h> //modbus

#define print_output 1

uint16_t read_reg(uint8_t n_reg, modbus_t * ctx){
	uint16_t value;
	if (modbus_read_registers(ctx, n_reg, n_reg, &value) == -1){
		return 0xFFFF; //Data in case of an error
	}
	return value;
}	

bool write_reg(uint8_t n_reg, uint16_t value, modbus_t * ctx){
	if(modbus_write_register(ctx, n_reg, value) == -1){
		return 0;
	}
	return 1; //1 if ok, 0 if error
}

void read_config(struct hm310t_config * config, modbus_t * ctx){
	config->voltage = read_reg(SET_VOLTAGE,ctx);
	config->current = read_reg(SET_CURRENT,ctx);
	config->output_state =read_reg(OUTPUT_STATE,ctx);
	return;
}

void read_data(struct hm310t_data * data, modbus_t * ctx){
	data->voltage = read_reg(ACTUAL_VOLTAGE,ctx);
	data->current = read_reg(ACTUAL_CURRENT,ctx);
	data->power = read_reg(ACTUAL_POWER_NORMAL,ctx);
	return;
}

bool write_config(struct hm310t_config * config, modbus_t * ctx){
	bool state = 1;
	state &= write_reg(SET_VOLTAGE, config->voltage, ctx);
	state &= write_reg(SET_CURRENT, config->current, ctx);
	state &= write_reg(OUTPUT_STATE, config->output_state, ctx);
	return state; //1 if ok, 0 if error
}

int main(int argc, char * argv[]) {
	//Memory struct allocation
	struct hm310t_config * config = malloc(sizeof(struct hm310t_config));
	struct hm310t_data * data = malloc(sizeof(struct hm310t_data));
	modbus_t *ctx = NULL;
    
    	// Create Modbus RTU context
    	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    	if (ctx == NULL) {
		if(print_output)printf("cannot create modbus context\n");
        	return 1;
    	}
	if(print_output)printf("modbus context created\n");

    	// Configuration des paramètres série
    	modbus_set_slave(ctx, 1); // Adresse de l'esclave Modbus

    	// Connexion au périphérique Modbus
    	if (modbus_connect(ctx) == -1) {
        	modbus_free(ctx);
		if(print_output)printf("cannot connect to modbus peripheral\n");
        	return EXIT_FAILURE;
    	}
	if(print_output)printf("modbus peripheral connected\n\n");

	while(1){
		read_data(data, ctx);
		if(print_output)printf("%d;%d;%d",data->voltage,data->current,data->power);
	}

    	// Fermeture de la connexion
    	modbus_close(ctx);
    	modbus_free(ctx);
	
	//Analocation memory
	free(&config);
	free(&data);

    	return 0;
}

