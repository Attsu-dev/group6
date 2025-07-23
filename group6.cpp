//
//  group6.cpp
//  PlayingCard
//
//  Created by 西村 淳志 on 25/07/07.
//  Modified by

#include <iostream>
#include <string>

#include "card.h"
#include "cardset.h"
#include "gamestatus.h"
#include "group6.h"
#include "player.h"

void Group6::ready() {
  pile_history.clear();
  hand.sort(myCardCmp);
}

// 出す
bool Group6::follow(const GameStatus& gstat, CardSet& cs) {
  gamestatus = gstat;
  pile = gstat.pile;

  auto valid = getValidSets();
  CardSet unk = unknownCards();
  std::cout << "\n-----デバッグ用出力-----\n";
  std::cout << "敵のカード: " << unk << std::endl;
  std::cout << "捨てられたカード: " << pile_history << std::endl;
  std::cout << "場のカード: " << pile << std::endl;
  std::cout << "手札: " << hand << std::endl;
  for (auto v : valid) {
    std::cout << "選択肢:" << v
              << ", 次出される可能性のあるもの:" << nextMax(unk, v)
              << std::endl;
  }

  CardSet win = win100();
  if (!win.isEmpty()) {
    cs = win;
    hand.remove(win);
    if (!hand.isEmpty())
      std::cout << "勝ち確定!\n";
    return true;
  }
  cs = getHandByPoint();
  // std::cout << "\n選択肢\n";
  // for (auto a : getValidSets()) {
  //   std::cout << "{" << a << "} ";
  // }
  std::cout << "\n選ばれたもの: ";
  hand.remove(cs);
  return true;
}

// 毎ターン終了時の処理
bool Group6::approve(const GameStatus& gstat) {
  pile_history.insert(gstat.pile);
  pile_history.sort();
  return true;
}

// CardSetの中で、数字がrankのカードの集合を返す
CardSet Group6::filterByRank(const CardSet& cs, int rank) {
  return cs.intersection(rank_set[rank]);
}

// まだ見えていないカードの集合を返す
CardSet Group6::unknownCards() {
  CardSet cs;
  cs.setupDeck();
  cs.remove(hand);
  cs.remove(pile_history);
  return cs;
}

// 数字がrankのカードで、まだ見えていないカードの集合を返す
CardSet Group6::unknownCards(int rank) {
  return unknownCards().intersection(rank_set[rank]);
}

// csの中から、nowに重ねて出せる最強のカードセットを返す
CardSet Group6::nextMax(const CardSet& cs, const CardSet& now) {
  int size = now.size();
  CardSet ans;

  if (now.size() == 1 && now.includes(JOKER))
    return CardSet();

  // 1枚提出する
  if (size == 1) {
    // ジョーカーがある
    if (cs.includes(JOKER)) {
      ans.insert(JOKER);
      return ans;
    }
    // ジョーカーがない
    for (int i = 0; i < (13 - now[0].strength()); i++) {
      int rank = (14 - i) % 13 + 1;
      CardSet a = filterByRank(cs, rank);
      if (!a.isEmpty()) {
        ans.insert(a.at(0));
        return ans;
      }
    }
    // 2枚以上提出する
  } else {
    bool has_joker = cs.includes(JOKER);
    int now_strength = now.at(0).strength();
    for (int i = 0; i < now.size(); i++) {
      now_strength = std::min(now_strength, now.at(i).strength());
    }
    for (int i = 0; i < (13 - now_strength); i++) {
      int rank = (14 - i) % 13 + 1;
      CardSet a = filterByRank(cs, rank);
      // 数字のみでsize枚出せるとき
      if (a.size() >= size) {
        for (int j = 0; j < size; j++) {
          ans.insert(a.at(j));
        }
        return ans;
      }
      // joker込みでsize枚出せるとき
      if (has_joker && (a.size() + 1 == size)) {
        ans.insert(a);
        ans.insert(JOKER);
        return ans;
      }
    }
  }
  return ans;
}

