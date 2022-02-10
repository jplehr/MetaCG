/**
 * File: CgNodeMetaData.h
 * License: Part of the MetaCG project. Licensed under BSD 3 clause license. See LICENSE.txt file at
 * https://github.com/tudasc/metacg/LICENSE.txt
 */

#ifndef PIRA_CGNODE_METADATA_H
#define PIRA_CGNODE_METADATA_H

// clang-format off
// Graph library
#include "CgNodePtr.h"
#include "MetaData.h"

// PGIS library
#include "ExtrapConnection.h"
#include "config/GlobalConfig.h"

#include "CgLocation.h"

#include "nlohmann/json.hpp"

// clang-format on

namespace pira {

inline void assert_pira_one_data() {
  assert(false && "PIRA I data should be available in node");
}

typedef unsigned long long Statements;

/**
 * This class holds basic profile information, e.g., from reading a CUBE
 */
class BaseProfileData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "BaseProfileData"; }

  // Regular profile data
  // Warning: This function is *not* used by the Cube reader
  void setCallData(CgNodePtr parentNode, unsigned long long calls, double timeInSeconds, double inclusiveTimeInSeconds,
                   int threadId, int procId) {
    callFrom[parentNode] += calls;
    timeFrom[parentNode] += timeInSeconds;
    this->timeInSeconds += timeInSeconds;
    this->inclTimeInSeconds += inclusiveTimeInSeconds;
    this->threadId = threadId;
    this->processId = procId;
    this->cgLoc.emplace_back(CgLocation(timeInSeconds, inclusiveTimeInSeconds, threadId, procId, calls));
  }
  unsigned long long getNumberOfCalls() const { return this->numCalls; }
  void setNumberOfCalls(unsigned long long nrCall) { this->numCalls = nrCall; }

  double getRuntimeInSeconds() const { return this->timeInSeconds; }
  void setRuntimeInSeconds(double newRuntimeInSeconds) { this->timeInSeconds = newRuntimeInSeconds; }
  double getRuntimeInSecondsForParent(CgNodePtr parent) { return this->timeFrom[parent]; }

  void setInclusiveRuntimeInSeconds(double newInclusiveTimeInSeconds) {
    this->inclTimeInSeconds = newInclusiveTimeInSeconds;
  }
  double getInclusiveRuntimeInSeconds() const { return this->inclTimeInSeconds; }
  unsigned long long getNumberOfCallsWithCurrentEdges() const {
    auto v = 0ull;
    for (const auto &p : callFrom) {
      v += p.second;
    }
    return v;
  }
  unsigned long long getNumberOfCalls(CgNodePtr parentNode) { return callFrom[parentNode]; }

  const std::vector<CgLocation> &getCgLocation() const { return cgLoc; }

  void pushCgLocation(CgLocation toPush) { this->cgLoc.push_back(toPush); }

 private:
  unsigned long long numCalls{0};
  double timeInSeconds{.0};
  double inclTimeInSeconds{.0};
  int threadId{0};
  int processId{0};
  std::unordered_map<CgNodePtr, unsigned long long> callFrom;
  std::unordered_map<CgNodePtr, double> timeFrom;
  std::vector<CgLocation> cgLoc;
};

inline void to_json(nlohmann::json &j, const BaseProfileData &data) {
  j = nlohmann::json{{"numCalls", data.getNumberOfCalls()}, {"timeInSeconds", data.getRuntimeInSeconds()}};
}

/**
 * This class holds data relevant to the PIRA I analyses.
 * Most notably, it offers the number of statements and the principal (dominant) runtime node
 */
class PiraOneData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "PiraOneData"; }

  void setNumberOfStatements(int numStmts) { this->numStmts = numStmts; }
  int getNumberOfStatements() const { return this->numStmts; }
  void setHasBody(bool hasBody=true) { this->hasBody = hasBody; }
  bool getHasBody() const { return this->hasBody; }
  void setDominantRuntime(bool dominantRuntime = true) { this->dominantRuntime = dominantRuntime; }
  bool isDominantRuntime() const { return this->dominantRuntime; }
  void setComesFromCube(bool fromCube = true) { this->wasInPreviousProfile = fromCube; }
  bool comesFromCube() const { return this->wasInPreviousProfile;}
  bool inPreviousProfile() const { return wasInPreviousProfile; }

 private:
  bool wasInPreviousProfile{false};
  bool dominantRuntime{false};
  bool hasBody{false};
  int numStmts{0};
};

template<typename T>
inline void setPiraOneData(T node, int numStmts = 0, bool hasBody = false, bool dominantRuntime = false, bool inPrevProfile = false) {
  const auto &[has, data] = node->template checkAndGet<PiraOneData>();
  if (has) {
    data->setNumberOfStatements(numStmts);
    data->setHasBody(hasBody);
    data->setDominantRuntime(dominantRuntime);
    data->setComesFromCube(inPrevProfile);
  } else {
    assert_pira_one_data();
  }
}

