//
// Created by Hongbo Tang on 2018/7/5.
// Modified by Ferenc Kiraly on 2018/10/15
//

#include "signupexodus.hpp"

void signupexodus::transfer(account_name from, account_name to, asset quantity, string memo) {
    if (from == _self || to != _self) {
        return;
    }
    eosio_assert(quantity.symbol == CORE_SYMBOL, "signupexodus only accepts CORE for signup eos account");
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");

    memo.erase(memo.begin(), find_if(memo.begin(), memo.end(), [](int ch) {
        return !isspace(ch);
    }));
    memo.erase(find_if(memo.rbegin(), memo.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), memo.end());

    auto separator_pos = memo.find(' ');
    if (separator_pos == string::npos) {
        separator_pos = memo.find('-');
    }
    eosio_assert(separator_pos != string::npos, "Account name and public key must be separated by space or dash");

    string account_name_str = memo.substr(0, separator_pos);
    eosio_assert(account_name_str.length() == 12, "Length of account name should be 12");
    account_name new_account_name = string_to_name(account_name_str.c_str());

    // Verify that the account does not exists
    eosio_assert(!is_account(new_account_name), "Account already exists");

    string public_key_str = memo.substr(separator_pos + 1);
    eosio_assert(public_key_str.length() == 53, "Length of public key should be 53");

    string pubkey_prefix("EOS");
    auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), public_key_str.begin());
    eosio_assert(result.first == pubkey_prefix.end(), "Public key should be prefixed with EOS");
    auto base58substr = public_key_str.substr(pubkey_prefix.length());

    vector<unsigned char> vch;
    eosio_assert(decode_base58(base58substr, vch), "Decode pubkey failed");
    eosio_assert(vch.size() == 37, "Invalid public key");

    array<unsigned char,33> pubkey_data;
    copy_n(vch.begin(), 33, pubkey_data.begin());

    checksum160 check_pubkey;
    ripemd160(reinterpret_cast<char *>(pubkey_data.data()), 33, &check_pubkey);
    eosio_assert(memcmp(&check_pubkey.hash, &vch.end()[-4], 4) == 0, "invalid public key");

    const int64_t max_ram_cost = 20000; // Maximum RAM cost
    const int64_t net_stake = 200*5;  // Amount to stake for NET [1/10 mEOS]
    const int64_t cpu_stake = 9800*5; // Amount to stake for CPU [1/10 mEOS]
    const uint64_t bytes = 4096;      // Number of bytes of RAM to buy for the created account
    const uint64_t bytes_self = 200;  // Number of bytes of RAM to buy for signupexodus to reimburse for lost RAM and make signupexodus self-sufficient

    asset buy_ram_total = buyrambytes((uint32_t)(bytes + bytes_self));
    eosio_assert(buy_ram_total.amount <= max_ram_cost, "RAM currently too expensive, try again later");
    asset buy_ram = buy_ram_total * bytes / (bytes + bytes_self);
    asset buy_ram_self = buy_ram_total - buy_ram;

    asset stake_net(net_stake, CORE_SYMBOL);
    asset stake_cpu(cpu_stake, CORE_SYMBOL);
    asset liquid = quantity - stake_net - stake_cpu - buy_ram_total;
    eosio_assert(liquid.amount > 0, "Not enough balance to buy ram");

    signup_public_key pubkey = {
        .type = 0,
        .data = pubkey_data,
    };
    key_weight pubkey_weight = {
        .key = pubkey,
        .weight = 1,
    };
    authority owner = authority{
        .threshold = 1,
        .keys = {pubkey_weight},
        .accounts = {},
        .waits = {}
    };
    authority active = authority{
        .threshold = 1,
        .keys = {pubkey_weight},
        .accounts = {},
        .waits = {}
    };
    newaccount new_account = newaccount{
        .creator = _self,
        .name = new_account_name,
        .owner = owner,
        .active = active
    };

    action(
        permission_level{ _self, N(active) },
        N(eosio),
        N(newaccount),
        new_account
    ).send();

    action(
        permission_level{ _self, N(active)},
        N(eosio),
        N(buyram),
        make_tuple(_self, new_account_name, buy_ram)
    ).send();

    action(
        permission_level{ _self, N(active)},
        N(eosio),
        N(buyram),
        make_tuple(_self, _self, buy_ram_self)
    ).send();

    action(
        permission_level{ _self, N(active)},
        N(eosio),
        N(delegatebw),
        make_tuple(_self, new_account_name, stake_net, stake_cpu, true)
    ).send();

    action(
        permission_level{ _self, N(active) },
        N(eosio.token),
        N(transfer),
        std::make_tuple(_self, new_account_name, liquid, std::string("liquid balance"))
    ).send();
}
