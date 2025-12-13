#ifndef CONTROLLER_H
#define CONTROLLER_H

void turn_on_device(int sock, int token);
void turn_off_device(int sock, int token);
void set_pump_device(int sock, int token);
void set_aerator_device(int sock, int token);
void set_feeder_device(int sock, int token);
void set_ph_regulator_device(int sock, int token);

#endif
