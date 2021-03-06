// Copyright (c) 2010 ANH Studios. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the COPYING file.

#include "anh/event.h"

#include <cassert>

/// The anh namespace hosts a number of useful utility classes intended
/// to be used and reused in domain specific classes.
namespace anh {

BaseEvent::BaseEvent(EventSubject subject, uint64_t delay_ms)
    : subject_(subject)
    , priority_(0)
    , timestamp_(0)
    , delay_ms_(delay_ms)
    , next_(nullptr)
    , callback_(nullptr) {}

BaseEvent::BaseEvent(EventSubject subject, uint64_t delay_ms, EventCallback callback)
    : subject_(subject)
    , priority_(0)
    , timestamp_(0)
    , delay_ms_(delay_ms)
    , next_(nullptr)
    , callback_(new EventCallback(callback)) {}

BaseEvent::~BaseEvent() {}

EventSubject BaseEvent::subject() const {
    return subject_;
}

void BaseEvent::subject(uint64_t subject) {
    subject_ = subject;
}

uint64_t BaseEvent::timestamp() const {
    return timestamp_;
}

void BaseEvent::timestamp(uint64_t timestamp) {
    timestamp_ = timestamp;
}

uint64_t BaseEvent::delay_ms() const {
    return delay_ms_;
}

void BaseEvent::delay_ms(uint64_t delay_ms) {
    delay_ms_ = delay_ms;
}

EventPriority BaseEvent::priority() const {
    return priority_;
}

void BaseEvent::priority(EventPriority priority) {
    priority_ = priority;
}

IEventPtr BaseEvent::next() const {
    return next_;
}

void BaseEvent::next(IEventPtr next) {
    next_ = next;
}

void BaseEvent::serialize(ByteBuffer& out) const {
    out.write<uint32_t>(event_type().ident());

    onSerialize(out);
}

void BaseEvent::deserialize(ByteBuffer& in) {
    // If the buffer size is less than at least the size of the crc or
    // if the crc read from the buffer doesn't match this event trigger
    // an assert and return.
    if (in.size() < sizeof(uint32_t) || event_type().ident() != in.read<uint32_t>()) {
        assert(!"Invalid buffer passed to this event");
        return;
    }

    // Call the virtual method to allow derived classes an opportunity to
    // deserialize from the bytebuffer further.
    onDeserialize(in);
}

void BaseEvent::consume(bool handled) const {
    if (onConsume(handled) && callback_) {
        (*callback_)();
    }
}

SimpleEvent::SimpleEvent(EventType& event_type, uint64_t subject_id, uint64_t delay_ms)
    : BaseEvent(subject_id, delay_ms)
    , event_type_(event_type) {}

SimpleEvent::SimpleEvent(EventType& event_type, uint64_t subject_id, uint64_t delay_ms, EventCallback callback)
    : BaseEvent(subject_id, delay_ms, callback)
    , event_type_(event_type) {}

SimpleEvent::~SimpleEvent() {}

const EventType& SimpleEvent::event_type() const {
    return event_type_;
}

void SimpleEvent::onSerialize(ByteBuffer& out) const {}
void SimpleEvent::onDeserialize(ByteBuffer& in) {}

bool SimpleEvent::onConsume(bool handled) const {
    return true;
}

// Helper function implementations

bool CompareEventWeightLessThan(const IEvent& lhs, const IEvent& rhs) {
    return (((lhs.timestamp() + lhs.delay_ms())+ lhs.priority()) < ((rhs.timestamp() + rhs.delay_ms()) + rhs.priority()));
}

bool CompareEventWeightLessThan(const std::shared_ptr<IEvent>& lhs, const std::shared_ptr<IEvent>& rhs) {
    return CompareEventWeightLessThan(*lhs, *rhs);
}

bool CompareEventWeightGreaterThan(const IEvent& lhs, const IEvent& rhs) {
    return (((lhs.timestamp() + lhs.delay_ms()) + lhs.priority()) > ((rhs.timestamp() + rhs.delay_ms()) + rhs.priority()));
}

bool CompareEventWeightGreaterThan(const std::shared_ptr<IEvent>& lhs, const std::shared_ptr<IEvent>& rhs) {
    return CompareEventWeightGreaterThan(*lhs, *rhs);
}

}  // namespace anh
