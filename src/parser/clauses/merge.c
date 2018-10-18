/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "merge.h"
#include "../ast_common.h"
#include "../../rmutil/rmalloc.h"

AST_MergeNode* New_AST_MergeNode(Vector *graphEntities) {
	AST_MergeNode *mergeNode = rm_malloc(sizeof(AST_MergeNode));
	mergeNode->graphEntities = graphEntities;
	return mergeNode;
}

void Free_AST_MergeNode(AST_MergeNode *mergeNode) {
	if(!mergeNode) return;

	/* There's no need in freeing entities vector
	 * as it is being used by match clause,
	 * which will free it. */
	free(mergeNode);
}
