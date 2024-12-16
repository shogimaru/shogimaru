#pragma once
#include "global.h"
#include <QString>


class Player {
public:
    Player() {}
    Player(maru::PlayerType type, const QString &name, int rating = 0) :
        _type(type),
        _name(name),
        _rating(rating)
    {}
    ~Player() {}
    Player(const Player &other) = default;
    Player &operator=(const Player &other) = default;

    maru::PlayerType type() const { return _type; }
    QString name() const { return _name; }
    void setName(const QString &name) { _name = name; }
    int rating() const { return _rating; }

private:
    maru::PlayerType _type {maru::Computer};
    QString _name;
    int _rating {0};
};
