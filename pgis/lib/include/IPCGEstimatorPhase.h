/**
 * File: IPCGEstimatorPhase.h
 * License: Part of the metacg project. Licensed under BSD 3 clause license. See LICENSE.txt file at
 * https://github.com/tudasc/metacg/LICENSE.txt
 */

#ifndef IPCGESTIMATORPHASE_H_
#define IPCGESTIMATORPHASE_H_

#include "EstimatorPhase.h"

#include <map>
#include <queue>
#include <set>

/** RN: instrument the first n levels starting from main */
class FirstNLevelsEstimatorPhase : public EstimatorPhase {
 public:
  FirstNLevelsEstimatorPhase(int levels);
  ~FirstNLevelsEstimatorPhase();

  void modifyGraph(metacg::CgNode* mainMethod);

 private:
  void instrumentLevel(metacg::CgNode* parentNode, int levelsLeft);

  const int levels;
};

class StatisticsEstimatorPhase : public EstimatorPhase {
 public:
  StatisticsEstimatorPhase(bool shouldPrintReport, metacg::Callgraph *cg)
      : EstimatorPhase("StatisticsEstimatorPhase", cg),
        shouldPrintReport(shouldPrintReport),
        numFunctions(0),
        numReachableFunctions(0),
        totalStmts(0),
        stmtsCoveredWithInstr(0),
        stmtsActuallyCovered(0),
        totalVarDecls(0) {}

  void modifyGraph(metacg::CgNode* mainMethod) override;
  void printReport() override;
  long int getCuttoffNumInclStmts();
  long int getCuttoffReversesConditionalBranches() const;
  long int getCuttoffConditionalBranches() const;
  long int getCuttoffRoofline() const;
  long int getCuttoffLoopDepth() const;
  long int getCuttoffGlobalLoopDepth() const;

 private:
  using MapT = std::map<long int, long int>;
  long int getCuttoffValue(const MapT &hist) const;
  long int getUniqueMedianFromHist(const MapT &hist) const;
  long int getMedianFromHist(const MapT &hist) const;
  long int getHalfMaxFromHist(const MapT &hist) const;
  std::string printHist(const MapT &hist, const std::string &name);
  bool shouldPrintReport;
  long int numFunctions;
  long int numReachableFunctions;
  long int totalStmts;
  MapT stmtHist;
  MapT stmtInclHist;
  long int stmtsCoveredWithInstr;
  long int stmtsActuallyCovered;
  long int totalVarDecls;
  MapT conditionalBranchesInclHist;
  MapT reverseConditionalBranchesInclHist;
  MapT rooflineInclHist;
  MapT loopDepthInclHist;
  MapT globalLoopDepthInclHist;
};

/**
 * RN: An optimistic inclusive statement count heuristic.
 * Sums up statement count for all reachable nodes from a startNode.
 * Cycles are counted only once.
 * Edge counts are NOT taken into account.
 */
class StatementCountEstimatorPhase : public EstimatorPhase {
 public:
  explicit StatementCountEstimatorPhase(int numberOfStatementsThreshold, metacg::Callgraph* callgraph, bool inclusiveMetric = true,
                               StatisticsEstimatorPhase *prevStatEP = nullptr);
  ~StatementCountEstimatorPhase() override;

  void modifyGraph(metacg::CgNode* mainMethod) override;
  void estimateStatementCount(metacg::CgNode* startNode, metacg::analysis::ReachabilityAnalysis &ra);

  int getNumStatements(metacg::CgNode* node) { return inclStmtCounts[node]; }

 private:
  int numberOfStatementsThreshold;
  bool inclusiveMetric;
  std::map<metacg::CgNode*, long int> inclStmtCounts;
  StatisticsEstimatorPhase *pSEP;
};

/**
 * @brief The RuntimeEstimatorPhase class
 */
class RuntimeEstimatorPhase : public EstimatorPhase {
 public:
  explicit RuntimeEstimatorPhase(metacg::Callgraph *cg, double runTimeThreshold, bool inclusiveMetric = true);
  ~RuntimeEstimatorPhase() override;

