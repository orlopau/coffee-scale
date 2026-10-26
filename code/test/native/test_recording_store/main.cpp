#include <unity.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <string>

#include "recorder.h"
#include "recording_store.h"

static std::string dir;
static std::string recDir;
static RecordingStore *store;

static void removeTree(const std::string &path)
{
    DIR *d = opendir(path.c_str());
    if (d)
    {
        while (dirent *e = readdir(d))
        {
            std::string name = e->d_name;
            if (name != "." && name != "..")
            {
                removeTree(path + "/" + name);
            }
        }
        closedir(d);
        rmdir(path.c_str());
    }
    else
    {
        remove(path.c_str());
    }
}

static bool exists(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static std::string slurp(const std::string &path)
{
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void writeFile(const std::string &path, const char *content)
{
    FILE *f = fopen(path.c_str(), "w");
    fputs(content, f);
    fclose(f);
}

static bool endsWith(const std::string &s, const char *suffix)
{
    std::string x = suffix;
    return s.size() >= x.size() && s.compare(s.size() - x.size(), x.size(), x) == 0;
}

static void fill(Recorder &r)
{
    r.start(0);
    r.addSample(0, 1000);
    r.addSample(100, 1100);
    r.addClick(150);
    r.addTare(180);
    r.addSample(200, 1200);
}

static std::string csvOf(const Recorder &r)
{
    std::string out;
    r.writeCsv("v1.0.0", 0.01f, [&out](const char *d, size_t n) { out.append(d, n); });
    return out;
}

void setUp(void)
{
    char tmpl[] = "/tmp/recording_store_XXXXXX";
    dir = mkdtemp(tmpl);
    recDir = dir + "/recordings";
    store = new RecordingStore(recDir);
}

void tearDown(void)
{
    delete store;
    removeTree(dir);
}

void test_begin_creates_the_directory(void)
{
    TEST_ASSERT_TRUE(store->begin());
    TEST_ASSERT_TRUE(exists(recDir));
    TEST_ASSERT_EQUAL(0, store->count());
    TEST_ASSERT_EQUAL_STRING("", store->oldest().c_str());
}

void test_save_writes_the_csv(void)
{
    Recorder rec;
    fill(rec);
    store->begin();
    TEST_ASSERT_TRUE(store->save(rec, "v1.0.0", 0.01f));
    TEST_ASSERT_EQUAL(1, store->count());
    TEST_ASSERT_TRUE(endsWith(store->oldest(), "/0001.csv"));
    TEST_ASSERT_EQUAL_STRING(csvOf(rec).c_str(), slurp(store->oldest()).c_str());
}

void test_an_empty_recording_saves_its_header(void)
{
    Recorder rec;
    rec.start(0);
    store->begin();
    TEST_ASSERT_TRUE(store->save(rec, "v1.0.0", 0.01f));
    std::string content = slurp(store->oldest());
    TEST_ASSERT_EQUAL_STRING(csvOf(rec).c_str(), content.c_str());
    TEST_ASSERT_EQUAL(0, content.rfind("# coffee-scale recording", 0));
}

void test_oldest_first(void)
{
    Recorder rec;
    fill(rec);
    store->begin();
    for (int i = 0; i < 3; i++)
    {
        TEST_ASSERT_TRUE(store->save(rec, "v1.0.0", 0.01f));
    }
    TEST_ASSERT_TRUE(endsWith(store->oldest(), "0001.csv"));
    TEST_ASSERT_TRUE(store->remove(store->oldest()));
    TEST_ASSERT_TRUE(endsWith(store->oldest(), "0002.csv"));
    TEST_ASSERT_EQUAL(2, store->count());
}

void test_numbers_continue_after_the_highest(void)
{
    Recorder rec;
    fill(rec);
    store->begin();
    store->save(rec, "v1.0.0", 0.01f);
    store->save(rec, "v1.0.0", 0.01f);
    TEST_ASSERT_TRUE(store->remove(recDir + "/0001.csv"));
    TEST_ASSERT_TRUE(store->save(rec, "v1.0.0", 0.01f));
    TEST_ASSERT_FALSE(exists(recDir + "/0001.csv"));
    TEST_ASSERT_TRUE(exists(recDir + "/0002.csv"));
    TEST_ASSERT_TRUE(exists(recDir + "/0003.csv"));
    TEST_ASSERT_TRUE(endsWith(store->oldest(), "0002.csv"));
}

void test_begin_removes_a_leftover_temporary_file(void)
{
    mkdir(recDir.c_str(), 0755);
    writeFile(recDir + "/saving.tmp", "half");
    TEST_ASSERT_TRUE(store->begin());
    TEST_ASSERT_FALSE(exists(recDir + "/saving.tmp"));
    TEST_ASSERT_EQUAL(0, store->count());
}

void test_other_files_dont_count(void)
{
    mkdir(recDir.c_str(), 0755);
    writeFile(recDir + "/notes.txt", "x");
    writeFile(recDir + "/12.csv", "x");
    TEST_ASSERT_TRUE(store->begin());
    TEST_ASSERT_EQUAL(0, store->count());
    TEST_ASSERT_EQUAL_STRING("", store->oldest().c_str());
}

void test_failed_save_keeps_nothing(void)
{
    Recorder rec;
    fill(rec);
    store->begin();
    removeTree(recDir);
    TEST_ASSERT_FALSE(store->save(rec, "v1.0.0", 0.01f));
    TEST_ASSERT_EQUAL(0, store->count());
    TEST_ASSERT_FALSE(exists(recDir + "/saving.tmp"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_begin_creates_the_directory);
    RUN_TEST(test_save_writes_the_csv);
    RUN_TEST(test_an_empty_recording_saves_its_header);
    RUN_TEST(test_oldest_first);
    RUN_TEST(test_numbers_continue_after_the_highest);
    RUN_TEST(test_begin_removes_a_leftover_temporary_file);
    RUN_TEST(test_other_files_dont_count);
    RUN_TEST(test_failed_save_keeps_nothing);
    return UNITY_END();
}
