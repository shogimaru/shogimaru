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

        EngineData() {}
        EngineData(const EngineData &) = default;
        EngineData &operator=(const EngineData &) = default;
        QJsonObject toJsonObject() const;
        static EngineData fromJsonObject(const QJsonObject &object);
    };

    QList<EngineData> availableEngines() const { return _availableEngines; }
    int availableEngineCount() const { return _availableEngines.count(); }
    int selectedEngine() const { return _selectedEngine; }
    void setSelectedEngine(int index) { _selectedEngine = index; }
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

//Q_DECLARE_METATYPE(EngineSettings::EngineData)