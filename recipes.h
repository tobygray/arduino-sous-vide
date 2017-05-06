#ifndef RECIPIES_H
#define RECIPIES_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#define RECIPE_NAME_MAX 12
void recipes_get_animal_name(byte animal_idx, char* buffer);
void recipes_get_type_name(byte animal_idx, byte type_idx, char* buffer);
void recipes_get_level_name(byte animal_idx, byte type_idx, byte level_idx, char* buffer);
double recipes_get_temperature(byte animal_idx, byte type_idx, byte level_idx);
unsigned long recipes_get_ideal_minutes(byte animal_idx, byte type_idx, byte level_idx);
unsigned long recipes_get_last_call_minutes(byte animal_idx, byte type_idx, byte level_idx);
bool recipes_is_valid(byte animal_idx, byte type_idx, byte level_idx);

#endif // RECIPIES_H
