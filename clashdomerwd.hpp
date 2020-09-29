#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

using eosio::asset;
using eosio::name;
using eosio::symbol;
using eosio::symbol_code;
using eosio::check;
using eosio::permission_level;
using eosio::current_time_point;
using eosio::action;
using eosio::same_payer;
using std::string;
using std::vector;
using std::make_tuple;
using namespace std;

#define CONTRACTN eosio::name("clashdomerwd")

class [[eosio::contract]] clashdomerwd : public eosio::contract
{
  public:
    using contract::contract;

    ACTION claim(uint64_t id, vector<name> winners, vector<uint32_t> rewards_percentages);
    ACTION remove(uint64_t id);
    ACTION create(uint64_t id, string date, string game);
    ACTION update(uint64_t id, const asset &quantity);
    [[eosio::on_notify("eosio.token::transfer")]] void transfer(const name &from, const name &to, const asset &quantity, const string &memo);

private:

    struct player
    {
      eosio::name account;          //user wax account
      vector<uint64_t> timestamps;   //payment times
    };

    TABLE reward {
        uint64_t id; 
        string date;       //date of leaderboard
        string game;       //game of leaderboard
        asset quantity;
        vector<player>  payments; 
        uint64_t primary_key() const { return id; }
    };

    typedef eosio::multi_index<name("rewards"), reward> rewards;

    static constexpr name COMPANY_ACCOUNT = "gr.au.wam"_n;
    static constexpr name EOS_CONTRACT = "eosio.token"_n;
    static constexpr symbol WAX_SYMBOL = symbol(symbol_code("WAX"), 8);

    uint64_t finder(vector<player> payments, eosio::name account);
};