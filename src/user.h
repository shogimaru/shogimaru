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
    int illegal() const { return _illegal; }
    void setIllegal(int illegal) { _illegal = illegal; }
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
    bool soundEnable() const { return _soundEnable; }
    void setSoundEnable(bool enable) { _soundEnable = enable; }
    int pieceType() const { return _pieceType; }
    void setPieceType(int type) { _pieceType = type; }
    int version() const { return _version; }
    void setVersion(int version) { _version = version; }

    bool isEmpty() const { return _nickname.isEmpty() && !_rating && !_wins && !_losses && !_draws; }
    bool save();
    static User &load();

private:
    User();
    User(const User &) = delete;
    User &operator=(const User &) = delete;

    QString _nickname;
    int _rating {0};
    int _wins {0};
    int _losses {0};
    int _draws {0};
    int _illegal {0};
    int _scale {80};  // ??????????????????(%)
    int _byoyomi {0};  // ?????????
    int _basicTime {0};  // ????????????(???)
    int _analysisTimeSeconds {-10};  // ????????????(???/???)
    qint64 _analysisNodes {-1000000000};  // ???????????????
    int _analysisDepth {-23};  // ????????????
    bool _soundEnable {true};  // ??????????????????
    int _pieceType {1};  // ?????????
    int _version {1};
};
