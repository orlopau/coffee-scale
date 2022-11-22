#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"
#include "formatters.h"

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

ModeCalibrateLoadCell::ModeCalibrateLoadCell(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch)
    : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch),
      calibrationStep(CalibrationStep::ENTER_WEIGHT) {}

void ModeCalibrateLoadCell::update()
{
    unsigned int weight = calibrationWeight;
    static char buffer[20];
    float sumMeasurements = 0;
    unsigned int numMeasurements = 0;

    if (buttons.getEncoderClick() == ClickType::SINGLE)
    {
        calibrationStep = static_cast<CalibrationStep>(static_cast<int>(calibrationStep) + 1);
    }

    switch (calibrationStep)
    {
    case CalibrationStep::ENTER_WEIGHT:
        weight += buttons.getEncoderTicks();
        sprintf(buffer, "Weight: %s", formatWeight(weight));
        display.promptText("Enter known weight placed on load cell.", buffer);

        if (buttons.getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }
        break;
    case CalibrationStep::CALIBRATING:
        display.singleText("Calibrating...");

        if (loadCell.isNewWeight())
        {
            sumMeasurements += loadCell.getWeight();
            numMeasurements++;
        }

        if (sumMeasurements >= 20)
        {
            calibrationStep = CalibrationStep::SUCCESS;
        }
        break;
    case CalibrationStep::SUCCESS:
        float scale = weight / (sumMeasurements / numMeasurements);
        // TODO save scale to EEPROM
        display.singleText("Calibration successful, please reboot!");
        break;
    }
}