#pragma once

#include "SyntaxNodeImpl.h"
#include <vector>

struct TokenNode;
struct BaseClassNode;
//struct TypeNameNode;

struct BaseClassListNode : SyntaxNodeImpl
{
	BaseClassListNode* m_baseClassList;
	TokenNode* m_delimiter;
	BaseClassNode* m_baseClass;
public:
	BaseClassListNode(BaseClassListNode* baseClassList, TokenNode* delimiter, BaseClassNode* baseClass);
	void collectBaseClassNodes(std::vector<BaseClassNode*>& baseClassNodes);
	void collectBaseClassNodesNotNoCode(std::vector<std::pair<TokenNode*, BaseClassNode*>>& baseClassNodes);
	void collectBaseClassNodesNotNoMeta(std::vector<std::pair<TokenNode*, BaseClassNode*>>& baseClassNodes);
};