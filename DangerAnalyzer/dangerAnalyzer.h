#ifndef DANGER_ANAYLZER_H_
#define DANGER_ANAYLZER_H_

#define DANGER_ANALYZER_DEFAULT_THRESHOLD 50

void DangerAnalyzer_start(void);

void DangerAnalyzer_resetDangerLevel(void);
double DangerAnalyzer_getDangerLevel(void);
long long DangerAnalyzer_getNumTriggers(void);

void DangerAnalyzer_stop(void);

#endif