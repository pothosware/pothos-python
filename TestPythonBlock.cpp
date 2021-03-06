// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

POTHOS_TEST_BLOCK("/proxy/python/tests", python_module_import)
{
    auto env = Pothos::ProxyEnvironment::make("python");
    env->findProxy("Pothos");
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_python_module)
{
    auto env = Pothos::ProxyEnvironment::make("python");
    env->findProxy("Pothos.TestPothos").call("main");
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_python_block)
{
    auto feeder = Pothos::BlockRegistry::make("/blocks/feeder_source", "int");
    auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", "int");
    auto forwarder = Pothos::BlockRegistry::make("/python/forwarder", Pothos::DType("int"));

    //create a test plan
    json testPlan;
    testPlan["enableBuffers"] = true;
    testPlan["enableLabels"] = true;
    testPlan["enableMessages"] = true;
    auto expected = feeder.call("feedTestPlan", testPlan.dump());

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, forwarder, 0);
        topology.connect(forwarder, 0, collector, 0);
        std::cout << "topology commit\n";
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.5, 5.0));
    }

    collector.call("verifyTestPlan", expected);
    std::cout << "run done\n";
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_signals_and_slots)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto reg = env->findProxy("Pothos/BlockRegistry");
    auto emitter = Pothos::BlockRegistry::make("/python/simple_signal_emitter");
    auto acceptor = Pothos::BlockRegistry::make("/python/simple_slot_acceptor");

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(emitter, "activateCalled", acceptor, "activateHandler");
        std::cout << "topology commit\n";
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    std::string lastWord = acceptor.call("getLastWord");
    POTHOS_TEST_EQUAL(lastWord, "hello");
}
