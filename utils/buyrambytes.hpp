#pragma once

namespace eosio {

   asset buyrambytes( uint32_t bytes ) {
      eosiosystem::rammarket market(N(eosio),N(eosio));
      auto itr = market.find(S(4,RAMCORE));
      eosio_assert(itr != market.end(), "RAMCORE market not found");
      auto tmp = *itr;
      return tmp.convert( asset(bytes,S(0,RAM)), CORE_SYMBOL );
   }

}