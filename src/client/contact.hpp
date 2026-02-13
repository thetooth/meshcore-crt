#pragma once

#include <map>

namespace MeshCore {
//     public_key: bytes(32),
//   type: byte,   // one of ADV_TYPE_*
//   flags: byte,
//   out_path_len: signed-byte,
//   out_path: bytes(64),
//   adv_name: chars(32),    // advertised  name (null terminated)
//   last_advert: uint32,
//   adv_lat: int32,    // advertised latitude * 1E6
//   adv_lon: int32,    // advertised longitude * 1E6
//   lastmod: uint32
struct CONTACT_INFO_MSG {
  char pubKey[32];
  uint8_t type;
  uint8_t flags;
  int8_t outPathLen;
  char outPath[64];
  arduino::String advName;
  uint32_t lastAdvert;
  int32_t advLat;
  int32_t advLon;
  uint32_t lastMod;
};
class Contact : public CONTACT_INFO_MSG {
public:
};
class ContactList : public std::map<arduino::String, Contact> {
public:
  bool contains(const arduino::String &pubKeyPrefix) const { return find(pubKeyPrefix) != end(); }

  uint32_t count = 0;
  uint32_t since = 0;
};
} // namespace MeshCore