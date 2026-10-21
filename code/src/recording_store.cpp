#include "recording_store.h"

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

namespace
{
    const char *TEMP_NAME = "saving.tmp";

    /** Returns the number of a name like 0042.csv, or -1. */
    int numberOf(const char *name)
    {
        int n = 0;
        for (int i = 0; i < 4; i++)
        {
            if (name[i] < '0' || name[i] > '9')
            {
                return -1;
            }
            n = n * 10 + (name[i] - '0');
        }
        return std::string(name + 4) == ".csv" ? n : -1;
    }

    std::string nameOf(int number)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%04d.csv", number);
        return buf;
    }
}

RecordingStore::RecordingStore(std::string dir) : dir(std::move(dir)) {}

template <typename Fn>
void RecordingStore::forEachNumber(Fn fn) const
{
    DIR *d = opendir(dir.c_str());
    if (!d)
    {
        return;
    }
    while (dirent *e = readdir(d))
    {
        int n = numberOf(e->d_name);
        if (n >= 0)
        {
            fn(n);
        }
    }
    closedir(d);
}

bool RecordingStore::begin()
{
    struct stat st;
    if (stat(dir.c_str(), &st) != 0 && mkdir(dir.c_str(), 0755) != 0)
    {
        return false;
    }
    ::remove(path(TEMP_NAME).c_str());
    return stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

size_t RecordingStore::count() const
{
    size_t n = 0;
    forEachNumber([&n](int) { n++; });
    return n;
}

bool RecordingStore::save(const Recorder &recorder, const char *firmware, float gramsPerCount)
{
    int highest = 0;
    forEachNumber([&highest](int n) { highest = n > highest ? n : highest; });
    if (highest >= 9999)
    {
        return false;
    }

    std::string temp = path(TEMP_NAME);
    FILE *f = fopen(temp.c_str(), "wb");
    if (!f)
    {
        return false;
    }
    bool ok = true;
    recorder.writeCsv(firmware, gramsPerCount, [&](const char *data, size_t length) {
        if (ok && fwrite(data, 1, length, f) != length)
        {
            ok = false;
        }
    });
    if (fclose(f) != 0)
    {
        ok = false;
    }
    if (ok && rename(temp.c_str(), path(nameOf(highest + 1)).c_str()) != 0)
    {
        ok = false;
    }
    if (!ok)
    {
        ::remove(temp.c_str());
    }
    return ok;
}

std::string RecordingStore::oldest() const
{
    int lowest = -1;
    forEachNumber([&lowest](int n) { lowest = (lowest < 0 || n < lowest) ? n : lowest; });
    return lowest < 0 ? "" : path(nameOf(lowest));
}

bool RecordingStore::remove(const std::string &file)
{
    return ::remove(file.c_str()) == 0;
}
