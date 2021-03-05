#include "clashdomerwd.hpp"

void clashdomerwd::claim(uint64_t id, vector<name> winners, vector<uint32_t> rewards_percentages)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

    uint32_t commission_percetage = 100;

    string game_name = "";

    if (rw_itr->game == "endless-siege") {
        game_name = "Endless Siege";
    } else {
        game_name = "Candy Fiesta";
    }

    if (rw_itr->quantity.amount > 0) {

        for(int i = 0; i < rewards_percentages.size(); i++) {
            action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, winners[i], rw_itr->quantity * rewards_percentages[i] / 100, string(game_name + " - " + to_string(i + 1) + " classified"))).send();
            commission_percetage -= rewards_percentages[i];
        }

        action(permission_level{_self, "active"_n}, EOS_CONTRACT, "transfer"_n, make_tuple(_self, COMPANY_ACCOUNT, rw_itr->quantity * commission_percetage / 100, string(game_name + " - Commission"))).send();   
    }
}

void clashdomerwd::remove(uint64_t id)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

     _rw.erase(rw_itr);
}

void clashdomerwd::create(uint64_t id, string date, string game)
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

void clashdomerwd::update(uint64_t id, const asset &quantity)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

    _rw.modify(rw_itr, get_self(), [quantity](auto &a) {
        a.quantity = quantity;
    });
}

void clashdomerwd::freeticket(uint64_t id, const asset &quantity, const name &account)
{
    require_auth(_self);

    rewards _rw(CONTRACTN, CONTRACTN.value);

    auto rw_itr = _rw.find(id);
    check(rw_itr != _rw.end(), "Reward with " + to_string(id) + " id doesn't exist!");

    _rw.modify(rw_itr, get_self(), [&](auto &a) {
        a.quantity += quantity;
    });
}

void clashdomerwd::transfer(const name &from, const name &to, const asset &quantity, const string &memo)
{
    require_auth(from);

    if (from == _self) {
        return;
    }

    check(EOS_CONTRACT == get_first_receiver(), "invalid contract");
    check(to == _self, "contract is not involved in this transfer");
    check(quantity.symbol.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "only positive quantity allowed");
    check(quantity.symbol == WAX_SYMBOL, "only WAX tokens allowed");

    rewards _rw(CONTRACTN, CONTRACTN.value);
    auto rw_itr = _rw.find(std::atoi(memo.c_str()));

    check(rw_itr != _rw.end(), "Reward with " + memo + " id doesn't exist!");

    uint64_t timestamp = eosio::current_time_point().sec_since_epoch();

    uint64_t pos = finder(rw_itr->payments, from);

    if (pos == -1)
    {
        vector<uint64_t> timestamps;
        timestamps.push_back(timestamp); 
        pos = rw_itr->payments.size();
        _rw.modify(rw_itr, get_self(), [&](auto &mod_board) {
            mod_board.payments.push_back({
                from,
                timestamps
            });
        });
    } else {

        uint64_t old_timestamp = rw_itr->payments.at(pos).timestamps.back();

        check(old_timestamp + 120 < timestamp, "A payment cannot be made in less than 2 minutes.");

        _rw.modify(rw_itr, get_self(), [&](auto &mod_board) {
            mod_board.payments.at(pos).timestamps.push_back(timestamp); 
        });
    }

    _rw.modify(rw_itr, get_self(), [quantity](auto &a) {
        a.quantity += quantity;
    });
}

uint64_t clashdomerwd::finder(vector<player> payments, eosio::name account)
{
    for (uint64_t i = 0; i < payments.size(); i++)
    {
        if (payments.at(i).account == account)
        {
            return i;
        }
    }
    return -1;
}