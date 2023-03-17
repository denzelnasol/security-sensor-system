#ifndef DANGER_ANAYLZER_H_
#define DANGER_ANAYLZER_H_

#define DANGER_ANALYZER_DEFAULT_THRESHOLD 50

void DangerAnalyzer_start(void);

void DangerAnalyzer_setThreshold(int dangerThreshold);
void DangerAnalyzer_resetThreshold(void);
void DangerAnalyzer_resetDangerLevel(void);

double DangerAnalyzer_getDangerLevel(void);

void DangerAnalyzer_stop(void);

#endif