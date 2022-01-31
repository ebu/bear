#pragma once
#include "bear/api.hpp"
#include "listener_impl.hpp"

namespace bear {
void adapt_otm(const ObjectsInput &block_in, ObjectsInput &block_out, const ListenerImpl &listener);
void adapt_dstm(const DirectSpeakersInput &block_in,
                DirectSpeakersInput &block_out,
                const ListenerImpl &listener);
}  // namespace bear
