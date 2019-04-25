#ifndef GWENJVS_CONFIG_H
#define GWENJVS_CONFIG_H

namespace GwenJVS {

const int kPollingRate = 250;
const int kReadTimeout = 60000;
const int kIdleTimeout = 3600000;
const char kDefaultPort[] = "COM2";
const char kLogLocation[] = "C:\\GameData";

const int kNumPlayersPerNode = 2;
const int kNumNodes = 2;
const int kNumButtons = 13; // test + start + service + UDLR + btn 1-6

} // namespace GwenJVS

#endif