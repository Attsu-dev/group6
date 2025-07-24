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

class Group6 : public Player {
  /* 変数 */
  CardSet pile;                    // 現在の場のカード
  CardSet pile_history;            // 今までに捨てられたすべてのカードの集合
  CardSet unknownCards;            // まだ見えていないカードの集合
  std::vector<CardSet> validSets;  // 出すことのできるカードの集合の配列
  GameStatus gamestatus;           // 現在のゲームステータス
  int lastSubmiterIndex;           // 最後にカードを出したプレイヤの添字
  int sumCardsNum;

  /* 定数 */
  CardSet rank_set[16];  // 数字ごとのカード集合
  Card JOKER;            // ジョーカー
  CardSet PASS;          // パスするときに返す

  /*------------------ 自作ツール ---------------------*/
  // カードセットのポイントを返す（親の場合）
  int getPoint_myTurn(const CardSet& cs);

  // カードセットのポイントを返す（子の場合）
  int getPoint_NotMyTurn(const CardSet& cs);

  // ポイントが一番多い選択肢を返す
  CardSet getHandByPoint();

  // 勝ち確定ならば出すべきカードセットを返す
  CardSet win100();

  // 出せるカードセットの選択肢を配列で返す
  std::vector<CardSet> getValidSets();

  // まだ見えていないカードの集合を返す
  CardSet getUnknownCards();

  // CardSetの中で、数字がrankのカードの集合を返す
  CardSet filterByRank(const CardSet& cs, int rank);

  // CardSetの中で、最弱のrankを返す
  Card getWeakestCard(CardSet cs);

  // csの中から、nowに重ねて出せる最強のカードセットを返す
  CardSet nextMax(const CardSet& cs, const CardSet& now);

  // csを出した場合、確実に自分のターンが回ってくるかどうか
  bool is_strongest(const CardSet& cs);

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
