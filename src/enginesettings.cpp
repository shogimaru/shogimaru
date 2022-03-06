#include "enginesettings.h"

#ifdef Q_OS_WASM
constexpr auto SETTINGS_JSON_FILE_NAME = "assets/defaults/engines.json";
#else
constexpr auto SETTINGS_JSON_FILE_NAME = "engines.json";
#endif
constexpr auto AVAILABLE_ENGINES_KEY = "availableEngines";
constexpr auto SELECTED_ENGINE_INDEX_KEY = "selectedEngineIndex";
constexpr auto GENERAL_OPTIONS_KEY = "generalOptions";


EngineSettings::EngineSettings()
{
// #ifdef Q_OS_WASM
//     EngineData data;
//     data.name = QLatin1String("YaneuraOu NNUE KP256");
//     _availableEngines << data;
// #endif
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
//         // デフォルト値
//         int con = std::thread::hardware_concurrency();  // コア（スレッド）数
// #ifdef Q_OS_WASM
//         int threads = std::max((int)std::round(con * 0.8), 2);  // 80%
// #else
//         int threads = std::max(con, 2);
// #endif
//         settings._generalOptions.insert(QLatin1String("Threads"), threads);
//         settings._generalOptions.insert(QLatin1String("USI_Hash"), 256);
// #ifdef Q_OS_WASM
//         // WASMでのデフォルト
//         settings._generalOptions.insert(QLatin1String("NetworkDelay"), 300);  // ネットワーク遅延
//         settings._generalOptions.insert(QLatin1String("NetworkDelay2"), 600);  // 切れ負けになる場合のネットワーク遅延
// #endif
        return settings;
    }

    auto json = QJsonDocument::fromJson(file.readAll()).object();
    for (const auto &engine : json.value(AVAILABLE_ENGINES_KEY).toArray()) {
        settings._availableEngines << EngineData::fromJsonObject(engine.toObject());
    }

    settings._currentIndex = json.value(SELECTED_ENGINE_INDEX_KEY).toInt();
    settings._generalOptions = json.value(GENERAL_OPTIONS_KEY).toObject().toVariantMap();
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
    jsonObject[GENERAL_OPTIONS_KEY] = QJsonObject::fromVariantMap(_generalOptions);

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
    SET_JSON_VALUE(jo, path);
    SET_JSON_VALUE(jo, options);
    SET_JSON_VALUE(jo, defaultOptions);
    return jo;
}


EngineSettings::EngineData EngineSettings::EngineData::fromJsonObject(const QJsonObject &object)
{
    EngineSettings::EngineData data;
    data.name = object["name"].toString();
    data.path = object["path"].toString();
    data.options = object["options"].toVariant().toMap();
    data.defaultOptions = object["defaultOptions"].toVariant().toMap();
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
