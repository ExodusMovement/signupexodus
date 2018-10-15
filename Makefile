build: wast

clean:
	rm -f signupexodus.wast
	rm -f signupexodus.wasm
	rm -f signupexodus.abi

abi:
	eosio-cpp -g signupexodus.abi signupexodus.hpp

wast: wasm
	eosio-wasm2wast -o signupexodus.wast signupexodus.wasm

wasm:
	eosio-cpp -o signupexodus.wasm signupexodus.cpp --abigen

deploy: wast wasm
	cleos set contract signupexodus ../signupexodus

build_and_deploy: build deploy
