#pragma once
#include <QDateTime>
#include <QString>


class User {
public:
    ~User() { }

    QString nickname() const { return _nickname; }
    void setNickname(const QString &name) { _nickname = name; }
    int rating() const { return _rating; }
    void setRating(int rating) { _rating = rating; }
    int wins() const { return _wins; }
    void setWins(int wins) { _wins = wins; }
    int losses() const { return _losses; }
    void setLosses(int losses) { _losses = losses; }
    int draws() const { return _draws; }
    void setDraws(int draws) { _draws = draws; }
    int fouls() const { return _fouls; }
    void setFouls(int fouls) { _fouls = fouls; }
    int scale() const { return _scale; }
    void setScale(int scale) { _scale = scale; }
    int byoyomi() const { return _byoyomi; }
    void setByoyomi(int byoyomi) { _byoyomi = byoyomi; }
    int basicTime() const { return _basicTime; }
    void setBasicTime(int basicTime) { _basicTime = basicTime; }
    int analysisTimeSeconds() const { return _analysisTimeSeconds; }
    void setAnalysisTimeSeconds(int seconds) { _analysisTimeSeconds = seconds; }
    qint64 analysisNodes() const { return _analysisNodes; }
    void setAnalysisNodes(qint64 nodes) { _analysisNodes = nodes; }
    int analysisDepth() const { return _analysisDepth; }
    void setAnalysisDepth(int depth) { _analysisDepth = depth; }
    int version() const { return _version; }
    void setVersion(int version) { _version = version; }

    bool isEmpty() const { return _nickname.isEmpty() && !_rating && !_wins && !_losses && !_draws; }
    bool save();
    static User &load();

private:
    User() { }

    QString _nickname;
    int _rating {0};
    int _wins {0};
    int _losses {0};
    int _draws {0};
    int _fouls {0};
    int _scale {80};  // 将棋盤拡縮値
    int _byoyomi {0};
    int _basicTime {0};  // 持ち時間(分)
    int _analysisTimeSeconds {-10};  // 解析時間(秒/手)
    qint64 _analysisNodes {-1000000000};  // 解析ノード
    int _analysisDepth {-23};  // 解析深度
    int _version {1};
};
