#pragma once

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <functional>

namespace MeshCore {
struct CHANNEL_INFO_MSG {
  uint8_t channelIndex;
  arduino::String name;
};
class Channel : public CHANNEL_INFO_MSG {
public:
  Channel() {}
  void update(const CHANNEL_INFO_MSG &info) {
    channelIndex = info.channelIndex;
    name = info.name;
  }

  CircularBuffer<arduino::String, 10> messages;
};
class ChannelList : public std::array<Channel, 8> {
public:
  enum class SyncState { Start, InProgress, Done } state = SyncState::Start;
  bool lastSyncOK = false;
  uint8_t syncIndex = 0;

  bool requestAll(std::function<void(uint8_t)> requestChannelInfo) {
    switch (state) {
    case SyncState::Start:
      state = SyncState::InProgress;
      syncIndex = 0;
      lastSyncOK = false;
      requestChannelInfo(0);
      break;
    case SyncState::InProgress:
      if (lastSyncOK) {
        syncIndex++;
        if (syncIndex >= size()) {
          state = SyncState::Done;
          break;
        }
        requestChannelInfo(syncIndex);
        lastSyncOK = false;
      }
      break;
    case SyncState::Done:
      return true;
    }
    return false;
  }
};
} // namespace MeshCore