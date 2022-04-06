#include "enginesettings.h"

#ifdef Q_OS_WASM
constexpr auto DEFAULT_SETTINGS_JSON_FILE_NAME = "assets/defaults/engines.json";
#endif
constexpr auto SETTINGS_JSON_FILE_NAME = "engines.json";
constexpr auto AVAILABLE_ENGINES_KEY = "availableEngines";
constexpr auto SELECTED_ENGINE_INDEX_KEY = "selectedEngineIndex";


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


EngineSettings EngineSettings::loadJson(const QString &path)
{
    EngineSettings settings;
    QFile file(path);

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


EngineSettings EngineSettings::load()
{
    EngineSettings settings = loadJson(SETTINGS_JSON_FILE_NAME);

    if (settings._availableEngines.isEmpty()) {
#ifdef Q_OS_WASM
        settings = loadJson(DEFAULT_SETTINGS_JSON_FILE_NAME);
        if (settings.availableEngines().isEmpty()) {
            qCritical() << "Error load settings:" << DEFAULT_SETTINGS_JSON_FILE_NAME;
            return settings;
        }

        QVariantMap options;
        auto info = Engine::getEngineInfo(QString());
        for (auto it = info.options.begin(); it != info.options.end(); ++it) {
            options.insert(it.key(), it.value().defaultValue);
            //qDebug() << it.key() << it.value().value;
        }
        setCustomOptions(options);  // カスタムオプション
        settings._availableEngines[0].options = options;
        qDebug() << "Loaded default json:" << DEFAULT_SETTINGS_JSON_FILE_NAME;
        settings.save();
#endif
    }
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
    jsonObject["version"] = 1;

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


#ifdef Q_OS_WASM

void EngineSettings::setCustomOptions(QVariantMap &options)
{
    // 将棋丸用デフォルト値
    options["MultiPV"].setValue(5);

    // エンジンスレッド数
    int con = std::thread::hardware_concurrency();  // コア（スレッド）数
    int threads = std::max((int)std::round(con * 0.8), 1);  // 80%
    options["Threads"].setValue(threads);

    options["BookDir"].setValue(QString("assets/YaneuraOu"));
    options["BookFile"].setValue(QString("user_book1.db"));
    options["EvalDir"].setValue(QString("assets/YaneuraOu/nnue-kp256"));
}

#else

void EngineSettings::setCustomOptions(QVariantMap &options)
{
    // 将棋丸用デフォルト値
    if (options.contains("MultiPV")) {
        options["MultiPV"].setValue(5);
    }

    // エンジンスレッド数
    if (options.contains("Threads")) {
        int con = std::thread::hardware_concurrency();  // コア（スレッド）数
        int threads = std::max((int)std::round(con * 0.8), 1);  // 80%
        options["Threads"].setValue(threads);
    }
}

#endif

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
            "author": "xxxx",
            "options": {
                "EvalDir": "fuga",
                "BookDir": "fuga",
                "option1": 1,
                "option2": 3
            }
        },
        {
            "name": "engine2",
            "path": "xxxx/foo",
            "author": "xxxx",
            "options": {
                "EvalDir": "foo1",
                "BookDir": "foo2",
                "option1": 16,
                "option2": 32
            }
        }
    ],
    "selectedEngineIndex": 0,
    "version": 1
}
*/
