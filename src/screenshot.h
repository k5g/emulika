#ifndef SCREENSHOT_H_INCLUDED
#define SCREENSHOT_H_INCLUDED

#include "sms.h"
#include "string.h"

int savescreenshot(const mastersystem *sms, const string filename);
void takescreenshot(const mastersystem *sms, const string screenshotsdir);

#endif // SCREENSHOT_H_INCLUDED
