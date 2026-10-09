#include <stdio.h>

#include "mode_calibrate.h"
#include "logger.h"
#include "data/localization.h"
#include "loadcell.h"
#include "interface.h"
#include "ui/widgets.h"

#define TAG "MODE-CAL"

ModeCalibration::ModeCalibration(Stopwatch &stopwatch, void (*saveScaleFnc)(float))
    : stopwatch(stopwatch),
      calibrationStep(CalibrationStep::BEGIN), saveScaleFnc(saveScaleFnc) {}

void ModeCalibration::update()
{
    float average;

    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        sumMeasurements = 0;
        numMeasurements = 0;

        if (LoadCell::isReady())
        {
            tare = LoadCell::read();
        }

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            LOGI(TAG, "Tare: %ld", tare);
            calibrationStep = CalibrationStep::ADD_WEIGHT;
        }
        break;
    case CalibrationStep::ADD_WEIGHT:
        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }
        break;
    case CalibrationStep::CALIBRATING:
        if (LoadCell::isReady())
        {
            sumMeasurements += static_cast<unsigned long>(LoadCell::read());
            numMeasurements++;
        }

        if (numMeasurements >= CALIBRATION_SAMPLE_SIZE)
        {
            average = static_cast<float>(sumMeasurements) / static_cast<float>(numMeasurements);
            LOGI(TAG, "Average: %f", average);

            scale = static_cast<float>(DEFAULT_CALIBRATION_WEIGHT) / (average - tare);
            saveScaleFnc(scale);
            calibrationStep = CalibrationStep::END;
        }
        break;
    case CalibrationStep::END:
        break;
    }
}

void ModeCalibration::render(Canvas &canvas)
{
    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        Widgets::textLines(canvas, "Starting calibration.\nRemove all items from\nscale.\n\nClick to continue!");
        break;
    case CalibrationStep::ADD_WEIGHT:
        Widgets::textLines(canvas, "Add weight to scale.\n\nClick to continue!");
        break;
    case CalibrationStep::CALIBRATING:
        Widgets::textLines(canvas, "Calibrating...");
        break;
    case CalibrationStep::END:
    {
        char buffer[48];
        snprintf(buffer, sizeof(buffer), "Calibration complete.\nScale: %.4f", scale);
        Widgets::textLines(canvas, buffer);
        break;
    }
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