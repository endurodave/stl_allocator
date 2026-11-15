// stl_allocator demo: C++ standard library containers whose memory comes
// from xallocator's fixed blocks instead of the global heap. Results are
// printed on Serial at 115200. Needs a board with a full C++ standard library
// (not 8-bit AVR).
//
// @see https://github.com/endurodave/stl_allocator

#include <xlist.h>
#include <xmap.h>
#include <xset.h>
#include <xstring.h>
#include <xmake_shared.h>
#include <cstdio>

static void Log(const char* msg)
{
    Serial.println(msg);
}

struct Reading
{
    Reading(int id, int value) : id(id), value(value) {}
    int id;
    int value;
};

static bool Demo()
{
    char buf[96];
    bool ok = true;

    // xstring: std::basic_string with stl_allocator
    xstring name("stl_");
    name += "allocator";
    snprintf(buf, sizeof(buf), "xstring: \"%s\"", name.c_str());
    Log(buf);
    ok = ok && name == "stl_allocator";

    // xlist: each node is a fixed block
    xlist<int> list;
    for (int i = 1; i <= 10; i++)
        list.push_back(i * i);
    int sum = 0;
    for (int v : list)
        sum += v;
    snprintf(buf, sizeof(buf), "xlist: %u items, sum %d", (unsigned)list.size(), sum);
    Log(buf);
    ok = ok && sum == 385;

    // xmap with xstring keys: nodes and strings both use fixed blocks
    xmap<xstring, int> ports;
    ports["http"] = 80;
    ports["https"] = 443;
    ports["mqtt"] = 1883;
    snprintf(buf, sizeof(buf), "xmap: mqtt port %d", ports["mqtt"]);
    Log(buf);
    ok = ok && ports.size() == 3 && ports["https"] == 443;

    // xset keeps its keys sorted, like std::set
    xset<int> set = { 5, 1, 4, 1, 3 };
    snprintf(buf, sizeof(buf), "xset: %u unique, first %d", (unsigned)set.size(), *set.begin());
    Log(buf);
    ok = ok && set.size() == 4 && *set.begin() == 1;

    // xmake_shared: object and control block in one fixed block
    auto reading = xmake_shared<Reading>(7, 21);
    snprintf(buf, sizeof(buf), "xmake_shared: Reading %d = %d", reading->id, reading->value);
    Log(buf);
    ok = ok && reading->id == 7 && reading->value == 21;

    // Containers grow and shrink many times without fragmenting the heap
    for (int round = 0; round < 100; round++)
    {
        xlist<xstring> words;
        for (int i = 0; i < 20; i++)
            words.push_back(xstring(1 + (round + i) % 40, 'x'));
        words.clear();
    }

    return ok;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Log("stl_allocator demo");
    Log(Demo() ? "stl_allocator demo PASSED" : "stl_allocator demo FAILED");
}

void loop()
{
}
