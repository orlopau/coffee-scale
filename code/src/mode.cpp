#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"
#include "formatters.h"
#include "millis.h"

#include <Arduino.h>

void ModeDefault::update()
{
    display.display(loadCell.getWeight(), stopwatch.getTime());

    if (buttons.getEncoderClick() == ClickType::LONG)
    {
        loadCell.tare();
    }

    if (buttons.getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
    }
}

ModeCalibrateLoadCell::ModeCalibrateLoadCell(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch, void (*saveScaleFnc)(float))
    : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch),
      calibrationStep(CalibrationStep::BEGIN), saveScaleFnc(saveScaleFnc) {}

float sumMeasurements = 0;
unsigned int numMeasurements = 0;

void ModeCalibrateLoadCell::update()
{
    unsigned int weight = calibrationWeight;
    static char buffer[20];
    float scale;

    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        display.text("Starting calibration.\nRemove all items from\nscale.\n \nClick to continue!");
        loadCell.setScale(1);
        sumMeasurements = 0;
        numMeasurements = 0;

        if (buttons.getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::ENTER_WEIGHT;
        }

        Serial.println(loadCell.getWeight());

        break;
    case CalibrationStep::ENTER_WEIGHT:
        loadCell.tare();

        if (weight + buttons.getEncoderTicks() > 0)
        {
            weight += buttons.getEncoderTicks();
        }
        else
        {
            weight = 0;
        }

        sprintf(buffer, "Weight: %s", formatWeight(weight));
        display.promptText("Enter known weight\nplaced on load cell.", buffer);

        if (buttons.getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }

        Serial.println(loadCell.getWeight());

        break;
    case CalibrationStep::CALIBRATING:
        display.text("Calibrating...");

        if (loadCell.isNewWeight())
        {
            sumMeasurements += loadCell.getWeight();
            numMeasurements++;
        }

        if (numMeasurements >= 20)
        {
            calibrationStep = CalibrationStep::SUCCESS;
        }
        break;
    case CalibrationStep::SUCCESS:
        Serial.printf("Sum: %f, num: %d\n", sumMeasurements, numMeasurements);
        scale = (float)weight / (sumMeasurements / (float)numMeasurements);

        saveScaleFnc(scale);
        display.text("Calibration success!\n\nPlease reboot!");
        calibrationStep = CalibrationStep::END;
        break;
    case CalibrationStep::END:
        break;
    }
}