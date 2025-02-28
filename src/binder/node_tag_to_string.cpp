

#include "../include/binder/binder.h"

namespace hmssql {

auto Binder::NodeTagToString(duckdb_libpgquery::PGNodeTag type) -> std::string {
  switch (type) {
    case duckdb_libpgquery::T_PGInvalid:
      return "T_Invalid";
    case duckdb_libpgquery::T_PGIndexInfo:
      return "T_IndexInfo";
    case duckdb_libpgquery::T_PGExprContext:
      return "T_ExprContext";
    case duckdb_libpgquery::T_PGProjectionInfo:
      return "T_ProjectionInfo";
    case duckdb_libpgquery::T_PGJunkFilter:
      return "T_JunkFilter";
    case duckdb_libpgquery::T_PGResultRelInfo:
      return "T_ResultRelInfo";
    case duckdb_libpgquery::T_PGEState:
      return "T_EState";
    case duckdb_libpgquery::T_PGTupleTableSlot:
      return "T_TupleTableSlot";
    case duckdb_libpgquery::T_PGPlan:
      return "T_Plan";
    case duckdb_libpgquery::T_PGResult:
      return "T_Result";
    case duckdb_libpgquery::T_PGProjectSet:
      return "T_ProjectSet";
    case duckdb_libpgquery::T_PGModifyTable:
      return "T_ModifyTable";
    case duckdb_libpgquery::T_PGAppend:
      return "T_Append";
    case duckdb_libpgquery::T_PGMergeAppend:
      return "T_MergeAppend";
    case duckdb_libpgquery::T_PGRecursiveUnion:
      return "T_RecursiveUnion";
    case duckdb_libpgquery::T_PGBitmapAnd:
      return "T_BitmapAnd";
    case duckdb_libpgquery::T_PGBitmapOr:
      return "T_BitmapOr";
    case duckdb_libpgquery::T_PGScan:
      return "T_Scan";
    case duckdb_libpgquery::T_PGSeqScan:
      return "T_SeqScan";
    case duckdb_libpgquery::T_PGSampleScan:
      return "T_SampleScan";
    case duckdb_libpgquery::T_PGIndexScan:
      return "T_IndexScan";
    case duckdb_libpgquery::T_PGIndexOnlyScan:
      return "T_IndexOnlyScan";
    case duckdb_libpgquery::T_PGBitmapIndexScan:
      return "T_BitmapIndexScan";
    case duckdb_libpgquery::T_PGBitmapHeapScan:
      return "T_BitmapHeapScan";
    case duckdb_libpgquery::T_PGTidScan:
      return "T_TidScan";
    case duckdb_libpgquery::T_PGSubqueryScan:
      return "T_SubqueryScan";
    case duckdb_libpgquery::T_PGFunctionScan:
      return "T_FunctionScan";
    case duckdb_libpgquery::T_PGValuesScan:
      return "T_ValuesScan";
    case duckdb_libpgquery::T_PGTableFuncScan:
      return "T_TableFuncScan";
    case duckdb_libpgquery::T_PGCteScan:
      return "T_CteScan";
    case duckdb_libpgquery::T_PGNamedTuplestoreScan:
      return "T_NamedTuplestoreScan";
    case duckdb_libpgquery::T_PGWorkTableScan:
      return "T_WorkTableScan";
    case duckdb_libpgquery::T_PGForeignScan:
      return "T_ForeignScan";
    case duckdb_libpgquery::T_PGCustomScan:
      return "T_CustomScan";
    case duckdb_libpgquery::T_PGJoin:
      return "T_Join";
    case duckdb_libpgquery::T_PGNestLoop:
      return "T_NestLoop";
    case duckdb_libpgquery::T_PGMergeJoin:
      return "T_MergeJoin";
    case duckdb_libpgquery::T_PGHashJoin:
      return "T_HashJoin";
    case duckdb_libpgquery::T_PGMaterial:
      return "T_Material";
    case duckdb_libpgquery::T_PGSort:
      return "T_Sort";
    case duckdb_libpgquery::T_PGGroup:
      return "T_Group";
    case duckdb_libpgquery::T_PGAgg:
      return "T_Agg";
    case duckdb_libpgquery::T_PGWindowAgg:
      return "T_WindowAgg";
    case duckdb_libpgquery::T_PGUnique:
      return "T_Unique";
    case duckdb_libpgquery::T_PGGather:
      return "T_Gather";
    case duckdb_libpgquery::T_PGGatherMerge:
      return "T_GatherMerge";
    case duckdb_libpgquery::T_PGHash:
      return "T_Hash";
    case duckdb_libpgquery::T_PGSetOp:
      return "T_SetOp";
    case duckdb_libpgquery::T_PGLockRows:
      return "T_LockRows";
    case duckdb_libpgquery::T_PGLimit:
      return "T_Limit";
    case duckdb_libpgquery::T_PGNestLoopParam:
      return "T_NestLoopParam";
    case duckdb_libpgquery::T_PGPlanRowMark:
      return "T_PlanRowMark";
    case duckdb_libpgquery::T_PGPlanInvalItem:
      return "T_PlanInvalItem";
    case duckdb_libpgquery::T_PGPlanState:
      return "T_PlanState";
    case duckdb_libpgquery::T_PGResultState:
      return "T_ResultState";
    case duckdb_libpgquery::T_PGProjectSetState:
      return "T_ProjectSetState";
    case duckdb_libpgquery::T_PGModifyTableState:
      return "T_ModifyTableState";
    case duckdb_libpgquery::T_PGAppendState:
      return "T_AppendState";
    case duckdb_libpgquery::T_PGMergeAppendState:
      return "T_MergeAppendState";
    case duckdb_libpgquery::T_PGRecursiveUnionState:
      return "T_RecursiveUnionState";
    case duckdb_libpgquery::T_PGBitmapAndState:
      return "T_BitmapAndState";
    case duckdb_libpgquery::T_PGBitmapOrState:
      return "T_BitmapOrState";
    case duckdb_libpgquery::T_PGScanState:
      return "T_ScanState";
    case duckdb_libpgquery::T_PGSeqScanState:
      return "T_SeqScanState";
    case duckdb_libpgquery::T_PGSampleScanState:
      return "T_SampleScanState";
    case duckdb_libpgquery::T_PGIndexScanState:
      return "T_IndexScanState";
    case duckdb_libpgquery::T_PGIndexOnlyScanState:
      return "T_IndexOnlyScanState";
    case duckdb_libpgquery::T_PGBitmapIndexScanState:
      return "T_BitmapIndexScanState";
    case duckdb_libpgquery::T_PGBitmapHeapScanState:
      return "T_BitmapHeapScanState";
    case duckdb_libpgquery::T_PGTidScanState:
      return "T_TidScanState";
    case duckdb_libpgquery::T_PGSubqueryScanState:
      return "T_SubqueryScanState";
    case duckdb_libpgquery::T_PGFunctionScanState:
      return "T_FunctionScanState";
    case duckdb_libpgquery::T_PGTableFuncScanState:
      return "T_TableFuncScanState";
    case duckdb_libpgquery::T_PGValuesScanState:
      return "T_ValuesScanState";
    case duckdb_libpgquery::T_PGCteScanState:
      return "T_CteScanState";
    case duckdb_libpgquery::T_PGNamedTuplestoreScanState:
      return "T_NamedTuplestoreScanState";
    case duckdb_libpgquery::T_PGWorkTableScanState:
      return "T_WorkTableScanState";
    case duckdb_libpgquery::T_PGForeignScanState:
      return "T_ForeignScanState";
    case duckdb_libpgquery::T_PGCustomScanState:
      return "T_CustomScanState";
    case duckdb_libpgquery::T_PGJoinState:
      return "T_JoinState";
    case duckdb_libpgquery::T_PGNestLoopState:
      return "T_NestLoopState";
    case duckdb_libpgquery::T_PGMergeJoinState:
      return "T_MergeJoinState";
    case duckdb_libpgquery::T_PGHashJoinState:
      return "T_HashJoinState";
    case duckdb_libpgquery::T_PGMaterialState:
      return "T_MaterialState";
    case duckdb_libpgquery::T_PGSortState:
      return "T_SortState";
    case duckdb_libpgquery::T_PGGroupState:
      return "T_GroupState";
    case duckdb_libpgquery::T_PGAggState:
      return "T_AggState";
    case duckdb_libpgquery::T_PGWindowAggState:
      return "T_WindowAggState";
    case duckdb_libpgquery::T_PGUniqueState:
      return "T_UniqueState";
    case duckdb_libpgquery::T_PGGatherState:
      return "T_GatherState";
    case duckdb_libpgquery::T_PGGatherMergeState:
      return "T_GatherMergeState";
    case duckdb_libpgquery::T_PGHashState:
      return "T_HashState";
    case duckdb_libpgquery::T_PGSetOpState:
      return "T_SetOpState";
    case duckdb_libpgquery::T_PGLockRowsState:
      return "T_LockRowsState";
    case duckdb_libpgquery::T_PGLimitState:
      return "T_LimitState";
    case duckdb_libpgquery::T_PGAlias:
      return "T_Alias";
    case duckdb_libpgquery::T_PGRangeVar:
      return "T_RangeVar";
    case duckdb_libpgquery::T_PGTableFunc:
      return "T_TableFunc";
    case duckdb_libpgquery::T_PGExpr:
      return "T_Expr";
    case duckdb_libpgquery::T_PGVar:
      return "T_Var";
    case duckdb_libpgquery::T_PGConst:
      return "T_Const";
    case duckdb_libpgquery::T_PGParam:
      return "T_Param";
    case duckdb_libpgquery::T_PGAggref:
      return "T_Aggref";
    case duckdb_libpgquery::T_PGGroupingFunc:
      return "T_GroupingFunc";
    case duckdb_libpgquery::T_PGWindowFunc:
      return "T_WindowFunc";
    case duckdb_libpgquery::T_PGArrayRef:
      return "T_ArrayRef";
    case duckdb_libpgquery::T_PGFuncExpr:
      return "T_FuncExpr";
    case duckdb_libpgquery::T_PGNamedArgExpr:
      return "T_NamedArgExpr";
    case duckdb_libpgquery::T_PGOpExpr:
      return "T_OpExpr";
    case duckdb_libpgquery::T_PGDistinctExpr:
      return "T_DistinctExpr";
    case duckdb_libpgquery::T_PGNullIfExpr:
      return "T_NullIfExpr";
    case duckdb_libpgquery::T_PGScalarArrayOpExpr:
      return "T_ScalarArrayOpExpr";
    case duckdb_libpgquery::T_PGBoolExpr:
      return "T_BoolExpr";
    case duckdb_libpgquery::T_PGSubLink:
      return "T_SubLink";
    case duckdb_libpgquery::T_PGSubPlan:
      return "T_SubPlan";
    case duckdb_libpgquery::T_PGAlternativeSubPlan:
      return "T_AlternativeSubPlan";
    case duckdb_libpgquery::T_PGFieldSelect:
      return "T_FieldSelect";
    case duckdb_libpgquery::T_PGFieldStore:
      return "T_FieldStore";
    case duckdb_libpgquery::T_PGRelabelType:
      return "T_RelabelType";
    case duckdb_libpgquery::T_PGCoerceViaIO:
      return "T_CoerceViaIO";
    case duckdb_libpgquery::T_PGArrayCoerceExpr:
      return "T_ArrayCoerceExpr";
    case duckdb_libpgquery::T_PGConvertRowtypeExpr:
      return "T_ConvertRowtypeExpr";
    case duckdb_libpgquery::T_PGCollateExpr:
      return "T_CollateExpr";
    case duckdb_libpgquery::T_PGCaseExpr:
      return "T_CaseExpr";
    case duckdb_libpgquery::T_PGCaseWhen:
      return "T_CaseWhen";
    case duckdb_libpgquery::T_PGCaseTestExpr:
      return "T_CaseTestExpr";
    case duckdb_libpgquery::T_PGArrayExpr:
      return "T_ArrayExpr";
    case duckdb_libpgquery::T_PGRowExpr:
      return "T_RowExpr";
    case duckdb_libpgquery::T_PGRowCompareExpr:
      return "T_RowCompareExpr";
    case duckdb_libpgquery::T_PGCoalesceExpr:
      return "T_CoalesceExpr";
    case duckdb_libpgquery::T_PGMinMaxExpr:
      return "T_MinMaxExpr";
    case duckdb_libpgquery::T_PGSQLValueFunction:
      return "T_SQLValueFunction";
    case duckdb_libpgquery::T_PGXmlExpr:
      return "T_XmlExpr";
    case duckdb_libpgquery::T_PGNullTest:
      return "T_NullTest";
    case duckdb_libpgquery::T_PGBooleanTest:
      return "T_BooleanTest";
    case duckdb_libpgquery::T_PGCoerceToDomain:
      return "T_CoerceToDomain";
    case duckdb_libpgquery::T_PGCoerceToDomainValue:
      return "T_CoerceToDomainValue";
    case duckdb_libpgquery::T_PGSetToDefault:
      return "T_SetToDefault";
    case duckdb_libpgquery::T_PGCurrentOfExpr:
      return "T_CurrentOfExpr";
    case duckdb_libpgquery::T_PGNextValueExpr:
      return "T_NextValueExpr";
    case duckdb_libpgquery::T_PGInferenceElem:
      return "T_InferenceElem";
    case duckdb_libpgquery::T_PGTargetEntry:
      return "T_TargetEntry";
    case duckdb_libpgquery::T_PGRangeTblRef:
      return "T_RangeTblRef";
    case duckdb_libpgquery::T_PGJoinExpr:
      return "T_JoinExpr";
    case duckdb_libpgquery::T_PGFromExpr:
      return "T_FromExpr";
    case duckdb_libpgquery::T_PGOnConflictExpr:
      return "T_OnConflictExpr";
    case duckdb_libpgquery::T_PGIntoClause:
      return "T_IntoClause";
    case duckdb_libpgquery::T_PGExprState:
      return "T_ExprState";
    case duckdb_libpgquery::T_PGAggrefExprState:
      return "T_AggrefExprState";
    case duckdb_libpgquery::T_PGWindowFuncExprState:
      return "T_WindowFuncExprState";
    case duckdb_libpgquery::T_PGSetExprState:
      return "T_SetExprState";
    case duckdb_libpgquery::T_PGSubPlanState:
      return "T_SubPlanState";
    case duckdb_libpgquery::T_PGAlternativeSubPlanState:
      return "T_AlternativeSubPlanState";
    case duckdb_libpgquery::T_PGDomainConstraintState:
      return "T_DomainConstraintState";
    case duckdb_libpgquery::T_PGPlannerInfo:
      return "T_PlannerInfo";
    case duckdb_libpgquery::T_PGPlannerGlobal:
      return "T_PlannerGlobal";
    case duckdb_libpgquery::T_PGRelOptInfo:
      return "T_RelOptInfo";
    case duckdb_libpgquery::T_PGIndexOptInfo:
      return "T_IndexOptInfo";
    case duckdb_libpgquery::T_PGForeignKeyOptInfo:
      return "T_ForeignKeyOptInfo";
    case duckdb_libpgquery::T_PGParamPathInfo:
      return "T_ParamPathInfo";
    case duckdb_libpgquery::T_PGPath:
      return "T_Path";
    case duckdb_libpgquery::T_PGIndexPath:
      return "T_IndexPath";
    case duckdb_libpgquery::T_PGBitmapHeapPath:
      return "T_BitmapHeapPath";
    case duckdb_libpgquery::T_PGBitmapAndPath:
      return "T_BitmapAndPath";
    case duckdb_libpgquery::T_PGBitmapOrPath:
      return "T_BitmapOrPath";
    case duckdb_libpgquery::T_PGTidPath:
      return "T_TidPath";
    case duckdb_libpgquery::T_PGSubqueryScanPath:
      return "T_SubqueryScanPath";
    case duckdb_libpgquery::T_PGForeignPath:
      return "T_ForeignPath";
    case duckdb_libpgquery::T_PGCustomPath:
      return "T_CustomPath";
    case duckdb_libpgquery::T_PGNestPath:
      return "T_NestPath";
    case duckdb_libpgquery::T_PGMergePath:
      return "T_MergePath";
    case duckdb_libpgquery::T_PGHashPath:
      return "T_HashPath";
    case duckdb_libpgquery::T_PGAppendPath:
      return "T_AppendPath";
    case duckdb_libpgquery::T_PGMergeAppendPath:
      return "T_MergeAppendPath";
    case duckdb_libpgquery::T_PGResultPath:
      return "T_ResultPath";
    case duckdb_libpgquery::T_PGMaterialPath:
      return "T_MaterialPath";
    case duckdb_libpgquery::T_PGUniquePath:
      return "T_UniquePath";
    case duckdb_libpgquery::T_PGGatherPath:
      return "T_GatherPath";
    case duckdb_libpgquery::T_PGGatherMergePath:
      return "T_GatherMergePath";
    case duckdb_libpgquery::T_PGProjectionPath:
      return "T_ProjectionPath";
    case duckdb_libpgquery::T_PGProjectSetPath:
      return "T_ProjectSetPath";
    case duckdb_libpgquery::T_PGSortPath:
      return "T_SortPath";
    case duckdb_libpgquery::T_PGGroupPath:
      return "T_GroupPath";
    case duckdb_libpgquery::T_PGUpperUniquePath:
      return "T_UpperUniquePath";
    case duckdb_libpgquery::T_PGAggPath:
      return "T_AggPath";
    case duckdb_libpgquery::T_PGGroupingSetsPath:
      return "T_GroupingSetsPath";
    case duckdb_libpgquery::T_PGMinMaxAggPath:
      return "T_MinMaxAggPath";
    case duckdb_libpgquery::T_PGWindowAggPath:
      return "T_WindowAggPath";
    case duckdb_libpgquery::T_PGSetOpPath:
      return "T_SetOpPath";
    case duckdb_libpgquery::T_PGRecursiveUnionPath:
      return "T_RecursiveUnionPath";
    case duckdb_libpgquery::T_PGLockRowsPath:
      return "T_LockRowsPath";
    case duckdb_libpgquery::T_PGModifyTablePath:
      return "T_ModifyTablePath";
    case duckdb_libpgquery::T_PGLimitPath:
      return "T_LimitPath";
    case duckdb_libpgquery::T_PGEquivalenceClass:
      return "T_EquivalenceClass";
    case duckdb_libpgquery::T_PGEquivalenceMember:
      return "T_EquivalenceMember";
    case duckdb_libpgquery::T_PGPathKey:
      return "T_PathKey";
    case duckdb_libpgquery::T_PGPathTarget:
      return "T_PathTarget";
    case duckdb_libpgquery::T_PGRestrictInfo:
      return "T_RestrictInfo";
    case duckdb_libpgquery::T_PGPlaceHolderVar:
      return "T_PlaceHolderVar";
    case duckdb_libpgquery::T_PGSpecialJoinInfo:
      return "T_SpecialJoinInfo";
    case duckdb_libpgquery::T_PGAppendRelInfo:
      return "T_AppendRelInfo";
    case duckdb_libpgquery::T_PGPartitionedChildRelInfo:
      return "T_PartitionedChildRelInfo";
    case duckdb_libpgquery::T_PGPlaceHolderInfo:
      return "T_PlaceHolderInfo";
    case duckdb_libpgquery::T_PGMinMaxAggInfo:
      return "T_MinMaxAggInfo";
    case duckdb_libpgquery::T_PGPlannerParamItem:
      return "T_PlannerParamItem";
    case duckdb_libpgquery::T_PGRollupData:
      return "T_RollupData";
    case duckdb_libpgquery::T_PGGroupingSetData:
      return "T_GroupingSetData";
    case duckdb_libpgquery::T_PGStatisticExtInfo:
      return "T_StatisticExtInfo";
    case duckdb_libpgquery::T_PGMemoryContext:
      return "T_MemoryContext";
    case duckdb_libpgquery::T_PGAllocSetContext:
      return "T_AllocSetContext";
    case duckdb_libpgquery::T_PGSlabContext:
      return "T_SlabContext";
    case duckdb_libpgquery::T_PGValue:
      return "T_Value";
    case duckdb_libpgquery::T_PGInteger:
      return "T_Integer";
    case duckdb_libpgquery::T_PGFloat:
      return "T_Float";
    case duckdb_libpgquery::T_PGString:
      return "T_String";
    case duckdb_libpgquery::T_PGBitString:
      return "T_BitString";
    case duckdb_libpgquery::T_PGNull:
      return "T_Null";
    case duckdb_libpgquery::T_PGList:
      return "T_List";
    case duckdb_libpgquery::T_PGIntList:
      return "T_IntList";
    case duckdb_libpgquery::T_PGOidList:
      return "T_OidList";
    case duckdb_libpgquery::T_PGExtensibleNode:
      return "T_ExtensibleNode";
    case duckdb_libpgquery::T_PGRawStmt:
      return "T_RawStmt";
    case duckdb_libpgquery::T_PGQuery:
      return "T_Query";
    case duckdb_libpgquery::T_PGPlannedStmt:
      return "T_PlannedStmt";
    case duckdb_libpgquery::T_PGInsertStmt:
      return "T_InsertStmt";
    case duckdb_libpgquery::T_PGDeleteStmt:
      return "T_DeleteStmt";
    case duckdb_libpgquery::T_PGUpdateStmt:
      return "T_UpdateStmt";
    case duckdb_libpgquery::T_PGSelectStmt:
      return "T_SelectStmt";
    case duckdb_libpgquery::T_PGAlterTableStmt:
      return "T_AlterTableStmt";
    case duckdb_libpgquery::T_PGAlterTableCmd:
      return "T_AlterTableCmd";
    case duckdb_libpgquery::T_PGAlterDomainStmt:
      return "T_AlterDomainStmt";
    case duckdb_libpgquery::T_PGSetOperationStmt:
      return "T_SetOperationStmt";
    case duckdb_libpgquery::T_PGGrantStmt:
      return "T_GrantStmt";
    case duckdb_libpgquery::T_PGGrantRoleStmt:
      return "T_GrantRoleStmt";
    case duckdb_libpgquery::T_PGAlterDefaultPrivilegesStmt:
      return "T_AlterDefaultPrivilegesStmt";
    case duckdb_libpgquery::T_PGClosePortalStmt:
      return "T_ClosePortalStmt";
    case duckdb_libpgquery::T_PGClusterStmt:
      return "T_ClusterStmt";
    case duckdb_libpgquery::T_PGCopyStmt:
      return "T_CopyStmt";
    case duckdb_libpgquery::T_PGCreateStmt:
      return "T_CreateStmt";
    case duckdb_libpgquery::T_PGDefineStmt:
      return "T_DefineStmt";
    case duckdb_libpgquery::T_PGDropStmt:
      return "T_DropStmt";
    case duckdb_libpgquery::T_PGTruncateStmt:
      return "T_TruncateStmt";
    case duckdb_libpgquery::T_PGCommentStmt:
      return "T_CommentStmt";
    case duckdb_libpgquery::T_PGFetchStmt:
      return "T_FetchStmt";
    case duckdb_libpgquery::T_PGIndexStmt:
      return "T_IndexStmt";
    case duckdb_libpgquery::T_PGCreateFunctionStmt:
      return "T_CreateFunctionStmt";
    case duckdb_libpgquery::T_PGAlterFunctionStmt:
      return "T_AlterFunctionStmt";
    case duckdb_libpgquery::T_PGDoStmt:
      return "T_DoStmt";
    case duckdb_libpgquery::T_PGRenameStmt:
      return "T_RenameStmt";
    case duckdb_libpgquery::T_PGRuleStmt:
      return "T_RuleStmt";
    case duckdb_libpgquery::T_PGNotifyStmt:
      return "T_NotifyStmt";
    case duckdb_libpgquery::T_PGListenStmt:
      return "T_ListenStmt";
    case duckdb_libpgquery::T_PGUnlistenStmt:
      return "T_UnlistenStmt";
    case duckdb_libpgquery::T_PGTransactionStmt:
      return "T_TransactionStmt";
    case duckdb_libpgquery::T_PGViewStmt:
      return "T_ViewStmt";
    case duckdb_libpgquery::T_PGLoadStmt:
      return "T_LoadStmt";
    case duckdb_libpgquery::T_PGCreateDomainStmt:
      return "T_CreateDomainStmt";
    case duckdb_libpgquery::T_PGCreatedbStmt:
      return "T_CreatedbStmt";
    case duckdb_libpgquery::T_PGDropdbStmt:
      return "T_DropdbStmt";
    case duckdb_libpgquery::T_PGVacuumStmt:
      return "T_VacuumStmt";
    case duckdb_libpgquery::T_PGExplainStmt:
      return "T_ExplainStmt";
    case duckdb_libpgquery::T_PGCreateTableAsStmt:
      return "T_CreateTableAsStmt";
    case duckdb_libpgquery::T_PGCreateSeqStmt:
      return "T_CreateSeqStmt";
    case duckdb_libpgquery::T_PGAlterSeqStmt:
      return "T_AlterSeqStmt";
    case duckdb_libpgquery::T_PGVariableSetStmt:
      return "T_VariableSetStmt";
    case duckdb_libpgquery::T_PGVariableShowStmt:
      return "T_VariableShowStmt";
    case duckdb_libpgquery::T_PGVariableShowSelectStmt:
      return "T_VariableShowSelectStmt";
    case duckdb_libpgquery::T_PGDiscardStmt:
      return "T_DiscardStmt";
    case duckdb_libpgquery::T_PGCreateTrigStmt:
      return "T_CreateTrigStmt";
    case duckdb_libpgquery::T_PGCreatePLangStmt:
      return "T_CreatePLangStmt";
    case duckdb_libpgquery::T_PGCreateRoleStmt:
      return "T_CreateRoleStmt";
    case duckdb_libpgquery::T_PGAlterRoleStmt:
      return "T_AlterRoleStmt";
    case duckdb_libpgquery::T_PGDropRoleStmt:
      return "T_DropRoleStmt";
    case duckdb_libpgquery::T_PGLockStmt:
      return "T_LockStmt";
    case duckdb_libpgquery::T_PGConstraintsSetStmt:
      return "T_ConstraintsSetStmt";
    case duckdb_libpgquery::T_PGReindexStmt:
      return "T_ReindexStmt";
    case duckdb_libpgquery::T_PGCheckPointStmt:
      return "T_CheckPointStmt";
    case duckdb_libpgquery::T_PGCreateSchemaStmt:
      return "T_CreateSchemaStmt";
    case duckdb_libpgquery::T_PGAlterDatabaseStmt:
      return "T_AlterDatabaseStmt";
    case duckdb_libpgquery::T_PGAlterDatabaseSetStmt:
      return "T_AlterDatabaseSetStmt";
    case duckdb_libpgquery::T_PGAlterRoleSetStmt:
      return "T_AlterRoleSetStmt";
    case duckdb_libpgquery::T_PGCreateConversionStmt:
      return "T_CreateConversionStmt";
    case duckdb_libpgquery::T_PGCreateCastStmt:
      return "T_CreateCastStmt";
    case duckdb_libpgquery::T_PGCreateOpClassStmt:
      return "T_CreateOpClassStmt";
    case duckdb_libpgquery::T_PGCreateOpFamilyStmt:
      return "T_CreateOpFamilyStmt";
    case duckdb_libpgquery::T_PGAlterOpFamilyStmt:
      return "T_AlterOpFamilyStmt";
    case duckdb_libpgquery::T_PGPrepareStmt:
      return "T_PrepareStmt";
    case duckdb_libpgquery::T_PGExecuteStmt:
      return "T_ExecuteStmt";
    case duckdb_libpgquery::T_PGCallStmt:
      return "T_CallStmt";
    case duckdb_libpgquery::T_PGDeallocateStmt:
      return "T_DeallocateStmt";
    case duckdb_libpgquery::T_PGDeclareCursorStmt:
      return "T_DeclareCursorStmt";
    case duckdb_libpgquery::T_PGCreateTableSpaceStmt:
      return "T_CreateTableSpaceStmt";
    case duckdb_libpgquery::T_PGDropTableSpaceStmt:
      return "T_DropTableSpaceStmt";
    case duckdb_libpgquery::T_PGAlterObjectDependsStmt:
      return "T_AlterObjectDependsStmt";
    case duckdb_libpgquery::T_PGAlterObjectSchemaStmt:
      return "T_AlterObjectSchemaStmt";
    case duckdb_libpgquery::T_PGAlterOwnerStmt:
      return "T_AlterOwnerStmt";
    case duckdb_libpgquery::T_PGAlterOperatorStmt:
      return "T_AlterOperatorStmt";
    case duckdb_libpgquery::T_PGDropOwnedStmt:
      return "T_DropOwnedStmt";
    case duckdb_libpgquery::T_PGReassignOwnedStmt:
      return "T_ReassignOwnedStmt";
    case duckdb_libpgquery::T_PGCompositeTypeStmt:
      return "T_CompositeTypeStmt";
    case duckdb_libpgquery::T_PGCreateTypeStmt:
      return "T_CreateTypeStmt";
    case duckdb_libpgquery::T_PGCreateRangeStmt:
      return "T_CreateRangeStmt";
    case duckdb_libpgquery::T_PGAlterEnumStmt:
      return "T_AlterEnumStmt";
    case duckdb_libpgquery::T_PGAlterTSDictionaryStmt:
      return "T_AlterTSDictionaryStmt";
    case duckdb_libpgquery::T_PGAlterTSConfigurationStmt:
      return "T_AlterTSConfigurationStmt";
    case duckdb_libpgquery::T_PGCreateFdwStmt:
      return "T_CreateFdwStmt";
    case duckdb_libpgquery::T_PGAlterFdwStmt:
      return "T_AlterFdwStmt";
    case duckdb_libpgquery::T_PGCreateForeignServerStmt:
      return "T_CreateForeignServerStmt";
    case duckdb_libpgquery::T_PGAlterForeignServerStmt:
      return "T_AlterForeignServerStmt";
    case duckdb_libpgquery::T_PGCreateUserMappingStmt:
      return "T_CreateUserMappingStmt";
    case duckdb_libpgquery::T_PGAlterUserMappingStmt:
      return "T_AlterUserMappingStmt";
    case duckdb_libpgquery::T_PGDropUserMappingStmt:
      return "T_DropUserMappingStmt";
    case duckdb_libpgquery::T_PGAlterTableSpaceOptionsStmt:
      return "T_AlterTableSpaceOptionsStmt";
    case duckdb_libpgquery::T_PGAlterTableMoveAllStmt:
      return "T_AlterTableMoveAllStmt";
    case duckdb_libpgquery::T_PGSecLabelStmt:
      return "T_SecLabelStmt";
    case duckdb_libpgquery::T_PGCreateForeignTableStmt:
      return "T_CreateForeignTableStmt";
    case duckdb_libpgquery::T_PGImportForeignSchemaStmt:
      return "T_ImportForeignSchemaStmt";
    case duckdb_libpgquery::T_PGCreateExtensionStmt:
      return "T_CreateExtensionStmt";
    case duckdb_libpgquery::T_PGAlterExtensionStmt:
      return "T_AlterExtensionStmt";
    case duckdb_libpgquery::T_PGAlterExtensionContentsStmt:
      return "T_AlterExtensionContentsStmt";
    case duckdb_libpgquery::T_PGCreateEventTrigStmt:
      return "T_CreateEventTrigStmt";
    case duckdb_libpgquery::T_PGAlterEventTrigStmt:
      return "T_AlterEventTrigStmt";
    case duckdb_libpgquery::T_PGRefreshMatViewStmt:
      return "T_RefreshMatViewStmt";
    case duckdb_libpgquery::T_PGReplicaIdentityStmt:
      return "T_ReplicaIdentityStmt";
    case duckdb_libpgquery::T_PGAlterSystemStmt:
      return "T_AlterSystemStmt";
    case duckdb_libpgquery::T_PGCreatePolicyStmt:
      return "T_CreatePolicyStmt";
    case duckdb_libpgquery::T_PGAlterPolicyStmt:
      return "T_AlterPolicyStmt";
    case duckdb_libpgquery::T_PGCreateTransformStmt:
      return "T_CreateTransformStmt";
    case duckdb_libpgquery::T_PGCreateAmStmt:
      return "T_CreateAmStmt";
    case duckdb_libpgquery::T_PGCreatePublicationStmt:
      return "T_CreatePublicationStmt";
    case duckdb_libpgquery::T_PGAlterPublicationStmt:
      return "T_AlterPublicationStmt";
    case duckdb_libpgquery::T_PGCreateSubscriptionStmt:
      return "T_CreateSubscriptionStmt";
    case duckdb_libpgquery::T_PGAlterSubscriptionStmt:
      return "T_AlterSubscriptionStmt";
    case duckdb_libpgquery::T_PGDropSubscriptionStmt:
      return "T_DropSubscriptionStmt";
    case duckdb_libpgquery::T_PGCreateStatsStmt:
      return "T_CreateStatsStmt";
    case duckdb_libpgquery::T_PGAlterCollationStmt:
      return "T_AlterCollationStmt";
    case duckdb_libpgquery::T_PGAExpr:
      return "TAExpr";
    case duckdb_libpgquery::T_PGColumnRef:
      return "T_ColumnRef";
    case duckdb_libpgquery::T_PGParamRef:
      return "T_ParamRef";
    case duckdb_libpgquery::T_PGAConst:
      return "TAConst";
    case duckdb_libpgquery::T_PGFuncCall:
      return "T_FuncCall";
    case duckdb_libpgquery::T_PGAStar:
      return "TAStar";
    case duckdb_libpgquery::T_PGAIndices:
      return "TAIndices";
    case duckdb_libpgquery::T_PGAIndirection:
      return "TAIndirection";
    case duckdb_libpgquery::T_PGAArrayExpr:
      return "TAArrayExpr";
    case duckdb_libpgquery::T_PGResTarget:
      return "T_ResTarget";
    case duckdb_libpgquery::T_PGMultiAssignRef:
      return "T_MultiAssignRef";
    case duckdb_libpgquery::T_PGTypeCast:
      return "T_TypeCast";
    case duckdb_libpgquery::T_PGCollateClause:
      return "T_CollateClause";
    case duckdb_libpgquery::T_PGSortBy:
      return "T_SortBy";
    case duckdb_libpgquery::T_PGWindowDef:
      return "T_WindowDef";
    case duckdb_libpgquery::T_PGRangeSubselect:
      return "T_RangeSubselect";
    case duckdb_libpgquery::T_PGRangeFunction:
      return "T_RangeFunction";
    case duckdb_libpgquery::T_PGRangeTableSample:
      return "T_RangeTableSample";
    case duckdb_libpgquery::T_PGRangeTableFunc:
      return "T_RangeTableFunc";
    case duckdb_libpgquery::T_PGRangeTableFuncCol:
      return "T_RangeTableFuncCol";
    case duckdb_libpgquery::T_PGTypeName:
      return "T_TypeName";
    case duckdb_libpgquery::T_PGColumnDef:
      return "T_ColumnDef";
    case duckdb_libpgquery::T_PGIndexElem:
      return "T_IndexElem";
    case duckdb_libpgquery::T_PGConstraint:
      return "T_Constraint";
    case duckdb_libpgquery::T_PGDefElem:
      return "T_DefElem";
    case duckdb_libpgquery::T_PGRangeTblEntry:
      return "T_RangeTblEntry";
    case duckdb_libpgquery::T_PGRangeTblFunction:
      return "T_RangeTblFunction";
    case duckdb_libpgquery::T_PGTableSampleClause:
      return "T_TableSampleClause";
    case duckdb_libpgquery::T_PGWithCheckOption:
      return "T_WithCheckOption";
    case duckdb_libpgquery::T_PGSortGroupClause:
      return "T_SortGroupClause";
    case duckdb_libpgquery::T_PGGroupingSet:
      return "T_GroupingSet";
    case duckdb_libpgquery::T_PGWindowClause:
      return "T_WindowClause";
    case duckdb_libpgquery::T_PGObjectWithArgs:
      return "T_ObjectWithArgs";
    case duckdb_libpgquery::T_PGAccessPriv:
      return "T_AccessPriv";
    case duckdb_libpgquery::T_PGCreateOpClassItem:
      return "T_CreateOpClassItem";
    case duckdb_libpgquery::T_PGTableLikeClause:
      return "T_TableLikeClause";
    case duckdb_libpgquery::T_PGFunctionParameter:
      return "T_FunctionParameter";
    case duckdb_libpgquery::T_PGLockingClause:
      return "T_LockingClause";
    case duckdb_libpgquery::T_PGRowMarkClause:
      return "T_RowMarkClause";
    case duckdb_libpgquery::T_PGXmlSerialize:
      return "T_XmlSerialize";
    case duckdb_libpgquery::T_PGWithClause:
      return "T_WithClause";
    case duckdb_libpgquery::T_PGInferClause:
      return "T_InferClause";
    case duckdb_libpgquery::T_PGOnConflictClause:
      return "T_OnConflictClause";
    case duckdb_libpgquery::T_PGCommonTableExpr:
      return "T_CommonTableExpr";
    case duckdb_libpgquery::T_PGRoleSpec:
      return "T_RoleSpec";
    case duckdb_libpgquery::T_PGTriggerTransition:
      return "T_TriggerTransition";
    case duckdb_libpgquery::T_PGPartitionElem:
      return "T_PartitionElem";
    case duckdb_libpgquery::T_PGPartitionSpec:
      return "T_PartitionSpec";
    case duckdb_libpgquery::T_PGPartitionBoundSpec:
      return "T_PartitionBoundSpec";
    case duckdb_libpgquery::T_PGPartitionRangeDatum:
      return "T_PartitionRangeDatum";
    case duckdb_libpgquery::T_PGPartitionCmd:
      return "T_PartitionCmd";
    case duckdb_libpgquery::T_PGIdentifySystemCmd:
      return "T_IdentifySystemCmd";
    case duckdb_libpgquery::T_PGBaseBackupCmd:
      return "T_BaseBackupCmd";
    case duckdb_libpgquery::T_PGCreateReplicationSlotCmd:
      return "T_CreateReplicationSlotCmd";
    case duckdb_libpgquery::T_PGDropReplicationSlotCmd:
      return "T_DropReplicationSlotCmd";
    case duckdb_libpgquery::T_PGStartReplicationCmd:
      return "T_StartReplicationCmd";
    case duckdb_libpgquery::T_PGTimeLineHistoryCmd:
      return "T_TimeLineHistoryCmd";
    case duckdb_libpgquery::T_PGSQLCmd:
      return "T_SQLCmd";
    case duckdb_libpgquery::T_PGTriggerData:
      return "T_TriggerData";
    case duckdb_libpgquery::T_PGEventTriggerData:
      return "T_EventTriggerData";
    case duckdb_libpgquery::T_PGReturnSetInfo:
      return "T_ReturnSetInfo";
    case duckdb_libpgquery::T_PGWindowObjectData:
      return "T_WindowObjectData";
    case duckdb_libpgquery::T_PGTIDBitmap:
      return "T_TIDBitmap";
    case duckdb_libpgquery::T_PGInlineCodeBlock:
      return "T_InlineCodeBlock";
    case duckdb_libpgquery::T_PGFdwRoutine:
      return "T_FdwRoutine";
    case duckdb_libpgquery::T_PGIndexAmRoutine:
      return "T_IndexAmRoutine";
    case duckdb_libpgquery::T_PGTsmRoutine:
      return "T_TsmRoutine";
    case duckdb_libpgquery::T_PGForeignKeyCacheInfo:
      return "T_ForeignKeyCacheInfo";
    default:
      return "(UNKNOWN)";
  }
}  // LCOV_EXCL_STOP

}  // namespace hmssql
