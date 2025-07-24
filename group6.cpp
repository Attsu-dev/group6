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

// カードセットのポイントを返す（親のとき）
int Group6::getPoint_myTurn(const CardSet& cs) {
  int size = cs.size();
  int strength = cs[0].strength();

  int point = 10000;
  point -= size;            // カード枚数が多いほど優先
  point -= strength * 100;  // カードが弱いほど優先

  // ペアを崩す手は最弱
  if (cs.size() < filterByRank(hand, cs[0].rank()).size()) {
    return -1;
  }
  return point;
}

// カードセットのポイントを返す（子のとき）
int Group6::getPoint_NotMyTurn(const CardSet& cs) {
  int point = 10000;
  int size = cs.size();
  int strength = cs[0].strength();
  point += cs.size();       // カード枚数が少ないほど優先
  point -= strength * 100;  // カードが弱いほど優先

  if (is_strongest(cs) && is_strongest(pile)) {
    if ((gamestatus.numPlayers + gamestatus.turnIndex - lastSubmiterIndex) %
            gamestatus.numPlayers <
        gamestatus.numPlayers / 2) {
      return -1;  // 自分のターンが回ってくるのが早くなるのでパスで良い
    } else {
      point += 10;  // 自分のターンが回ってくるのが遅くなる
    }
  }

  if (sumCardsNum > 25) {
    if (strength >= 12 && size >= 2) {
      point -= 1000;
    }
  }

  // ペアを崩すのに番を取れない手は最弱
  if (cs.size() < filterByRank(hand, cs[0].rank()).size() &&
      !is_strongest(cs)) {
    return -1;  // 負ならば選ばれない
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
  sumCardsNum = 0;
  for (int i = 0; i < gstat.numPlayers; i++) {
    sumCardsNum += gstat.numCards[i];
  }

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

  Card tmp;
  int leadSize;

  hand.sort(myCardCmp);  // 手札をソート(弱い方から順に)

  // 手札の種類確認用
  int weak_cards = 0;
  int normal_cards = 0;
  int strong_cards = 0;

  for (int i = 0; i < hand.size(); i++) {
    Card current_card = hand.at(i);
    int rank = current_card.rank();  // カードの数字を取得

    if (current_card.isJoker() || rank == 1 || rank == 2) {
      // 強いカード (A, 2, Joker)
      strong_cards++;
    } else if (rank >= 7 && rank <= 13) {
      // 普通のカード (7, 8, 9, 10, J, Q, K)
      normal_cards++;
    } else {
      // 弱いカード (3, 4, 5, 6)
      weak_cards++;
    }
  }

  std::cout << " [弱:" << weak_cards << " 中:" << normal_cards
            << " 強:" << strong_cards << "] ";

  bool is_late_game = false;
  bool more2_is_good = false;
  const int DANGER_THRESHOLD =
      3;  // 他のプレイヤーの残り枚数がこの数以下なら危険と判断

  for (int i = 0; i < gstat.numPlayers; i++) {
    // 自分以外のプレイヤーをチェック
    if (gstat.playerID[gstat.turnIndex] != gstat.playerID[i] &&
        gstat.numCards[i] <= 1) {
      more2_is_good = true;
      std::cout << " [２枚以上のペアを出したい] ";
    }
    if (gstat.playerID[gstat.turnIndex] != gstat.playerID[i] &&
        gstat.numCards[i] <= DANGER_THRESHOLD) {
      is_late_game = true;
      std::cout << " [上がり阻止モード] ";
      // break; // 危険な相手が一人でも見つかればチェック終了
    }
  }

  bool can_win = false;
  int my_hand[14];
  int oponent_hand[14];
  CardSet unk_card = getUnknownCards();
  int can_flow_cards_num = 0;
  int cant_flow_cards_num = 0;
  int need_num_to_flow[14];

  for (int i = 0; i <= 13; i++) {
    my_hand[i] = 0;
    oponent_hand[i] = 0;
  }

  for (int i = 0; i < hand.size(); i++) {
    if (hand.at(i).rank() == 15) {
      my_hand[0] = 1;
    } else {
      my_hand[hand.at(i).rank()]++;
    }
  }

  for (int i = 0; i < unk_card.size(); i++) {
    if (unk_card.at(i).rank() == 15) {
      oponent_hand[0] = 1;
    } else {
      oponent_hand[unk_card.at(i).rank()]++;
    }
  }

  need_num_to_flow[0] = 1;
  need_num_to_flow[2] = 2;
  need_num_to_flow[1] = 5;
  for (int i = 3; i <= 13; i++) {
    need_num_to_flow[i] = 5;
  }

  int max_num = 0;

  for (int i = 3; i < 13; i++) {
    for (int j = i; j < 13; j++) {
      max_num = 0;
      if (oponent_hand[j + 1] > max_num) {
        max_num = oponent_hand[j + 1];
      }
    }
    if (oponent_hand[1] > max_num) {
      max_num = oponent_hand[1];
    }
    if (oponent_hand[2] > max_num) {
      max_num = oponent_hand[2];
    }
    if (oponent_hand[0] > max_num) {
      max_num = oponent_hand[0];
    }
    need_num_to_flow[i] = max_num + 1;
  }

  max_num = 0;
  if (oponent_hand[1] > max_num) {
    max_num = oponent_hand[1];
  }
  if (oponent_hand[2] > max_num) {
    max_num = oponent_hand[2];
  }
  if (oponent_hand[0] > max_num) {
    max_num = oponent_hand[0];
  }
  need_num_to_flow[13] = max_num + 1;

  max_num = 0;
  if (oponent_hand[2] > max_num) {
    max_num = oponent_hand[2];
  }
  if (oponent_hand[0] > max_num) {
    max_num = oponent_hand[0];
  }
  need_num_to_flow[1] = max_num + 1;

  max_num = 0;
  if (oponent_hand[0] > max_num) {
    max_num = oponent_hand[0];
  }
  need_num_to_flow[2] = max_num + 1;

  for (int i = 0; i <= 13; i++) {
    if (my_hand[i] >= need_num_to_flow[i]) {
      can_flow_cards_num++;
    } else if (my_hand[i] != 0) {
      cant_flow_cards_num++;
    }
  }
  if (can_flow_cards_num >= 1 && cant_flow_cards_num <= 1) {
    can_win = true;
  }

  //////////////////////////////
  // カードを出す処理
  ////////////////////////////////

  leadSize = pile.size();

  if (leadSize == 0) {
    Card weakest_card = hand.at(0);

    if (can_win) {
      for (int i = 0; i <= 13; i++) {
        if (my_hand[i] >= 1 && need_num_to_flow[i] <= my_hand[i]) {
          for (int j = 0; j < hand.size(); j++) {
            if (i == 0 && hand.at(j).rank() == 15) {
              cs.insert(hand.at(j));
            } else if (i != 0 && hand.at(j).rank() == i) {
              cs.insert(hand.at(j));
            }
          }
          hand.remove(cs);
          return true;
        }
      }
    }

    if (more2_is_good) {
      for (int i = 0; i <= 13; i++) {
        if (2 <= my_hand[i]) {
          for (int j = 0; j < hand.size(); j++) {
            if (i == 0 && hand.at(j).rank() == 15) {
              cs.insert(hand.at(j));
            } else if (i != 0 && hand.at(j).rank() == i) {
              cs.insert(hand.at(j));
            }
          }
          hand.remove(cs);
          return true;
        }
      }
    }

    cs.insert(weakest_card);

    // 2枚目以降のカードを確認し、一番弱いカードと同じランクなら追加する
    for (int i = 1; i < hand.size(); i++) {
      if (hand.at(i).rank() == weakest_card.rank()) {
        cs.insert(hand.at(i));
      } else {
        // ランクが違うカードが出てきたら、ペア探しは終了
        break;
      }
    }

    // 決定したカード(cs)を手札から削除して場に出す
    hand.remove(cs);
    return true;

  } else {
    // 場に出せるカードの組み合わせをすべて探す
    std::vector<CardSet>
        candidates;  // 出せるカードの組み合わせを保存するリスト
    Card pile_rank_card = pile.at(0);  // 場に出ているカードの強さの基準
    // 手札のi枚目から、場の枚数(leadSize)ぶんのカードを確認していく
    for (int i = 0; i <= hand.size() - leadSize; i++) {
      // 同じランクのカードがleadSize枚あるかチェック
      CardSet potential_play;  // 「出せるかもしれないカード」のセット
      Card first_card = hand.at(i);

      // 場に出ているカードより強いかチェック
      if (first_card.isGreaterThan(pile_rank_card)) {
        potential_play.insert(first_card);
        for (int j = 1; j < leadSize; j++) {
          if (hand.at(i + j).rank() == first_card.rank()) {
            potential_play.insert(hand.at(i + j));
          }
        }
        if (potential_play.size() == leadSize) {
          candidates.push_back(potential_play);
          // 同じランクのカードを再度チェックしないように、iをジャンプさせる
          i += (leadSize - 1);
        }
      }
    }

    if (!candidates.empty()) {  // 候補が1つ以上見つかった場合

      CardSet best_play;  // 最もスコアが高かった手
      int max_score =
          -1000;  // 最高スコアを記録する変数（非常に低い値で初期化）
      // 全ての候補手のスコアを計算する
      for (int i = 0; i < static_cast<int>(candidates.size()); i++) {
        CardSet current_play = candidates.at(i);
        int current_score = 0;  // この手のスコア
        // --- スコア計算 ---
        // 1. 基本点：弱いカードほど価値が高い（温存できるから）
        current_score +=
            15 - current_play.at(0)
                     .strength();  // 3(強さ1)なら+14点、A(強さ12)なら+3点
        // 2. ボーナス点：上がりそうな相手がいるか？

        if (is_late_game) {
          // 相手を阻止できる手は非常に価値が高い！
          current_score += 50;  // 絶対出す
        }

        if (can_win) {
          if (current_play.at(0).rank() == 15) {
            // 流せるカードを出す｡
            current_score += 100;
          } else if (my_hand[current_play.at(0).rank()] ==
                         current_play.size() &&
                     my_hand[current_play.at(0).rank()] >=
                         need_num_to_flow[current_play.at(0).rank()]) {
            // 流せるカードを出す｡
            current_score += 100;
          }
        }

        //////  序盤  ////////////
        if (weak_cards + normal_cards >= 7) {
          int rank = current_play.at(0).rank();
          int op_rank = pile.at(0).rank();
          if (current_play.at(0).isJoker() || rank == 2 || rank == 1) {
            current_score -= 50;  // 大きなペナルティ、絶対パス
          }

          // if ((op_rank == 3 || op_rank == 4) &&
          //     my_hand[11] + my_hand[12] >= 3) {
          //   if (rank == 11 || rank == 12) {
          //     current_score += 50;
          //   }
          // }

          if (my_hand[rank] != current_play.size()) {
            current_score -= 50;
          }
        }

        ////// 中盤 ////////////
        if (weak_cards + normal_cards < 7 && weak_cards + normal_cards >= 3) {
          int rank = current_play.at(0).rank();
          int op_rank = pile.at(0).rank();
          if (strong_cards == 1) {
            if (current_play.at(0).isJoker() || rank == 1 || rank == 2 ||
                rank == 13) {
              current_score -= 30;  // 大きなペナルティ、絶対パス
            }
          }
          if (my_hand[rank] != current_play.size()) {
            current_score -= 50;
          }
        }

        ////// 終盤 ////////////
        /*if (weak_cards + normal_cards <= 4 && weak_cards + normal_cards > 2) {

          }*/

        ////// 最終盤 ////////////
        if (weak_cards + normal_cards <= 2) {
          current_score =
              1000 -
              current_score;  // 最終盤はスコアを反転させる、強いやつを出す
          int rank = current_play.at(0).rank();
          if (my_hand[rank] != current_play.size()) {
            current_score -= 50;
          }
        }

        // --- 最高スコアの更新 ---
        if (current_score > max_score) {
          max_score = current_score;
          best_play = current_play;
        }
      }

      // --- 最終判断 ---
      // 「パスする(0点)」よりも、最も良い手のスコアが高ければ、その手をプレイ
      if (max_score > 0) {
        cs = best_play;
      } else {
        std::cout << " [スコアパス] ";
        cs.clear();
      }
      if (!cs.isEmpty()) {
        hand.remove(cs);
      }
      return true;
    }
  }
  // 出せるカードがなければパス
  cs.clear();
  return true;
}

// ポイントが一番多い選択肢を返す
CardSet Group6::getHandByPoint() {
  std::vector<int> points(validSets.size());
  for (int i = 0; i < (int)validSets.size(); i++) {
    if (pile.isEmpty())
      points[i] = getPoint_myTurn(validSets[i]);
    else
      points[i] = getPoint_NotMyTurn(validSets[i]);
  }

  // パスの選択肢を追加 (ポイント = 0)
  if (!pile.isEmpty()) {
    validSets.push_back(CardSet());
    points.push_back(0);
  }

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
  if (!pile.equal(gstat.pile)) {
    lastSubmiterIndex = gstat.turnIndex;
  }
  pile = gstat.pile;
  pile_history.insert(gstat.pile);
  pile_history.sort(myCardCmp);
  return true;
}
