#pragma once
#include <QtCore>


class EngineSettings {
public:
    class EngineData {
    public:
        QString name;
        QString path;
        QVariantMap options;
        QVariantMap defaultOptions;

        QJsonObject toJsonObject() const;
        static EngineData fromJsonObject(const QJsonObject &object);
    };

    QList<EngineData> availableEngines() const { return _availableEngines; }
    void addEngine(const EngineData &engine);
    void removeEngine(int index);
    QVariantMap &generalOptions() { return _generalOptions; }
    void save() const;

    static EngineSettings &instance();

private:
    EngineSettings() { }
    static EngineSettings load();

    QList<EngineData> _availableEngines;
    int _selectedEngine {0};
    QVariantMap _generalOptions;
};
