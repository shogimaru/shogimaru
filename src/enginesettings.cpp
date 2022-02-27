#include "enginesettings.h"


constexpr auto SETTINGS_JSON_FILE_NAME = "engines.json";
constexpr auto AVAILABLE_ENGINES_KEY = "availableEngines";
constexpr auto SELECTED_ENGINES_KEY = "selectedEngine";
constexpr auto GENERAL_OPTIONS_KEY = "generalOptions";


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
        // デフォルト値
        int con = std::thread::hardware_concurrency();  // コア（スレッド）数
#ifdef Q_OS_WASM
        int threads = std::max((int)std::round(con * 0.8), 2);  // 80%
#else
        int threads = std::max(con, 2);
#endif
        settings._generalOptions.insert(QString("Threads"), QString::number(threads));
        settings._generalOptions.insert(QString("USI_Hash"), QString("256"));
        return settings;
    }

    auto json = QJsonDocument::fromJson(file.readAll()).object();
    qDebug() << "load" << json;
    for (const auto &engine : json.value(AVAILABLE_ENGINES_KEY).toArray()) {
        settings._availableEngines << EngineData::fromJsonObject(engine.toObject());
    }

    settings._selectedEngine = json.value(SELECTED_ENGINES_KEY).toInt();
    settings._generalOptions = json.value(GENERAL_OPTIONS_KEY).toObject().toVariantMap();
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

    jsonObject[SELECTED_ENGINES_KEY] = _selectedEngine;
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
