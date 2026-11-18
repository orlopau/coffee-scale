#include <stdio.h>

#include "mode_calibrate.h"
#include "logger.h"
#include "data/localization.h"
#include "interface.h"
#include "ui/widgets.h"

#define TAG "MODE-CAL"

ModeCalibration::ModeCalibration(WeightSource &weightSource, Stopwatch &stopwatch, void (*saveScaleFnc)(float))
    : weightSource(weightSource), stopwatch(stopwatch),
      calibrationStep(CalibrationStep::BEGIN), saveScaleFnc(saveScaleFnc),
      sumMeasurements(0), numMeasurements(0), tare(0), lastRaw(0),
      rate(0), rateCount(0), rateStartMs(0), rateStarted(false), average(0), scale(0) {}

void ModeCalibration::noteReading(const Reading &reading)
{
    lastRaw = reading.raw;
    if (!rateStarted)
    {
        rateStarted = true;
        rateStartMs = reading.ms;
    }
    rateCount++;
    if (reading.ms - rateStartMs >= 1000)
    {
        rate = rateCount;
        rateCount = 0;
        rateStartMs = reading.ms;
    }
}

void ModeCalibration::update()
{
    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        sumMeasurements = 0;
        numMeasurements = 0;

        for (const Reading &reading : weightSource.newReadings())
        {
            tare = reading.raw;
            noteReading(reading);
        }

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            LOGI(TAG, "Tare: %ld", tare);
            calibrationStep = CalibrationStep::ADD_WEIGHT;
        }
        break;
    case CalibrationStep::ADD_WEIGHT:
        for (const Reading &reading : weightSource.newReadings())
        {
            noteReading(reading);
        }

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }
        break;
    case CalibrationStep::CALIBRATING:
        for (const Reading &reading : weightSource.newReadings())
        {
            noteReading(reading);
            if (numMeasurements < CALIBRATION_SAMPLE_SIZE)
            {
                sumMeasurements += reading.raw;
                numMeasurements++;
            }
        }

        if (numMeasurements >= CALIBRATION_SAMPLE_SIZE)
        {
            average = static_cast<float>(sumMeasurements) / static_cast<float>(numMeasurements);
            LOGI(TAG, "Average: %f", average);

            // Without any change in the raw value the scale would be inf, and
            // saving that would break weighing until the next calibration.
            if (average != tare)
            {
                scale = static_cast<float>(DEFAULT_CALIBRATION_WEIGHT) / (average - tare);
                saveScaleFnc(scale);
            }
            calibrationStep = CalibrationStep::END;
        }
        break;
    case CalibrationStep::END:
        break;
    }
}

void ModeCalibration::render(Canvas &canvas)
{
    char buffer[128];

    switch (calibrationStep)
    {
    case CalibrationStep::BEGIN:
        snprintf(buffer, sizeof(buffer), "Starting calibration.\nRemove all items from\nscale.\nRaw: %ld  %u/s\n\nClick to continue!", lastRaw, rate);
        break;
    case CalibrationStep::ADD_WEIGHT:
        snprintf(buffer, sizeof(buffer), "Add %d g to scale.\nTare: %ld\nRaw:  %ld  %u/s\nDiff: %ld\n\nClick to continue!",
                 DEFAULT_CALIBRATION_WEIGHT, tare, lastRaw, rate, lastRaw - tare);
        break;
    case CalibrationStep::CALIBRATING:
        snprintf(buffer, sizeof(buffer), "Calibrating... %u/%d\nTare: %ld\nRaw:  %ld  %u/s\nDiff: %ld",
                 numMeasurements, CALIBRATION_SAMPLE_SIZE, tare, lastRaw, rate, lastRaw - tare);
        break;
    case CalibrationStep::END:
        if (average != tare)
        {
            snprintf(buffer, sizeof(buffer), "Calibration complete.\nTare: %ld\nAvg:  %.0f\nDiff: %.0f\nScale: %.6f",
                     tare, average, average - tare, scale);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "Calibration failed:\nraw value unchanged.\nNot saved.\nTare: %ld\nAvg:  %.0f",
                     tare, average);
        }
        break;
    }
    Widgets::textLines(canvas, buffer);
}

bool ModeCalibration::canSwitchMode()
{
    return calibrationStep == CalibrationStep::END;
}

const char *ModeCalibration::getName()
{
    return MODE_NAME_CALIBRATE;
}