build: wast

clean:
	rm -f signupexodus.wast signupexodus.wasm signupexodus.abi
	rm -f exodussignup.wast exodussignup.wasm exodussignup.abi

abi:
	eosio-cpp -g signupexodus.abi signupexodus.hpp
	eosio-cpp -g exodussignup.abi exodussignup.hpp

wast: wasm
	eosio-wasm2wast -o signupexodus.wast signupexodus.wasm
	eosio-wasm2wast -o exodussignup.wast exodussignup.wasm

wasm:
	eosio-cpp -o signupexodus.wasm signupexodus.cpp --abigen
	eosio-cpp -o exodussignup.wasm exodussignup.cpp --abigen

deploy: wast wasm
	cleos set contract signupexodus ../signupexodus
	cleos set contract exodussignup ../exodussignup

build_and_deploy: build deploy
