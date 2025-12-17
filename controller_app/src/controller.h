#ifndef CONTROLLER_H
#define CONTROLLER_H

void turn_on_device(int sock, int token);
void turn_off_device(int sock, int token);
void set_pump_device(int sock, int token);
void set_aerator_device(int sock, int token);
void set_feeder_device(int sock, int token);
void set_ph_regulator_device(int sock, int token);
void get_pump_device_info(int sock, int token);
void get_aerator_device_info(int sock, int token);
void get_feeder_device_info(int sock, int token);
void get_ph_regulator_device_info(int sock, int token);
void get_sensor_device_info(int sock, int token);
void manual_feed(int sock, int token);
void manual_pump(int sock, int token);
void manual_aerate(int sock, int token);

#endif
