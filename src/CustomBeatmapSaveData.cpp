#include "CustomBeatmapSaveData.h"

void Il2CppNamespace::CustomBeatmapSaveData::ctor(List_1<BeatmapSaveData::EventData*>* events, 
                                                  List_1<BeatmapSaveData::NoteData*>* notes, 
                                                  List_1<BeatmapSaveData::ObstacleData*>* obstacles) {
    this->events = events;
    this->notes = notes;
    this->obstacles = obstacles;
}

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapSaveData);

void Il2CppNamespace::CustomBeatmapSaveData_NoteData::ctor(float time, int lineIndex, NoteLineLayer lineLayer, BeatmapSaveData::NoteType type, NoteCutDirection cutDirection) {
    this->time = time;
    this->lineIndex = lineIndex;
    this->lineLayer = lineLayer;
    this->type = type;
    this->cutDirection = cutDirection;
}

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapSaveData_NoteData);

void Il2CppNamespace::CustomBeatmapSaveData_ObstacleData::ctor(float time, int lineIndex, ObstacleType type, float duration, int width) {
    this->time = time;
    this->lineIndex = lineIndex;
    this->type = type;
    this->duration = duration;
    this->width = width;
}

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapSaveData_ObstacleData);

void Il2CppNamespace::CustomBeatmapSaveData_EventData::ctor(float time, BeatmapSaveData::BeatmapEventType type, int value) {
    this->time = time;
    this->type = type;
    this->value = value;
}

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapSaveData_EventData);