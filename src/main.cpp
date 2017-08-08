// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"
#include "rdt/pqos.hpp"

int main() {
    int exit_code = -1;
    auto pqos = new rdt::PQOS();
    rdt::Collector *rdt;
    try
    {
        rdt = new rdt::Collector(pqos);
        start_collector(rdt, rdt->get_plugin_meta());
        exit_code = 0;
    } catch (Plugin::PluginException &e) {
        fprintf(stderr, "Plugin exception: %s\n", e.what());
    } catch (const char* what) {
        fprintf(stderr, "Cannot launch RDT Collector: %s\n", what);
    }
    delete(rdt);
    delete(pqos);
    return exit_code;
}
