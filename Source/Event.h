#pragma once

#include <vector>

namespace Event
{
	namespace Internal
	{
		template <typename TArgs>
		struct EventHandler
		{
			using ArgType = TArgs;

			virtual         ~EventHandler() = default;

			virtual void    operator() (void * sender, const TArgs & args) = 0;

		};
		template <>
		struct EventHandler<void>
		{
			using ArgType = void;

			virtual         ~EventHandler() = default;

			virtual void    operator() (void * sender) = 0;
		};
		template <typename TArgs>
		class Event
		{
		public:
			using EventHandlerRef = EventHandler<TArgs> &;
			using EventHandlerPtr = EventHandler<TArgs> *;

			void            AddHandler(EventHandlerRef handler)
			{
				handlers.emplace_back(&handler);
			}
			void            RemoveHandler(EventHandlerRef handler)
			{
				for ( auto it = handlers.rbegin(); it != handlers.rend(); ++it )
				{
					if ( *it == &handler )
					{
						handlers.erase(std::next(it).base());
						break;
					}
				}
			}

			void            Dispatch(void * sender, const TArgs & args)
			{
				for ( auto h : handlers ) ( *h )( sender, args );
			}

		private:
			std::vector<EventHandlerPtr> handlers;
		};
		template <>
		class Event<void>
		{
		public:
			using EventHandlerRef = EventHandler<void> &;
			using EventHandlerPtr = EventHandler<void> *;

			void            AddHandler(EventHandlerRef handler)
			{
				handlers.emplace_back(&handler);
			}
			void            RemoveHandler(EventHandlerRef handler)
			{
				for ( auto it = handlers.rbegin(); it != handlers.rend(); ++it )
				{
					if ( *it == &handler )
					{
						handlers.erase(std::next(it).base());
						break;
					}
				}
			}

			void            Dispatch(void * sender)
			{
				for ( auto h : handlers ) ( *h )( sender );
			}

		private:
			std::vector<EventHandlerPtr> handlers;
		};
	}
}

// --------------------------------------------------------------------------
// Macros to define Events
// --------------------------------------------------------------------------

#define _DEFINE_EVENT(name) \
    struct name##EventHandler : ::Event::Internal::EventHandler<void> { using ::Event::Internal::EventHandler<void>::EventHandler; }; \
    struct name##Event : ::Event::Internal::Event<void> {}
#define _DEFINE_EVENT1(name, args) \
    struct name##EventHandler : ::Event::Internal::EventHandler<args> { using ::Event::Internal::EventHandler<args>::EventHandler; }; \
    struct name##Event : ::Event::Internal::Event<args> {}

// --------------------------------------------------------------------------
// Macros to define Senders, Receivers, and Bind
// --------------------------------------------------------------------------

#define _SEND_EVENT(name) \
        name##Event & Get##name##Event() { return __##name##_event_sender; } \
    private: \
        name##Event __##name##_event_sender

// receiver with inline handler
#define _RECV_EVENT(receiver, name) \
        name##EventHandler & Get##name##EventHandler() \
        { \
            __##name##_event_receiver.m_receiver = this; \
            __##name##_event_receiver.m_handleFunc = &receiver::__##name##_EventHandleFunc; \
            return __##name##_event_receiver; \
        } \
    private: \
        struct __##name##_EventHandler : public name##EventHandler \
        { \
            using           FuncType = void (receiver::*)(void * sender); \
            receiver *      m_receiver = nullptr; \
            FuncType        m_handleFunc = nullptr; \
            virtual void    operator() (void * sender) override { (m_receiver->*m_handleFunc)(sender); } \
        } __##name##_event_receiver; \
        void __##name##_EventHandleFunc
#define _RECV_EVENT1(receiver, name) \
        name##EventHandler & Get##name##EventHandler() \
        { \
            __##name##_event_receiver.m_receiver = this; \
            __##name##_event_receiver.m_handleFunc = &receiver::__##name##_EventHandleFunc; \
            return __##name##_event_receiver; \
        } \
    private: \
        struct __##name##_EventHandler : public name##EventHandler \
        { \
            using           FuncType = void (receiver::*)(void * sender, const name##EventHandler::ArgType & args); \
            receiver *      m_receiver = nullptr; \
            FuncType        m_handleFunc = nullptr; \
            virtual void    operator() (void * sender, const name##EventHandler::ArgType & args) override { (m_receiver->*m_handleFunc)(sender, args); } \
        } __##name##_event_receiver; \
        void __##name##_EventHandleFunc
// receiver with handler decl & impl
#define _RECV_EVENT_DECL(receiver, name) \
    _RECV_EVENT(receiver, name) (void * sender)
#define _RECV_EVENT_DECL1(receiver, name) \
    _RECV_EVENT1(receiver, name) (void * sender, const name##EventHandler::ArgType & args)
#define _RECV_EVENT_IMPL(receiver, name) \
    void receiver::__##name##_EventHandleFunc

#define _BIND_EVENT(name, sender, receiver) \
    do { \
        (sender).Get##name##Event().AddHandler((receiver).Get##name##EventHandler()); \
    } while (0)
#define _UNBIND_EVENT(name, sender, receiver) \
    do { \
        (sender).Get##name##Event().RemoveHandler((receiver).Get##name##EventHandler()); \
    } while (0)

// --------------------------------------------------------------------------
// Macros to generate Events
// --------------------------------------------------------------------------

#define _DISPATCH_EVENT(name, sender) \
    do { \
        (sender).Get##name##Event().Dispatch(&(sender)); \
    } while (0)
#define _DISPATCH_EVENT1(name, sender, args) \
    do { \
        (sender).Get##name##Event().Dispatch(&(sender), (args)); \
    } while (0)

// --------------------------------------------------------------------------
// Event Definitions
// --------------------------------------------------------------------------

#include <Windows.h>
#include "SharedTypes.h"

namespace win32
{
	struct WindowRect
	{
		int x, y;
		int width, height;
	};

	struct WindowPaintArgs
	{
		HDC hdc;
	};

	struct MouseEventArgs
	{
		int pixelX;
		int pixelY;
		DWORD flags;
	};

	struct KeyboardEventArgs
	{
		WPARAM virtualKeyCode;
	};
}

// window
_DEFINE_EVENT(OnWndIdle);
_DEFINE_EVENT1(OnWndMove, win32::WindowRect);
_DEFINE_EVENT1(OnWndResize, win32::WindowRect);
_DEFINE_EVENT1(OnWndPaint, win32::WindowPaintArgs);
// mouse
_DEFINE_EVENT1(OnMouseMove, win32::MouseEventArgs);
_DEFINE_EVENT1(OnMouseLButtonDown, win32::MouseEventArgs);
_DEFINE_EVENT1(OnMouseLButtonUp, win32::MouseEventArgs);
// keyboard
_DEFINE_EVENT1(OnKeyDown, win32::KeyboardEventArgs);
_DEFINE_EVENT1(OnKeyUp, win32::KeyboardEventArgs);

// renderer window
_DEFINE_EVENT1(OnAspectRatioChange, float);

// camera
_DEFINE_EVENT1(OnCameraDirChange, Vec3);
_DEFINE_EVENT1(OnCameraPosChange, Vec3);
