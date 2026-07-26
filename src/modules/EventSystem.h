#pragma once
#include <functional>
#include <vector>
#include <string>
#include <jni.h>
#include <unordered_map>

enum class EventType {
    TICK,
    RENDER_3D,
    RENDER_2D,
    PACKET_SEND,
    PACKET_RECEIVE,
    KEY_PRESS,
    MOUSE_CLICK,
    CHAT_MESSAGE,
    PLAYER_MOVE,
    ENTITY_HIT,
    COUNT
};

class Event {
public:
    Event(EventType type) : type_(type), cancelled_(false) {}
    virtual ~Event() = default;

    EventType GetType() const { return type_; }
    bool IsCancelled() const { return cancelled_; }
    void SetCancelled(bool c) { cancelled_ = c; }

private:
    EventType type_;
    bool cancelled_;
};

class TickEvent : public Event {
public:
    TickEvent(JNIEnv* env) : Event(EventType::TICK), env_(env) {}
    JNIEnv* GetEnv() { return env_; }
private:
    JNIEnv* env_;
};

class PacketBusEvent : public Event {
public:
    PacketBusEvent(JNIEnv* env, jobject packet, bool outgoing)
        : Event(outgoing ? EventType::PACKET_SEND : EventType::PACKET_RECEIVE),
          env_(env), packet_(packet) {}
    JNIEnv* GetEnv() { return env_; }
    jobject GetPacket() { return packet_; }
    void SetPacket(jobject p) { packet_ = p; }
private:
    JNIEnv* env_;
    jobject packet_;
};

class EventManager {
public:
    using Handler = std::function<void(Event&)>;

    static EventManager& Get() {
        static EventManager inst;
        return inst;
    }

    void Subscribe(EventType type, Handler handler) {
        handlers_[type].push_back(handler);
    }

    void Unsubscribe(EventType type, Handler) {
        // Simplified: clear all handlers for type
        handlers_[type].clear();
    }

    void Fire(Event& event) {
        auto it = handlers_.find(event.GetType());
        if (it != handlers_.end()) {
            for (auto& handler : it->second) {
                handler(event);
                if (event.IsCancelled()) break;
            }
        }
    }

    void Clear() { handlers_.clear(); }

private:
    EventManager() = default;
    std::unordered_map<EventType, std::vector<Handler>> handlers_;
};
