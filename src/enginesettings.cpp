#include "enginesettings.h"
#include "file.h"

#ifdef Q_OS_WASM
constexpr auto DEFAULT_SETTINGS_JSON_FILE_NAME = "assets/defaults/wasm_engines.json";
#endif
constexpr auto SETTINGS_JSON_FILE_NAME = "engines.json";
constexpr auto AVAILABLE_ENGINES_KEY = "availableEngines";
constexpr auto SELECTED_ENGINE_INDEX_KEY = "selectedEngineIndex";
constexpr int  SETTINGS_JSON_VERSION = 2;


static QString settingsJsonPath()
{
    // 書き込みが可能なパス
    return QDir(maru::appLocalDataLocation()).absoluteFilePath(SETTINGS_JSON_FILE_NAME);
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


EngineSettings EngineSettings::loadJsonFile(const QString &path)
{
    File file(path);

    if (!file.exists()) {
        qWarning() << "No such file: " << path;
        return EngineSettings();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "File open error: " << path;
        return EngineSettings();
    }

    auto data = file.readAll();
    return loadJsonData(data);
}


EngineSettings EngineSettings::loadJsonData(const QByteArray &data)
{
    EngineSettings settings;
    auto json = QJsonDocument::fromJson(data).object();

#ifdef Q_OS_WASM
    if (json["version"] != SETTINGS_JSON_VERSION) {
        // WASM版の思考エンジンが変わったら、バージョンを変えて再読み込みさせる
        return settings;
    }
#endif

    for (const auto &engine : json.value(AVAILABLE_ENGINES_KEY).toArray()) {
        settings._availableEngines << EngineData::fromJsonObject(engine.toObject());
    }

    settings._currentIndex = json.value(SELECTED_ENGINE_INDEX_KEY).toInt();
    return settings;
}


EngineSettings EngineSettings::load()
{
    EngineSettings settings = loadJsonFile(settingsJsonPath());

#ifdef Q_OS_WASM
    if (settings._availableEngines.isEmpty()) {  // 初回のみ
        settings = loadJsonFile(maru::appResourcePath(DEFAULT_SETTINGS_JSON_FILE_NAME));
        if (settings.availableEngines().isEmpty()) {
            qCritical() << "Error load settings:" << maru::appResourcePath(DEFAULT_SETTINGS_JSON_FILE_NAME);
            return settings;
        }

        QVariantMap options;
        auto info = Engine::getEngineInfo(QString());
        for (auto it = info.options.begin(); it != info.options.end(); ++it) {
            options.insert(it.key(), it.value().defaultValue);
        }
        setCustomOptions(options);  // カスタムオプション
        settings._availableEngines[0].options = options;
        qDebug() << "Loaded default json:" << DEFAULT_SETTINGS_JSON_FILE_NAME;
        settings.save();
    }

    QVariantMap types;
    auto info = Engine::getEngineInfo(QString());
    for (auto it = info.options.begin(); it != info.options.end(); ++it) {
        types.insert(it.key(), QVariant((int)it.value().type));
    }
    settings._availableEngines[0].types = types;
#endif
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
    jsonObject["version"] = SETTINGS_JSON_VERSION;

    // ファイル書き込み
    File file(settingsJsonPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "File open error" << file.fileName();
        return;
    }

    file.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    // qDebug() << "save json:" << QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
    file.flush();
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

// 将棋丸用デフォルト値
void EngineSettings::setCustomOptions(QVariantMap &options)
{
    // 読み筋の数
    options["MultiPV"].setValue(5);

    // エンジンスレッド数
    int con = std::thread::hardware_concurrency();  // コア（スレッド）数
    int threads = std::max((int)std::round(con * 0.75), 1);  // 75%
    options["Threads"].setValue(threads);

    options["BookDir"].setValue(maru::appResourcePath("assets/YaneuraOu"));
    options["BookFile"].setValue(maru::appResourcePath("user_book1.db"));
    options["EvalDir"].setValue(maru::appResourcePath("assets/YaneuraOu/nnue-halfkp256"));
    options["FV_SCALE"].setValue(24);
    options["Stochastic_Ponder"].setValue(true);
}

#else

// 将棋丸用デフォルト値
void EngineSettings::setCustomOptions(QVariantMap &options)
{
    // 読み筋の数
    if (options.contains("MultiPV")) {
        options["MultiPV"].setValue(5);
    }

    // エンジンスレッド数
    if (options.contains("Threads")) {
        int con = std::thread::hardware_concurrency();  // コア（スレッド）数
        int threads = std::max((int)std::round(con * 0.8), 1);  // 80%
        options["Threads"].setValue(threads);
    }

    // 確率論的先読み
    if (options.contains("Stochastic_Ponder")) {
        options["Stochastic_Ponder"].setValue(true);
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
    "version": 2
}
*/
