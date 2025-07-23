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

// カードセットのポイントを返す (負ならば選ばれない)
int Group6::getPoint(const CardSet& cs) {
  int point = 10000;
  point -= cs.size();               // カード枚数が少ないほど優先
  point -= cs[0].strength() * 100;  // カードが弱いほど優先

  // ペアを崩すのに番を取れない手は最弱
  if (cs.size() < filterByRank(hand, cs[0].rank()).size()) {
    if (!is_strongest(cs))
      return -1;
  }
  return point;
}

// カードを出す
bool Group6::follow(const GameStatus& gstat, CardSet& cs) {
  // 使いやすいようにいろいろ保存しておく
  gamestatus = gstat;
  pile = gstat.pile;
  unknownCards = getUnknownCards();
  validSets = getValidSets();

  // 出せるものがないとき、パス
  if (validSets.empty()) {
    cs = PASS;
    return true;
  }

  // 勝ち確定に持っていけるなら、そうする
  CardSet win = win100();
  if (!win.isEmpty()) {
    cs = win;
    hand.remove(win);
    return true;
  }

  // ポイントが高いものを出す
  cs = getHandByPoint();
  hand.remove(cs);
  return true;
}

// ポイントが一番多い選択肢を返す
CardSet Group6::getHandByPoint() {
  std::vector<int> points(validSets.size());
  for (int i = 0; i < (int)validSets.size(); i++) {
    points[i] = getPoint(validSets[i]);
  }

  // パスの選択肢を追加（ポイント = 0）
  validSets.push_back(CardSet());
  points.push_back(0);

  // 最大値のインデックスを求める
  auto it = std::max_element(points.begin(), points.end());
  int index = std::distance(points.begin(), it);

  return validSets[index];
}

// CardSetの中で、数字がrankのカードの集合を返す
CardSet Group6::filterByRank(const CardSet& cs, int rank) {
  return cs.intersection(rank_set[rank]);
}

// CardSetの中で、最弱のカードを返す
Card Group6::getWeakestCard(CardSet cs) {
  if (cs.isEmpty()) {
    return Card();
  }
  cs.sort(myCardCmp);
  return cs[0];
}

// まだ見えていないカードの集合を返す
CardSet Group6::getUnknownCards() {
  CardSet cs;
  cs.setupDeck();
  cs.remove(hand);
  cs.remove(pile_history);
  cs.sort(myCardCmp);
  return cs;
}

// csの中から、nowに重ねて出せる最強のカードセットを返す
CardSet Group6::nextMax(const CardSet& cs, const CardSet& now) {
  int size = now.size();
  CardSet ans;

  // ジョーカーに重ねて出せるものはない
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
        ans.insert(a[0]);
        return ans;
      }
    }
    // 2枚以上提出する
  } else {
    bool has_joker = cs.includes(JOKER);
    int now_strength = getWeakestCard(now).strength();
    for (int i = 0; i < (13 - now_strength); i++) {
      int rank = (14 - i) % 13 + 1;
      CardSet a = filterByRank(cs, rank);
      // 数字のみでsize枚出せるとき
      if (a.size() >= size) {
        for (int j = 0; j < size; j++) {
          ans.insert(a[j]);
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
  // 出して上がりなら出す
  if (validSets.size() == 1 && validSets[0].equal(hand)) {
    return hand;
  }

  // 自分が親で手札cを持っているとき、無敵→...→無敵→ラストで勝てるなら無敵を1個返す関数
  auto win100_myturn = [&](const CardSet& c) {
    bool has_joker = c.includes(JOKER);

    // rankごとにokとngに振り分ける（ok=無敵）
    std::vector<CardSet> ok_set, ng_set;
    for (int rank = 1; rank <= 13; rank++) {
      CardSet a = filterByRank(c, rank);
      if (a.isEmpty())
        continue;
      if (is_strongest(a)) {
        ok_set.push_back(a);
      } else {
        ng_set.push_back(a);
      }
    }

    // 非無敵数が3つ以上なら勝ち確定ではない
    if (ng_set.size() >= 3) {
      return CardSet();
    }

    // 非無敵数が1つ以下なら勝ち確定
    if (ng_set.size() <= 1) {
      if (has_joker) {
        CardSet j;
        j.insert(JOKER);
        return j;
      }
      if (!ok_set.empty()) {
        return ok_set[0];
      } else {
        return ng_set[0];
      }
    }

    // 非無敵数が2つならば、JOKERの使い方次第で勝ち確の可能性がある
    if (!has_joker)
      return CardSet();

    CardSet ng0 = ng_set[0];
    CardSet ng1 = ng_set[1];

    ng0.insert(JOKER);
    if (is_strongest(ng0)) {
      return ng0;
    }
    ng1.insert(JOKER);
    if (is_strongest(ng1)) {
      return ng1;
    }
    return CardSet();
  };

  // win100の処理の中身ここから
  if (pile.isEmpty()) {
    // 親の場合
    CardSet w = win100_myturn(hand);
    if (!w.isEmpty())
      return w;
  } else {
    // 子の場合
    // すべての選択肢について、それを出して番を取ったあと勝ち確定か調べる
    for (const auto& v : validSets) {
      CardSet hand_copy(hand);
      hand_copy.remove(v);
      if (is_strongest(v) && !win100_myturn(hand_copy).isEmpty()) {
        return v;
      }
    }
  }

  return CardSet();
}

// 出せるカードセットの配列を返す
std::vector<CardSet> Group6::getValidSets() {
  std::vector<CardSet> sets;
  int size = pile.size();
  // 自分が親なら
  if (size == 0) {
    for (int rank = 1; rank <= 13; rank++) {
      CardSet a = filterByRank(hand, rank);
      CardSet c;
      for (int i = 0; i < a.size(); i++) {
        c.insert(a[i]);
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
      if ((rank + 10) % 13 <= (pile[0].getRank() + 10) % 13) {
        continue;
      }
      CardSet a = filterByRank(hand, rank);
      CardSet c;
      if (size <= a.size()) {
        for (int i = 0; i < size; i++) {
          c.insert(a[i]);
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

// csを出した場合、確実に自分のターンが回ってくるかどうか
bool Group6::is_strongest(const CardSet& cs) {
  return nextMax(unknownCards, cs).isEmpty();
}

// 試合開始前の処理
void Group6::ready() {
  pile_history.clear();
  hand.sort(myCardCmp);
}

// 毎ターン終了時の処理
bool Group6::approve(const GameStatus& gstat) {
  pile_history.insert(gstat.pile);
  pile_history.sort(myCardCmp);
  return true;
}