  void modifyGraph(metacg::CgNode* mainMethod) override;
  void estimateRuntime(metacg::CgNode* startNode);
  void doInstrumentation(metacg::CgNode* startNode, metacg::analysis::ReachabilityAnalysis &ra);

 private:
  double runTimeThreshold;
  bool inclusiveMetric;
  std::map<metacg::CgNode*, double> inclRunTime;
};

/**
 * RN: Gets a file with a whitelist of interesting nodes.
 * Instruments all paths to these nodes with naive callpathDifferentiation.
 */
class WLCallpathDifferentiationEstimatorPhase : public EstimatorPhase {
 public:
  explicit WLCallpathDifferentiationEstimatorPhase(std::string whiteListName = "whitelist.txt");
  ~WLCallpathDifferentiationEstimatorPhase() override;

  void modifyGraph(metacg::CgNode* mainMethod) override;

 private:
  CgNodeRawPtrUSet whitelist;  // all whitelisted nodes INCL. their paths to main
  std::string whitelistName;

  void addNodeAndParentsToWhitelist(metacg::CgNode* node);
};

class SummingCountPhaseBase : public EstimatorPhase {
 public:
  SummingCountPhaseBase(long int threshold, const std::string &name, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP,
                        bool inclusive = true);
  ~SummingCountPhaseBase() override;
  void modifyGraph(metacg::CgNode* mainMethod) override;
  static const long int limitThreshold = std::numeric_limits<long int>::max();
  long int getCounted(const metacg::CgNode* node);

 protected:
  void estimateCount(metacg::CgNode* startNode, metacg::analysis::ReachabilityAnalysis &ra);
  virtual long int getPreviousThreshold() const = 0;
  virtual long int getTargetCount(const metacg::CgNode *data) const = 0;
  long int threshold;
  std::map<const metacg::CgNode*, long int> counts;
  StatisticsEstimatorPhase *pSEP;
  virtual void runInitialization();
  const bool inclusive;
};

// Inclusive count
class ConditionalBranchesEstimatorPhase : public SummingCountPhaseBase {
 public:
  explicit ConditionalBranchesEstimatorPhase(long int threshold, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP = nullptr);

 protected:
  long int getPreviousThreshold() const override;
  long int getTargetCount(const metacg::CgNode *data) const override;
};

// Calculates the target count by subtracting the conditional branches from the max amount of conditional branches
// Inclusive count
class ConditionalBranchesReverseEstimatorPhase : public SummingCountPhaseBase {
 public:
  explicit ConditionalBranchesReverseEstimatorPhase(long int threshold, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP = nullptr);

 protected:
  long int maxBranches;
  long int getPreviousThreshold() const override;
  long int getTargetCount(const metacg::CgNode *data) const override;
  void runInitialization() override;
};

// Uses the combined Number of Floating-Point Operations and memory accesses
// Inclusive count
class FPAndMemOpsEstimatorPhase : public SummingCountPhaseBase {
 public:
  explicit FPAndMemOpsEstimatorPhase(long int threshold, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP = nullptr);

 protected:
  long int getPreviousThreshold() const override;
  long int getTargetCount(const metacg::CgNode *data) const override;
};

// Exclusive count
class LoopDepthEstimatorPhase : public SummingCountPhaseBase {
 public:
  explicit LoopDepthEstimatorPhase(long int threshold, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP = nullptr);

 protected:
  long int getPreviousThreshold() const override;
  long int getTargetCount(const metacg::CgNode *data) const override;
};

// Exclusive count
class GlobalLoopDepthEstimatorPhase : public SummingCountPhaseBase {
 public:
  explicit GlobalLoopDepthEstimatorPhase(long int threshold, metacg::Callgraph* callgraph, StatisticsEstimatorPhase *prevStatEP = nullptr);

 protected:
  long int getPreviousThreshold() const override;
  long int getTargetCount(const metacg::CgNode *data) const override;
};

#endif
