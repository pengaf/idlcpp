#pragma once

#include "SyntaxNodeImpl.h"

struct TokenNode;
struct TypeNameNode;
struct IdentifyNode;
struct MethodNode;
struct TemplateArguments;
struct EmbededCode;

struct VariableTypeNode : SyntaxNodeImpl
{
	TypeNameNode* m_typeName;
	TypeCompound m_typeCompound;
public:
	VariableTypeNode(TypeNameNode* typeName, TypeCompound typeCompound);
public:
	void checkSemantic(TemplateArguments* templateArguments, bool outputParam);
};
