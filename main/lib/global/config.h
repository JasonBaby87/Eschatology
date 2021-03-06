#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <vector>
#include <istream>
#include "type.h"

using namespace std;

/** Shifts all the notes later */
extern Milliseconds globalOffset = Milliseconds();
/** Shifts all the visual later */
extern Milliseconds visualOffset = Milliseconds();

/** The judge windows.
	<p>The smaller the indices the stricter it is. */
extern vector<Nanoseconds> judgeWindows = vector<Nanoseconds>();

extern double displayedBeats = 8;

void loadConfigurations(istream& is);

#include "config.cpp"

#endif // CONFIG_H_INCLUDED
