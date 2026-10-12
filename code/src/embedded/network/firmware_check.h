#ifndef NATIVE

#pragma once

/**
 * Rollback of a firmware that doesn't start properly. A firmware fresh from
 * an update boots "pending verify"; if the scale reboots before markValid(),
 * the bootloader returns to the firmware before. When to call markValid()
 * decides FirmwareProbation.
 */
namespace FirmwareCheck
{
    /** Marks the running firmware as working, if it is on probation. */
    void markValid();
    /** True while the running firmware is fresh from an update and not marked working yet. */
    bool onProbation();
    /** True once after an update was rolled back, false from then on until the next update. */
    bool takeRolledBack();
    /** Before booting a downloaded firmware: lets takeRolledBack() report its rollback. */
    void expectNewFirmware();
}

#endif
