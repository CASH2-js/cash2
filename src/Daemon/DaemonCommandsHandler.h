// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2018-2020 The Cash2 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "Common/ConsoleHandler.h"
#include "CryptoNoteCore/Core.h"
#include "P2p/NodeServer.h"
#include "Logging/LoggerRef.h"
#include "Logging/LoggerManager.h"

class DaemonCommandsHandler
{
public:
  DaemonCommandsHandler(CryptoNote::Core& core, CryptoNote::NodeServer& nodeServer, Logging::LoggerManager& log);

  bool start_handling() {
    m_consoleHandler.start();
    return true;
  }

  void stop_handling() {
    m_consoleHandler.stop();
  }

private:
  bool exit(const std::vector<std::string>& args);
  std::string get_commands_str();
  bool help(const std::vector<std::string>& args);
  bool hide_hr(const std::vector<std::string>& args);
  bool print_bc(const std::vector<std::string>& args);
  bool print_bc_outs(const std::vector<std::string>& args);
  bool print_bci(const std::vector<std::string>& args);
  bool print_block(const std::vector<std::string>& args);
  bool print_block_by_hash(const std::string& arg);
  bool print_block_by_index(uint32_t index);
  bool print_block_helper(uint32_t index);
  bool print_blockchain_height(const std::vector<std::string>& args);
  bool print_circulating_supply(const std::vector<std::string>& args);
  bool print_cn(const std::vector<std::string>& args);
  bool print_cn_count(const std::vector<std::string>& args);
  bool print_difficulty(const std::vector<std::string>& args);
  bool print_grey_pl(const std::vector<std::string>& args);
  bool print_grey_pl_count(const std::vector<std::string>& args);
  bool print_incoming_cn(const std::vector<std::string>& args);
  bool print_incoming_cn_count(const std::vector<std::string>& args);
  bool print_network_height(const std::vector<std::string>& args);
  bool print_outgoing_cn(const std::vector<std::string>& args);
  bool print_outgoing_cn_count(const std::vector<std::string>& args);
  bool print_pl(const std::vector<std::string>& args);
  bool print_pool(const std::vector<std::string>& args);
  bool print_pool_sh(const std::vector<std::string>& args);
  bool print_total_transactions_count(const std::vector<std::string>& args);
  bool print_transaction_fee(const std::vector<std::string>& args);
  bool print_tx(const std::vector<std::string>& args);
  bool print_white_pl(const std::vector<std::string>& args);
  bool print_white_pl_count(const std::vector<std::string>& args);
  bool set_log(const std::vector<std::string>& args);
  bool show_hr(const std::vector<std::string>& args);
  bool start_mining(const std::vector<std::string>& args);
  bool stop_mining(const std::vector<std::string>& args);

  Common::ConsoleHandler m_consoleHandler;
  CryptoNote::Core& m_core;
  CryptoNote::NodeServer& m_nodeServer;
  Logging::LoggerRef m_logger;
  Logging::LoggerManager& m_logManager;
};
