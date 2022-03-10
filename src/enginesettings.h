#pragma once
#include <QtCore>


class EngineSettings {
public:
    class EngineData {
    public:
        QString name;
        QString author;
        QString path;
        QVariantMap options;  // <QString, QVariant>
        QVariantMap types;  // type of option <QString, QMetaType::Type>

        EngineData() {}
        EngineData(const EngineData &) = default;
        EngineData &operator=(const EngineData &) = default;
        QJsonObject toJsonObject() const;
        static EngineData fromJsonObject(const QJsonObject &object);
    };

    QList<EngineData> availableEngines() const { return _availableEngines; }
    int availableEngineCount() const { return _availableEngines.count(); }
    EngineData currentEngine() const;
    EngineData getEngine(int index) const;
    void updateEngine(int index, const EngineData &data);
    int currentIndex() const { return _currentIndex; }
    void setCurrentIndex(int index) { _currentIndex = index; }
    void addEngine(const EngineData &engine);
    void removeEngine(int index);
    //QVariantMap &generalOptions() { return _generalOptions; }
    void save() const;

    static EngineSettings &instance();

private:
    EngineSettings();
    static EngineSettings load();

    QList<EngineData> _availableEngines;
    int _currentIndex {0};
    //QVariantMap _generalOptions;
};