// 勝ち確定ならば出すべきカードセットを返す
CardSet Group6::win100() {
  auto valid = getValidSets();
  CardSet unknown = unknownCards();

  // 出して上がりなら出す
  if (valid.size() == 1 && valid[0].equal(hand)) {
    return hand;
  }

  // 無敵→無敵→...→無敵→ラスト で勝てる場合
  auto win100_myturn = [&](const CardSet& c) {
    int joker_num = c.includes(JOKER);
    std::vector<CardSet> ok_set, ng_set;
    for (int rank = 1; rank <= 13; rank++) {
      CardSet a = filterByRank(c, rank);
      if (a.isEmpty())
        continue;
      if (nextMax(unknown, a).isEmpty()) {
        ok_set.push_back(a);
      } else {
        ng_set.push_back(a);
      }
    }
    if (ng_set.size() >= 3) {
      return CardSet();
    }
    if (ng_set.size() <= 1) {
      if (ok_set.size() >= 1) {
        if (joker_num) {
          ok_set[0].insert(JOKER);
        }
        return ok_set[0];
      } else {
        return ng_set[0];
      }
    }
    // ng_set.size() == 2
    if (joker_num == 0)
      return CardSet();
    CardSet ng0 = ng_set[0];
    CardSet ng1 = ng_set[1];

    ng0.insert(JOKER);
    if (nextMax(unknown, ng0).isEmpty()) {
      return ng0;
    }
    ng1.insert(JOKER);
    if (nextMax(unknown, ng1).isEmpty()) {
      return ng1;
    }
    return CardSet();
  };

  if (pile.isEmpty()) {
    CardSet w = win100_myturn(hand);
    if (!w.isEmpty())
      return w;
  } else {
    for (auto v : valid) {
      CardSet hand_copy(hand);
      hand_copy.remove(v);
      if (nextMax(unknown, v).isEmpty() &&
          !win100_myturn(hand_copy).isEmpty()) {
        return v;
      }
    }
  }

  return CardSet();
}

// 出せるカードセットの配列を返す
std::vector<CardSet> Group6::getValidSets() {
  // 自分が親なら
  std::vector<CardSet> sets;
  int size = pile.size();
  if (size == 0) {
    for (int rank = 1; rank <= 13; rank++) {
      CardSet a = filterByRank(hand, rank);
      CardSet c;
      for (int i = 0; i < a.size(); i++) {
        c.insert(a.at(i));
        sets.push_back(c);
      }
    }
    if (hand.includes(JOKER)) {
      CardSet c;
      c.insert(JOKER);
      sets.push_back(c);
    }
    // 自分が親でなければ
  } else {
    for (int rank = 1; rank <= 13; rank++) {
      if ((rank + 10) % 13 <= (pile.at(0).getRank() + 10) % 13) {
        continue;
      }
      CardSet a = filterByRank(hand, rank);
      CardSet c;
      if (size <= a.size()) {
        for (int i = 0; i < size; i++) {
          c.insert(a.at(i));
        }
        sets.push_back(c);
      } else if (hand.includes(JOKER) && (size >= 2) &&
                 (size == a.size() + 1)) {
        c.insert(a);
        c.insert(JOKER);
        sets.push_back(c);
      }
    }
    if (size == 1 && hand.includes(JOKER)) {
      CardSet c;
      c.insert(JOKER);
      sets.push_back(c);
    }
  }
  return sets;
}

// ポイントが一番多い選択肢を返す
CardSet Group6::getHandByPoint() {
  std::vector<CardSet> sets = getValidSets();

  if (sets.empty()) {
    return CardSet();
  }

  std::vector<int> points(sets.size());

  for (int i = 0; i < (int)sets.size(); i++) {
    points[i] = getPoint(sets[i]);
  }

  sets.push_back(CardSet());
  points.push_back(-1000000);

  auto it = std::max_element(points.begin(), points.end());
  int index = std::distance(points.begin(), it);

  return sets[index];
}

// csを出した場合、確実に自分のターンが回ってくるかどうか
bool Group6::is_strongest(const CardSet& cs) {
  return nextMax(unknownCards(), cs).isEmpty();
}

// カードセットのポイントを返す
int Group6::getPoint(const CardSet& cs) {
  int point = 0;
  point -= cs.size();                  // カード枚数が少ないほど優先
  point -= cs.at(0).strength() * 100;  // カードが弱いほど優先

  // ペアを崩すのに番を取れない手はカスなので最弱
  if (cs.size() < filterByRank(hand, cs.at(0).rank()).size()) {
    if (!is_strongest(cs))
      return INT32_MIN;
  }
  return point;
}