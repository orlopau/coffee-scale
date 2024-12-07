#include "mode_calibrate.h"
#include "logger.h"
#include "data/localization.h"
#include "loadcell.h"
#include "interface.h"

#define TAG "MODE-CAL"

ModeCalibration::ModeCalibration(Display &display, Stopwatch &stopwatch, void (*saveScaleFnc)(float))
    : display(display), stopwatch(stopwatch),
      calibrationStep(CalibrationStep::BEGIN), saveScaleFnc(saveScaleFnc) {}

void ModeCalibration::update()
{
    static char buffer[20];
    float average;
    float scale;

    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        display.text("Starting calibration.\nRemove all items from\nscale.\n\nClick to continue!");
        sumMeasurements = 0;
        numMeasurements = 0;

        if (LoadCell::isReady())
        {
            tare = LoadCell::read();
        }

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            LOGI(TAG, "Tare: %ld\n", tare);
            calibrationStep = CalibrationStep::ADD_WEIGHT;
        }
        break;
    case CalibrationStep::ADD_WEIGHT:
        display.text("Add weight to scale.\n\nClick to continue!");
        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }
        break;
    case CalibrationStep::CALIBRATING:
        display.text("Calibrating...");
        if (LoadCell::isReady())
        {
            sumMeasurements += static_cast<unsigned long>(LoadCell::read());
            numMeasurements++;
        }

        if (numMeasurements >= CALIBRATION_SAMPLE_SIZE)
        {
            average = static_cast<float>(sumMeasurements) / static_cast<float>(numMeasurements);
            LOGI(TAG, "Average: %f\n", average);

            scale = static_cast<float>(DEFAULT_CALIBRATION_WEIGHT) / (average - tare);
            saveScaleFnc(scale);
            calibrationStep = CalibrationStep::END;
        }
        break;
    case CalibrationStep::END:
        static char buffer[48];
        sprintf(buffer, "Calibration complete.\nScale: %.4f", scale);
        display.text(buffer);
        break;
    }
}

bool ModeCalibration::canSwitchMode()
{
    return calibrationStep == CalibrationStep::END;
}

const char *ModeCalibration::getName()
{
    return MODE_NAME_CALIBRATE;
}