//
//  group6.h
//  PlayingCard
//
//  Created by 西村 淳志 on 25/07/07.
//  Modified by

#ifndef _GROUP6_H_
#define _GROUP6_H_

#include <algorithm>
#include <random>

#include "gamestatus.h"
#include "player.h"

//
// すこしルールを覚えたプレイヤーのクラス
// Player クラスを継承して作成するクラスのサンプル
//
class Group6 : public Player {
  CardSet pile_history;  // 今までに捨てられたすべてカードの集合
  CardSet rank_set[16];  // 数字ごとのカード集合
  Card JOKER;
  GameStatus gamestatus;
  CardSet pile;

  /*------------------ 自作ツール ---------------------*/
  // CardSetの中で、数字がrankのカードの集合を返す
  CardSet filterByRank(const CardSet& cs, int rank);

  // まだ見えていないカードの集合を返す
  CardSet unknownCards();

  // 数字がrankのカードで、まだ見えていないカードの集合を返す
  CardSet unknownCards(int rank);

  // csの中から、nowに重ねて出せる最強のカードセットを返す
  CardSet nextMax(const CardSet& cs, const CardSet& now);

  // 自分が親の場合に、勝ち確定ならば出すべきカードセットを返す
  CardSet win100();

  // 出せるカードセットの配列を返す
  std::vector<CardSet> getValidSets();

  // ポイントが一番多い選択肢を返す
  CardSet getHandByPoint();

  // カードセットのポイントを返す
  int getPoint(const CardSet& cs);

 public:
  Group6(const char* name = "Group6") : Player(name) {
    // JOKERの初期化
    JOKER.set(Card::SUIT_JOKER, Card::RANK_JOKER);
    // rank_setの初期化
    for (int num = 1; num <= 13; num++) {
      for (int suit = 1; suit <= 4; suit++) {
        rank_set[num].insert(Card(suit, num));
      }
    }
    rank_set[Card::RANK_JOKER].insert(JOKER);
  }
  ~Group6() {}

  // 思考処理の関数：このクラスで実装する
  void ready(void);
  bool follow(const GameStatus&, CardSet&);
  bool approve(const GameStatus&);

  // 比較関数(ソート用)
  static bool myCardCmp(const Card& a, const Card& b) {
    return a.strength() <= b.strength();
  }
};

#endif
