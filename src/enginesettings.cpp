#include "enginesettings.h"

#ifdef Q_OS_WASM
constexpr auto SETTINGS_JSON_FILE_NAME = "assets/defaults/engines.json";
#else
constexpr auto SETTINGS_JSON_FILE_NAME = "engines.json";
#endif
constexpr auto AVAILABLE_ENGINES_KEY = "availableEngines";
constexpr auto SELECTED_ENGINE_INDEX_KEY = "selectedEngineIndex";


EngineSettings::EngineSettings()
{
}


void EngineSettings::addEngine(const EngineData &engine)
{
    _availableEngines << engine;
}


void EngineSettings::removeEngine(int index)
{
    if (index >= 0 && index < _availableEngines.count()) {
        _availableEngines.removeAt(index);
    }
}


EngineSettings EngineSettings::load()
{
    EngineSettings settings;
    QFile file(SETTINGS_JSON_FILE_NAME);

    if (!file.open(QIODevice::ReadOnly)) {
        return settings;
    }

    auto json = QJsonDocument::fromJson(file.readAll()).object();
    for (const auto &engine : json.value(AVAILABLE_ENGINES_KEY).toArray()) {
        settings._availableEngines << EngineData::fromJsonObject(engine.toObject());
    }

    settings._currentIndex = json.value(SELECTED_ENGINE_INDEX_KEY).toInt();
    //qDebug() << json;
    return settings;
}


EngineSettings &EngineSettings::instance()
{
    static EngineSettings globalInstance = load();
    return globalInstance;
}


void EngineSettings::save() const
{
    QJsonObject jsonObject;

    jsonObject[AVAILABLE_ENGINES_KEY] = [&]() {
        QJsonArray array;
        for (const auto &eng : _availableEngines) {
            array << eng.toJsonObject();
        }
        return array;
    }();

    jsonObject[SELECTED_ENGINE_INDEX_KEY] = _currentIndex;
    //jsonObject[GENERAL_OPTIONS_KEY] = QJsonObject::fromVariantMap(_generalOptions);

    // ファイル書き込み
    QFile file(SETTINGS_JSON_FILE_NAME);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "File open error" << SETTINGS_JSON_FILE_NAME;
        return;
    }

    file.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    file.close();
}


EngineSettings::EngineData EngineSettings::currentEngine() const
{
    return getEngine(_currentIndex);
}


EngineSettings::EngineData EngineSettings::getEngine(int index) const
{
    if (index >= 0 && index < _availableEngines.count()) {
        return _availableEngines[index];
    }
    return EngineSettings::EngineData();
}


void EngineSettings::updateEngine(int index, const EngineData &data)
{
    if (index >= 0 && index < _availableEngines.count()) {
        _availableEngines[index] = data;
    }
}


/*
  EngineData class
*/

QJsonObject EngineSettings::EngineData::toJsonObject() const
{
#define SET_JSON_VALUE(obj, variable)                  \
    do {                                               \
        QVariant var;                                  \
        var.setValue(variable);                        \
        obj[#variable] = QJsonValue::fromVariant(var); \
    } while (0)

    QJsonObject jo;
    SET_JSON_VALUE(jo, name);
    SET_JSON_VALUE(jo, author);
    SET_JSON_VALUE(jo, path);
    SET_JSON_VALUE(jo, options);
    SET_JSON_VALUE(jo, types);
    return jo;
}


EngineSettings::EngineData EngineSettings::EngineData::fromJsonObject(const QJsonObject &object)
{
    EngineSettings::EngineData data;
    data.name = object["name"].toString();
    data.author = object["author"].toString();
    data.path = object["path"].toString();
    data.options = object["options"].toVariant().toMap();
    data.types = object["types"].toVariant().toMap();
    return data;
}


/* 利用可能なエンジンのリスト
{
    "availableEngines": [
        {
            "name": "engine1",
            "path": "xxxx/hoge",
            "options": {
                "EvalDir": "fuga",
                "BookDir": "fuga",
                "option1": 1,
                "option2": 3
            },
            "defaultOptions": {
                "option1": 1,
                "option2": 1
            }
        },
        {
            "name": "engine2",
            "path": "xxxx/foo",
            "options": {
                "EvalDir": "fuga",
                "BookDir": "fuga",
                "option1": 16,
                "option2": 32
            },
            "defaultOptions": {
                "option1": 1,
                "option2": 1
            }
        }
    ],
    "selectedEngine": 0,
    "generalOptions": {
        "option1": 0,
        "option2": 0,
    }
}
*/
