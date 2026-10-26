#pragma once

#include <stddef.h>

#include <string>

#include "recorder.h"

/** Saved recordings as numbered CSV files in one directory, oldest first. Plain stdio and dirent, so it runs natively and on LittleFS. */
class RecordingStore
{
public:
    explicit RecordingStore(std::string dir);
    /** Creates the directory if missing, removes a leftover saving.tmp. False if the directory can't be used. */
    bool begin();
    /** Number of NNNN.csv files; other files don't count. */
    size_t count() const;
    /** Writes saving.tmp, renames it to the next NNNN.csv. False on any write, close or rename error; then nothing is left behind. */
    bool save(const Recorder &recorder, const char *firmware, float gramsPerCount);
    /** Full path of the lowest-numbered recording, empty if there is none. */
    std::string oldest() const;
    bool remove(const std::string &path);

private:
    std::string path(const std::string &name) const { return dir + "/" + name; }
    /** Calls fn with the number of each NNNN.csv file. */
    template <typename Fn>
    void forEachNumber(Fn fn) const;

    std::string dir;
};
