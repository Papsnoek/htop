/*
htop - TemperatureMeter.c
(C) 2013 Ralf Stemmer
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "TemperatureMeter.h"

#include "ProcessList.h"
#include "CRT.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*{
#include "Meter.h"
}*/

int TemperatureMeter_attributes[] = {
   TEMPERATURE_COOL,
   TEMPERATURE_MEDIUM,
   TEMPERATURE_HOT,
};

static void TemperatureMeter_setValues(Meter* this, char* buffer, int len) {
   ProcessList* pl = this->pl;
   this->total = pl->totalTasks;
   this->values[0] = pl->runningTasks;
   snprintf(buffer, len, "%d/%d", (int) this->values[0], (int) this->total);
}

static void TemperatureMeter_display(Object* cast, RichString* out) {
   FILE *p;
   p = popen("sensors", "r");
   if(p == NULL) return 1;

   int textColor   = CRT_colors[METER_TEXT];
   int coolColor   = CRT_colors[TEMPERATURE_COOL];
   int mediumColor = CRT_colors[TEMPERATURE_MEDIUM];
   int hotColor    = CRT_colors[TEMPERATURE_HOT];

   size_t read, len;
   char *line = NULL;
   char *entry = NULL;
   char *tstart = NULL, *tend = NULL;
   int  temperature;
   while ((read = getline(&line, &len, p)) != -1) {
      // contains this line a core-temperature?
      entry = strstr(line, "Core ");
      if (entry == NULL) continue;

      // find the begin of the temperature value
      tstart = strchr(entry, '+'); // no negative temperatures expected :)
      if (tstart == NULL) continue;
      tstart++; // jump over the '+'

      // find the end of the temperature. Remember, it can be above 99°C ;)
      tend = strchr(tstart, '.'); // just the integer
      if (tend == NULL) continue;

      // convert the string into an integer, this is necessary for further steps
      temperature = strtol(tstart, &tend, 10);
      if (temperature == LONG_MAX || temperature == LONG_MIN) continue;
      if (tstart == tend) continue;

      // choose the color for the temperature
      int tempColor;
      if      (temperature < 80)                      tempColor = coolColor;
      else if (temperature >= 80 && temperature < 90) tempColor = mediumColor;
      else                                            tempColor = hotColor;

      // output the temperature
      char buffer[20];
      sprintf(buffer, "%d", temperature);
      RichString_append(out, tempColor, buffer);
      RichString_append(out, textColor, "°C ");
   }

   free(line);
   pclose(p);
}

MeterType TemperatureMeter = {
   .setValues = TemperatureMeter_setValues,
   .display = TemperatureMeter_display,
   .mode = TEXT_METERMODE,
   .items = 1,
   .total = 100.0,
   .attributes = TemperatureMeter_attributes,
   .name = "Temperature",
   .uiName = "Temperature Sensors",
   .caption = "Temperature: "
};
