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
   /*p = popen("sensors", "r");*/
   p = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");
   if (p == NULL)
        exit(EXIT_FAILURE);

   int textColor   = CRT_colors[METER_TEXT];
   int coolColor   = CRT_colors[TEMPERATURE_COOL];
   int mediumColor = CRT_colors[TEMPERATURE_MEDIUM];

   size_t read, len;
   char *line = NULL;
   char *entry = NULL;
   char *tstart = NULL, *tend = NULL;
   int  temperature;
   while ((read = getline(&line, &len, p)) != -1) {
      sscanf (line, "%ld", &temperature);
      // convert the string into an integer, this is necessary for further steps
      //temperature = strtol(0, read, 10);
      if (temperature == LONG_MAX || temperature == LONG_MIN) continue;

      // choose the color for the temperature
      int tempColor;
      if      (temperature < 40)                      tempColor = coolColor;
      else if (temperature >= 40 && temperature < 80) tempColor = mediumColor;
      else                                            tempColor = hotColor;

      // output the temperature
      char buffer[20];
      sprintf(buffer, "%d", temperature);
      RichString_append(out, tempColor, buffer);
      RichString_append(out, textColor, "  C ");
   }

   free(line);
   fclose(p);
}

MeterClass TemperatureMeter_class = {
   .super = {
      .extends = Class(Meter),
      .delete = Meter_delete,
      .display = TemperatureMeter_display
   },
   .updateValues = TemperatureMeter_setValues,
   .defaultMode = TEXT_METERMODE,
   .maxItems = 1,
   .total = 100.0,
   .attributes = TemperatureMeter_attributes,
   .name = "Temperature",
   .uiName = "Temperature Sensors",
   .caption = "Temperature: "
};