inline void to_json(nlohmann::json &j, const PiraOneData &data) {
  j = nlohmann::json{{"numStatements", data.getNumberOfStatements()}};
}

/**
 * This class holds data relevant to the PIRA II anslyses.
 * Most notably it encapsulates the Extra-P peformance models
 */
class PiraTwoData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "PiraTwoData"; }

  PiraTwoData() : epCon({}, {}), params(), rtVec(), numReps(0) {}
  explicit PiraTwoData(const extrapconnection::ExtrapConnector &ec) : epCon(ec), params(), rtVec(), numReps(0) {}
  PiraTwoData(const PiraTwoData &other)
      : epCon(other.epCon), params(other.params), rtVec(other.rtVec), numReps(other.numReps) {
    spdlog::get("console")->trace("PiraTwo Copy CTor\n\tother: {}\n\tThis: {}", other.rtVec.size(), rtVec.size());
  }

  void setExtrapModelConnector(extrapconnection::ExtrapConnector epCon) { this->epCon = epCon; }
  extrapconnection::ExtrapConnector &getExtrapModelConnector() { return epCon; }
  const extrapconnection::ExtrapConnector &getExtrapModelConnector() const { return epCon; }

  void setExtrapParameters(std::vector<std::pair<std::string, std::vector<int>>> params) { this->params = params; }
  auto &getExtrapParameters() const { return this->params; }

  void addToRuntimeVec(double runtime) { this->rtVec.push_back(runtime); }
  auto &getRuntimeVec() const { return this->rtVec; }

  auto &getExtrapModel() const { return epCon.getEPModelFunction(); }

  bool hasExtrapModel() const { return epCon.hasModels(); }

  int getNumReps() const { return numReps; }

 private:
  extrapconnection::ExtrapConnector epCon;
  std::vector<std::pair<std::string, std::vector<int>>> params;
  std::vector<double> rtVec;
  int numReps;
};

class FilePropertiesMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "FilePropertiesMetaData"; }
  FilePropertiesMetaData() : origin("INVALID"), fromSystemInclude(false) {}
  std::string origin;
  bool fromSystemInclude;
};

class CodeStatisticsMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "CodeStatisticsMetaData"; }
  int numVars{0};
};

class NumConditionalBranchMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "NumConditionalBranchMetaData"; }
  int numConditionalBranches{0};
};

class NumOperationsMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "NumOperationsMetaData"; }
  int numberOfIntOps{0};
  int numberOfFloatOps{0};
  int numberOfControlFlowOps{0};
  int numberOfMemoryAccesses{0};
};

class LoopDepthMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "LoopDepthMetaData"; }
  int loopDepth{0};
};

class GlobalLoopDepthMetaData : public metacg::MetaData {
 public:
  static constexpr const char *key() { return "GlobalLoopDepthMetaData"; }
  int globalLoopDepth{0};
};

/**
 * TODO This works for only a single parameter for now!
 */
template <typename C1, typename C2>
auto valTup(C1 co, C2 ct, int numReps) {
  // TODO This assert seems wrong
  // assert(co.size() == ct.size() && "Can only value-tuple evenly sized containers");
  std::vector<
      std::pair<typename C1::value_type, std::pair<std::string, typename C2::value_type::second_type::value_type>>>
      res;
  if (ct.empty()) {
    return res;
  }
  assert(ct.size() == 1 && "Current limitation, only single parameter possible");
  auto coIt = std::begin(co);
  // Compute the median per numReps from co first.
  const auto median = [&](auto startIt, auto numElems) {
    if (numElems % 2 == 0) {
      return startIt[(numElems / 2) + 1];
    }
    return startIt[numElems / 2];
  };
  auto innerC = ct[0].second;
  auto ctIt = std::begin(innerC);
  res.reserve(co.size());
  for (; coIt != co.end() && ctIt != innerC.end(); std::advance(coIt, numReps), ++ctIt) {
    res.push_back(std::make_pair(median(coIt, numReps), std::make_pair(ct[0].first, *ctIt)));
  }
  return res;
}

inline void to_json(nlohmann::json &j, const PiraTwoData &data) {
  auto &gOpts = pgis::config::GlobalConfig::get();
  auto rtOnly = gOpts.getAs<bool>("runtime-only");

  auto rtAndParams = valTup(data.getRuntimeVec(), data.getExtrapParameters(), data.getNumReps());
  nlohmann::json experiments;
  for (auto elem : rtAndParams) {
    nlohmann::json exp{};
    exp["runtime"] = elem.first;
    exp[elem.second.first] = elem.second.second;
    experiments += exp;
  }
  if (!rtOnly) {
    j = nlohmann::json{{"experiments", experiments},
                       {"model", data.getExtrapModel()->getAsString(data.getExtrapModelConnector().getParamList())}};
  } else {
    j = nlohmann::json{{"experiments", experiments}};
  }
  spdlog::get("console")->debug("PiraTwoData to_json:\n{}", j.dump());
}

}  // namespace pira

#endif
