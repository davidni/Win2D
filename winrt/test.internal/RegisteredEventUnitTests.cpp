// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use these files except in compliance with the License. You may obtain
// a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.

#include "pch.h"

#include <RegisteredEvent.h>

#include "MockHelpers.h"

class TestEventSource : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IUnknown>
{
    EventSource<IEventHandler<IInspectable*>> m_eventSource;

public:
    IFACEMETHODIMP add_Event(IEventHandler<IInspectable*>* value, EventRegistrationToken* token)
    {
        return m_eventSource.Add(value, token);
    }

    IFACEMETHODIMP remove_Event(EventRegistrationToken token)
    {
        return m_eventSource.Remove(token);
    }

    void Raise()
    {
        ThrowIfFailed(m_eventSource.InvokeAll(nullptr, nullptr));
    }
};


TEST_CLASS(RegisteredEventTests)
{
    TEST_METHOD(RegisteredEvent_WhenDefaultConstructed_CanBeDestructed)
    {
        RegisteredEvent r;
    }

    TEST_METHOD(RegisteredEvent_WhenDefaultConstructed_CanBeDetached)
    {
        RegisteredEvent r;
        r.Detach();
    }

    TEST_METHOD(RegisteredEvent_WhenDefaultConstructed_CanBeReleased)
    {
        RegisteredEvent r;
        r.Release();
    }

    CALL_COUNTER(fn);

    TEST_METHOD(RegisteredEvent_WhenDestructed_CallsFunction)
    {
        fn.SetExpectedCalls(1);

        {
            RegisteredEvent r([=]() { fn.WasCalled(); });
        }

        Expectations::Instance()->Validate();
    }

    TEST_METHOD(RegisteredEvent_WhenReleased_CallsFunction)
    {
        fn.SetExpectedCalls(1);

        {
            RegisteredEvent r([=]() { fn.WasCalled(); });
            r.Release();
            fn.SetExpectedCalls(0);
        }

        Expectations::Instance()->Validate();
    }

    TEST_METHOD(RegisteredEvent_WhenDetached_DoesNotCallFunction)
    {
        fn.SetExpectedCalls(0);

        {
            RegisteredEvent r([=]() { fn.WasCalled(); });
            r.Detach();
            r.Release();
        }

        Expectations::Instance()->Validate();
    }

    TEST_METHOD(RegisteredEvent_AddAndsRemovesHandler)
    {
        fn.SetExpectedCalls(1);

        auto s = Make<TestEventSource>();

        auto callback = Callback<IEventHandler<IInspectable*>>(
            [&](IInspectable*, IInspectable*) { fn.WasCalled(); return S_OK; });

        auto r = RegisteredEvent(
            s.Get(),
            &TestEventSource::add_Event,
            &TestEventSource::remove_Event,
            callback.Get());

        Assert::IsTrue(static_cast<bool>(r));

        s->Raise();

        fn.SetExpectedCalls(0);

        r.Release();        
        s->Raise();

        Expectations::Instance()->Validate();
    }
};

