# signupexodus

Smart contract for creating EOS accounts for users of Exodus. EOS can be sent to this contract with a memo formatted as `<account_name>-<publick_key>`, and the account `account_name` will be created and permissions for controlling the account will be assgined to `public_key`. For the account approximately 4k of RAM will be purchaed and 2.0 EOS weill be staked. The remaining EOS will be sent to the created account for immediate use.

### Set Up Environment

Download and build developer toolkit:
```
git clone --recursive https://github.com/eosio/eosio.cdt
cd eosio.cdt
./build.sh EOS
sudo ./install.sh
```

Download and build eosio:
```
EOSIODIR=`pwd`/eos
git clone --recursive git@github.com:EOSIO/eos.git
cd $EOSIODIR
./eosio_build.sh
sudo ./eosio_install.sh
```

Download and build signupexodus:
```
git clone ...
cd signupexodus
make
```

Assuming you have installed eosio as above, you have links to binaries in `/usr/local/bin`, ready to use.

### Prepare EOS wallet
```
# start the wallet daemon
keosd --wallet-dir $WALLET_DIR &

# create the default wallet
wallet  wallet create --to-console
Creating wallet: default
Save password to use in the future to unlock this wallet.
Without password imported keys will not be retrievable.
"PW5..."

# import private key for the active permission of the 'signupexodus' account
cleos wallet import --private-key "..."

# check public key in wallet
cleos wallet list keys
Wallets:
[
  "default *"
]
[
  "EOS6AtonxqWfiQwoBEUsdGf3BSoobTeeQGgvDQnL4WWfpvkWqBiX9",
]

# wallet will self-lock after a short period of inactivity, unlock with:
cleos wallet unlock
```

### Deploy and test contract

This assumes the account `signupexodus` is already created and has enough RAM for the contract and enough staked for uploading the contract. Staked EOS is not required for contract operation. Choose a node that works for you and set the NODE variable accordingly.
```
NODE=https://eu.eosdac.io
cleos -u $NODE set contract signupexodus signupexodus
cleos -u $NODE set account permission signupexodus active '{"threshold": 1,"keys": [{"key": "EOS6AtonxqWfiQwoBEUsdGf3BSoobTeeQGgvDQnL4WWfpvkWqBiX9","weight": 1}],"accounts": [{"permission":{"actor":"signupexodus","permission":"eosio.code"},"weight":1}]}' owner -p signupexodus
```

Test contract from exodus. Choose an account name, that does not exists and verify at a block browser. Prepare a public/private key pair, so you have control of the resulting account. 
```
cleos wallet create_key
Created new private key with a public key of: "EOS7...WpM"
```

Form Exodus send 3 EOS to 'signupexodus', with a memo like `<account_name>-<public-key>`. Or from an account with a key in your local wallet:
```
cleos -u $NODE transfer <my_account> signupexodus "3 EOS" "account_name-public_key"
```

