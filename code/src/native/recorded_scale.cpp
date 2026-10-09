#ifdef NATIVE

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "native/recorded_scale.h"
#include "millis.h"

static const std::string FIRST_LINE = "# coffee-scale recording";
static const std::string COLUMNS = "ms,event,value";

static bool parseUnsigned(const std::string &text, unsigned long &value)
{
    if (text.empty() || text[0] == '-' || text[0] == '+')
    {
        return false;
    }
    char *end;
    errno = 0;
    value = strtoul(text.c_str(), &end, 10);
    return *end == '\0' && errno == 0;
}

static bool parseLong(const std::string &text, long &value)
{
    if (text.empty())
    {
        return false;
    }
    char *end;
    errno = 0;
    value = strtol(text.c_str(), &end, 10);
    return *end == '\0' && errno == 0;
}

static std::vector<std::string> split(const std::string &line)
{
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ','))
    {
        fields.push_back(field);
    }
    if (!line.empty() && line.back() == ',')
    {
        fields.push_back(""); // getline drops an empty last field
    }
    return fields;
}

RecordedScale::RecordedScale(const std::string &path)
{
    int lineNumber = 0;
    auto fail = [&](const std::string &reason) {
        throw std::runtime_error(path + ":" + std::to_string(lineNumber) + ": " + reason);
    };

    std::ifstream file(path);
    if (!file)
    {
        fail("cannot open the file");
    }

    std::map<std::string, std::string> header;
    bool inData = false;
    unsigned long lastMs = 0;
    std::string line;
    while (std::getline(file, line))
    {
        lineNumber++;
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (lineNumber == 1)
        {
            if (line != FIRST_LINE)
            {
                fail("not a recording, the first line must be \"" + FIRST_LINE + "\"");
            }
            continue;
        }

        if (!inData)
        {
            if (line.rfind("# ", 0) == 0)
            {
                size_t colon = line.find(": ");
                if (colon != std::string::npos)
                {
                    header[line.substr(2, colon - 2)] = line.substr(colon + 2);
                }
                continue;
            }
            if (line != COLUMNS)
            {
                fail("expected the column line \"" + COLUMNS + "\"");
            }
            const std::string &gramsPerCount = header["grams_per_count"];
            char *end;
            scale = strtof(gramsPerCount.c_str(), &end);
            if (gramsPerCount.empty() || *end != '\0' || scale == 0)
            {
                fail("no valid grams_per_count in the header");
            }
            firmwareVersion = header["firmware"];
            inData = true;
            continue;
        }

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> fields = split(line);
        if (fields.size() != 3)
        {
            fail("expected 3 fields: " + COLUMNS);
        }

        unsigned long ms;
        if (!parseUnsigned(fields[0], ms))
        {
            fail("ms is not a number: \"" + fields[0] + "\"");
        }
        if (ms < lastMs)
        {
            fail("ms goes backwards");
        }
        lastMs = ms;

        if (fields[1] == "sample")
        {
            long value;
            if (!parseLong(fields[2], value))
            {
                fail("sample without a numeric value");
            }
            samples.emplace_back(ms, value);
        }
        else if (fields[1] == "click")
        {
            clickTimes.push_back(ms);
        }
        // other events are from newer recordings, they are skipped
    }

    if (!inData)
    {
        fail("the recording ends before the column line \"" + COLUMNS + "\"");
    }
}

void RecordedScale::update()
{
    if (!started)
    {
        started = true;
        startTime = now();
    }
}

unsigned long RecordedScale::elapsed() const { return started ? now() - startTime : 0; }

bool RecordedScale::isReady()
{
    update();
    return next < samples.size() && samples[next].first <= elapsed();
}

long RecordedScale::read()
{
    update();
    while (next < samples.size() && samples[next].first <= elapsed())
    {
        last = samples[next].second;
        next++;
    }
    return last;
}

#endif
