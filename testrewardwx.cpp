#include "testrewardwx.hpp"

void testrewardwx::claim(uint64_t id, vector<name> winners)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

    action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, winners[0], rw_itr->quantity * PERCENTAGES[0] / 100, string("claim reward"))).send();
    action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, winners[1], rw_itr->quantity * PERCENTAGES[1] / 100, string("claim reward"))).send();
    action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, winners[2], rw_itr->quantity * PERCENTAGES[2] / 100, string("claim reward"))).send();
    action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, COMPANY_ACCOUNT, rw_itr->quantity * PERCENTAGES[3] / 100, string("claim reward"))).send();

    _rw.erase(rw_itr);
}

void testrewardwx::remove(uint64_t id)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

     _rw.erase(rw_itr);
}

void testrewardwx::create(uint64_t id, string date, string game)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr == _rw.end(), "Reward with " + to_string(id) + " id already exists!");

    _rw.emplace(CONTRACTN, [&](auto& new_board) {
        new_board.id = id;
        new_board.date = date;
        new_board.game = game;
        new_board.quantity = asset(0, WAX_SYMBOL);
    });
}

void testrewardwx::transfer(const name &from, const name &to, const asset &quantity, const string &memo)
{
    require_auth(from);

    if (from == _self) {
        return;
    }

    check(EOS_CONTRACT == get_first_receiver(), "invalid contract");
    check(to == _self, "contract is not involved in this transfer");
    check(quantity.symbol.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "only positive quantity allowed");
    check(quantity.symbol == WAX_SYMBOL, "only EOS tokens allowed");

    rewards _rw(CONTRACTN, CONTRACTN.value);
    auto rw_itr = _rw.find(std::atoi(memo.c_str()));

    check(rw_itr != _rw.end(), "Reward with " + memo + " id doesn't exist!");

    _rw.modify(rw_itr, get_self(), [quantity](auto &a) {
        a.quantity += quantity;
    });
}