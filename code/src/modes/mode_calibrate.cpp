#include "mode_calibrate.h"

ModeCalibration::ModeCalibration(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch, void (*saveScaleFnc)(float))
    : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch),
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
        loadCell.setScale(1);

        if (buttons.getEncoderClick() == ClickType::SINGLE)
        {
            loadCell.tare();
            calibrationStep = CalibrationStep::ADD_WEIGHT;
        }
        break;
    case CalibrationStep::ADD_WEIGHT:
        display.text("Add weight to scale.\n\nClick to continue!");
        if (buttons.getEncoderClick() == ClickType::SINGLE)
        {
            calibrationStep = CalibrationStep::CALIBRATING;
        }
        break;
    case CalibrationStep::CALIBRATING:
        display.text("Calibrating...");
        if (loadCell.isNewWeight())
        {
            sumMeasurements += static_cast<unsigned long>(loadCell.getWeight());
            numMeasurements++;
        }

        if (numMeasurements >= CALIBRATION_SAMPLE_SIZE)
        {
            calibrationStep = CalibrationStep::END;
        }
        break;
    case CalibrationStep::END:
        display.text("Calibration complete.\n\nClick to continue!");
        average = static_cast<float>(sumMeasurements) / static_cast<float>(numMeasurements);
        scale = static_cast<float>(DEFAULT_CALIBRATION_WEIGHT) / average;
        saveScaleFnc(scale);
        calibrationStep = CalibrationStep::END;
        break;
    }
}

bool ModeCalibration::canSwitchMode()
{
    return calibrationStep == CalibrationStep::END;
}

const char* ModeCalibration::getName()
{
    return "Calibration";
}