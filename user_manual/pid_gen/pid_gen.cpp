/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * pid_gen.cpp
 * Generate the markdown tables from the supported PIDs.
 * Copyright (C) 2016 Simon Newton
 */

#include <ola/Constants.h>
#include <ola/base/Flags.h>
#include <ola/base/Init.h>
#include <ola/base/SysExits.h>
#include <ola/file/Util.h>
#include <ola/network/NetworkUtils.h>
#include <ola/rdm/PidStoreHelper.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMCommandSerializer.h>
#include <ola/rdm/RDMEnums.h>
#include <ola/rdm/UID.h>

#include <string.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <fstream>
#include <memory>
#include <vector>

#include "Array.h"
#include "dimmer_model.h"
#include "led_model.h"
#include "moving_light.h"
#include "network_model.h"
#include "proxy_model.h"
#include "sensor_model.h"
#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"

using ola::rdm::PidDescriptor;
using ola::rdm::PidStoreHelper;
using ola::rdm::UID;
using std::cerr;
using std::cout;
using std::endl;
using std::setw;
using std::string;
using std::unique_ptr;
using std::vector;

const uint8_t TEST_UID[] = {0x7a, 0x70, 0xff, 0xff, 0xfe, 0x10};

const uint16_t MANUFACTURER_RANGE = 0x8000;

DEFINE_string(pid_location, "", "Location of the RDM PID Store");
DEFINE_string(output_dir, "", "Directory to output files to");

struct ModelProperties {
  string name;
  void (*init_fn)();
  const ModelEntry *entry;
};

const ModelProperties MODELS[] = {
  {
    "led",
    LEDModel_Initialize,
    &LED_MODEL_ENTRY,
  },
  {
    "proxy",
    ProxyModel_Initialize,
    &PROXY_MODEL_ENTRY,
  },
  {
    "moving_light",
    MovingLightModel_Initialize,
    &MOVING_LIGHT_MODEL_ENTRY,
  },
  {
    "sensor",
    SensorModel_Initialize,
    &SENSOR_MODEL_ENTRY,
  },
  {
    "network",
    NetworkModel_Initialize,
    &NETWORK_MODEL_ENTRY,
  },
  {
    "dimmer",
    DimmerModel_Initialize,
    &DIMMER_MODEL_ENTRY,
  },
};

struct PidEntry {
  string name;  // name of the PID
  uint16_t value;  // value of the PID
  bool supports_get;
  bool supports_set;

  bool operator<(const PidEntry &other) const {
    return this->value < other.value;
  }
};

string BuildLink(uint16_t pid) {
  std::stringstream str;
  str << "http://rdm.openlighting.org/pid/display?manufacturer=";
  if (pid < MANUFACTURER_RANGE) {
    str << 0;
  } else {
    str << ola::OPEN_LIGHTING_ESTA_CODE;
  }
  str << "&amp;pid=" << pid;
  return str.str();
}

void OutputTable(const string &model, const vector<PidEntry> &rows) {
  const string file_name = ola::file::JoinPaths(FLAGS_output_dir.str(),
                                                model + ".html");

  std::ofstream output;
  output.open(file_name);
  if (!output.is_open()) {
    cerr << "Failed to open " << file_name << endl;
    return;
  }

  output << "<table class=\"doxtable\"><tr><th>PID</th><th>Get</th><th>Set</th>"
         << "</tr>" << endl;

  for (const auto &row : rows) {
    output << " <tr><td><a href=\"" << BuildLink(row.value) << "\">"
           << row.name << "</a></td><td>"
           << (row.supports_get ? "Y" : "") << "</td><td>"
           << (row.supports_set ? "Y" : "") << "</td></tr>" << endl;
  }
  output << "</table>" << endl;
  output.close();
  cout << "Output " << file_name << endl;
}

void GenerateTable(PidStoreHelper *pid_helper, const ModelProperties &model) {
  UID controller_uid(0x7a70, 0x00000000);
  UID device_uid(TEST_UID);

  RDMResponderSettings settings;
  memcpy(settings.uid, TEST_UID, UID_LENGTH);
  RDMResponder_Initialize(&settings);

  model.init_fn();
  model.entry->activate_fn();

  ola::rdm::RDMGetRequest request(controller_uid, device_uid, 0, 0, 0,
                                  PID_SUPPORTED_PARAMETERS, nullptr, 0);

  ola::io::ByteString data;
  data.push_back(ola::rdm::RDMCommand::START_CODE);
  if (!ola::rdm::RDMCommandSerializer::Pack(request, &data)) {
    cerr << "Failed to pack PID_SUPPORTED_PARAMETERS for " << model.name
         << endl;
    return;
  }

  int size = model.entry->request_fn(
      reinterpret_cast<const RDMHeader*>(data.data()), request.ParamData());

  ola::rdm::RDMStatusCode status_code;
  std::unique_ptr<ola::rdm::RDMResponse> response(
      ola::rdm::RDMResponse::InflateFromData(
          g_rdm_buffer + 1, size - 1, &status_code, &request));

  if (response.get() == nullptr || response->ParamDataSize() % 2 != 0) {
    cerr << "Invalid response for " << model.name << endl;
    return;
  }

  const uint8_t *param_data = response->ParamData();
  vector<PidEntry> rows;
  for (; param_data < response->ParamData() + response->ParamDataSize();
       param_data += 2) {
    uint16_t pid = ola::utils::JoinUInt8(param_data[0], param_data[1]);

    const PidDescriptor *descriptor;
    if (pid < MANUFACTURER_RANGE) {
      descriptor = pid_helper->GetDescriptor(pid, 0);
    } else {
      descriptor = pid_helper->GetDescriptor(pid, ola::OPEN_LIGHTING_ESTA_CODE);
    }

    if (!descriptor) {
      cerr << "Failed to find descriptor for " << ola::strings::ToHex(pid)
           << endl;
    } else {
      PidEntry entry = {
        descriptor->Name(),
        pid,
        descriptor->GetRequest() != nullptr,
        descriptor->SetRequest() != nullptr,
      };
      rows.push_back(entry);
    }
  }
  std::sort(rows.begin(), rows.end());
  OutputTable(model.name, rows);
}

int main(int argc, char* argv[]) {
  ola::AppInit(
      &argc,
      argv,
      "[options]",
      "generate Markdown tables for each model's supported parameters");

  PidStoreHelper pid_helper(FLAGS_pid_location.str());
  if (!pid_helper.Init()) {
    cerr << "Failed to load PIDs" << endl;
    return ola::EXIT_DATAERR;
  }

  for (unsigned int i = 0; i < arraysize(MODELS); i++) {
    GenerateTable(&pid_helper, MODELS[i]);
  }
}
